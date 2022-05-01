/*! \file artbase/src/stylebase.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: stylebase.cpp,v 1.142 2009/08/20 18:59:14 titato Exp $
*/

#include "a2dprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/fontenum.h>
#include <wx/module.h>

#include "wx/general/genmod.h"

//#include "wx/general/id.inl"

#include "wx/artbase/stylebase.h"
#include "wx/artbase/bbox.h"
#include "wx/artbase/drawer2d.h"
#include "wx/artbase/polyver.h"

#if defined(__USE_WINAPI__)
#include "wx/msw/dc.h"
#include "wx/artbase/mswfont.h"
#endif

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

#define newline wxString("\n")

#if wxART2D_USE_FREETYPE
FT_Library g_freetypeLibrary;
#endif

#define wxART2D_USE_DC2FREETYPE 1


IMPLEMENT_DYNAMIC_CLASS( a2dStroke, wxObject )
IMPLEMENT_DYNAMIC_CLASS( a2dFill, wxObject )

//----------------------------------------------------------------------------
// a2dFontInfoList
//----------------------------------------------------------------------------

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( a2dFontInfoList );

//----------------------------------------------------------------------------
// a2dFontInfo
//----------------------------------------------------------------------------

a2dFontInfo::a2dFontInfo( const wxString& type, const wxString& name,
                          const wxString& style, double size, const wxString& extra )
{
    m_type = type;
    m_name = name;
    m_style = style;
    m_size = size;
    m_extra = extra;
}

a2dFontInfo::a2dFontInfo( const wxString& type, const wxString& name,
                          const wxString& style, const wxString& extra )
{
    m_type = type;
    m_name = name;
    m_style = style;
    m_size = 1.0;
    m_extra = extra;
}

a2dFontInfo::a2dFontInfo( const wxString& info )
{
    ParseString( info );
}

a2dFontInfo& a2dFontInfo::operator=( const a2dFontInfo& other )
{
    m_type = other.m_type;
    m_name = other.m_name;
    m_style = other.m_style;
    m_size = other.m_size;
    m_extra = other.m_extra;
    return *this;
}

bool a2dFontInfo::operator == ( const a2dFontInfo& other ) const
{
    return m_type == other.m_type
           && m_name == other.m_name
           && m_style == other.m_style
           && m_size == other.m_size
           && m_extra == other.m_extra;
}

wxString a2dFontInfo::CreateString() const
{
    wxString description;
    if ( m_type == "Freetype font" )
    {

        wxString size = wxString::FromCDouble( m_size );
        description = m_type + wxT( "-" ) + m_name + wxT( "-" ) + m_style
                               + wxT( "-" ) + size;
        if ( !m_extra.IsEmpty() )
        {
            wxFileName file( m_extra );

            description = m_type + wxT( "-" )  + m_name + wxT( "-" ) + m_style
                                   + wxT( "-" ) + size + wxT( "--" ) + file.GetFullName();
        }
    }
    else
    {
        wxString size = wxString::FromCDouble( m_size );
        description = m_type + wxT( "-" ) + m_name + wxT( "-" ) + m_style
                               + wxT( "-" ) + size + wxT( "-" ) ;
        if ( !m_extra.IsEmpty() )
            description += wxT( "-" ) + m_extra;
    }
    return description;
}

void a2dFontInfo::ParseString( wxString string )
{
    size_t stringlength = string.Length();
    size_t idx, last;
    idx = last = 0;
    while ( idx < stringlength && string[idx] != wxChar( '-' ) )
        idx++;
    m_type = string.Mid( last, idx - last );
    if ( idx < stringlength )
        idx++;
    last = idx;
    while ( idx < stringlength && string[idx] != wxChar( '-' ) )
        idx++;
    m_name = string.Mid( last, idx - last );
    if ( idx < stringlength )
        idx++;
    last = idx;
    while ( idx < stringlength && string[idx] != wxChar( '-' ) )
        idx++;
    m_style = string.Mid( last, idx - last );
    if ( idx < stringlength )
        idx++;
    last = idx;
    while ( idx < stringlength && string[idx] != wxChar( '-' ) )
        idx++;
    string.Mid( last, idx - last ).ToCDouble( &m_size );
    if ( idx < stringlength )
        idx++;
    last = idx;
    while ( idx < stringlength && string[idx] != wxChar( '-' ) )
        idx++;
    if ( idx < stringlength )
        idx++;
    last = idx;
    m_extra = string.Mid( last );
}

a2dFont a2dFontInfo::CreateFont( double size )
{
    m_size = size;
    return a2dFont::CreateFont( *this );
}

//----------------------------------------------------------------------------
// a2dFontRefData
//----------------------------------------------------------------------------

class A2DARTBASEDLLEXP a2dFontRefData : public wxObjectRefData
{
    friend class a2dFont;
public:

    virtual a2dFontType GetType( void ) const { return a2dFONT_NULL; }

    a2dFontRefData( double size )
    {
        m_fontinfo.SetSize( size );
        m_height = 1;
        m_desc = 0;
        m_lead = 0;
        m_ok = false;
    }

    //!constructor
    a2dFontRefData()
    {
    }

    a2dFontRefData( const a2dFontRefData& other )
        : wxObjectRefData()
    {
        m_fontinfo = other.m_fontinfo;
        m_height = other.m_height;
        m_desc = other.m_desc;
        m_lead = other.m_lead;
        m_ok = other.m_ok;
    }

    bool Ok() { return m_ok; }
    virtual void SetSize( double size ) { m_fontinfo.SetSize( size ); }
    inline double GetSize() const { return m_fontinfo.GetSize(); }

    double GetDescent() const
    {
        if ( m_ok )
            return m_fontinfo.GetSize() * m_desc;
        else
            return 0.0;
    }

    double GetLineHeight() const
    {
        if ( m_ok )
            return m_fontinfo.GetSize() * m_height;
        else
            return 0.0;
    }

    double GetExternalLead() const
    {
        if ( m_ok )
            return m_fontinfo.GetSize() * m_lead;
        else
            return 0.0;
    }

    //! Get kerning space between two characters.
    /*! Kerning is a method, which moves characters closer together/further apart, to
        improve the look of the font. Eg. the letter combination LT will be moved closer together.
        The return value is negative or positive, to respectively move characters closer together, or further
        apart.
    */
    virtual double GetKerning( wxChar WXUNUSED( c1 ), wxChar WXUNUSED( c2 ) ) const { return 0.0; }

    //! Get width of a single character
    virtual double GetWidth( wxChar c ) const = 0;

    virtual bool GetVpath( wxChar c, a2dVpath& vpath, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX ) { return false; }

    virtual void GetTextExtent( const wxString& string, double& w, double& h, double& descent, double& externalLeading ) const;


protected:

    //! Font info
    a2dFontInfo m_fontinfo;

    //! Font height cache normalized to a font height of 1.
    double m_height;

    //! Font descender cache normalized to a font height of 1.
    double m_desc;

    //! Font external leading normalized to a font height of 1.
    double m_lead;

    //! True if a valid font is set.
    bool m_ok;

#if wxART2D_USE_CVGIO
    //! Save settings.
    //virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite ) = 0;
    //! Load settings.
    //virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts ) = 0;
#endif //wxART2D_USE_CVGIO

};

void a2dFontRefData::GetTextExtent( const wxString& string, double& w,
                                    double& h, double& descent, double& externalLeading ) const
{
    if ( m_ok )
    {
        double x(0.0);
        int len(string.Length());
        if (len)
        {
            wxChar ch_n_1, ch_n(string[(size_t)0]);
            x = GetWidth( ch_n );
            for ( size_t n = 1; n < len; n++ )
            {
                ch_n_1 = ch_n;
                ch_n = string[n];
                x += GetKerning( ch_n_1, ch_n);
                x += GetWidth( ch_n );
            }
        }
        w = x;
        h = GetSize() * m_height;
        descent = GetSize() * m_desc;
        externalLeading = GetSize() * m_lead;
    }
    else
        w = h = descent = externalLeading = 0.0;
}

//----------------------------------------------------------------------------
// a2dFontDcData
//----------------------------------------------------------------------------

//! DC font class.
/*! \class a2dFontDc stylebase.h artbase/stylebase.h
    Font class, based on the DC (device context) fonts. While being relatively fast, it has a
    disadvantage, that they can neither be stretched, nor slanted. Scaling is not a  problem.

    \todo EW: To overcome the non-proportional font scaling on windows, I draw every character myself.
    This pretty much disables the pango (font) renderer for gtk2. Therefore some non-western languages
    might not work properly (if they ever did?).

    \ingroup property style
    \ingroup textrelated
*/
class A2DARTBASEDLLEXP a2dFontDcData
    : public a2dFontRefData
{
    friend class a2dFont;
public:

    virtual a2dFontType GetType( void ) const { return a2dFONT_WXDC; }

    //! Constructor.
    /*! \param font Font format. The size of the font is not used and should be set using the next parameter.
        \param size Size in world coordinates.
    */
    a2dFontDcData( const wxFont& font = wxNullFont, double size = 0.0 );

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
    a2dFontDcData( double size,  wxFontFamily family, wxFontStyle style = wxFONTSTYLE_NORMAL, wxFontWeight weight = wxFONTWEIGHT_NORMAL,
                   const bool underline = false, const wxString& faceName = wxT( "" ),
                   wxFontEncoding encoding = wxFONTENCODING_DEFAULT );

    //! Copy constructor.
    a2dFontDcData( const a2dFontDcData& other );
    //! Destructor.
    virtual ~a2dFontDcData();

    //! Set font format.
    /*! \param font The font information. The size of the font is not used.
    */
    void SetFont( const wxFont& font );

    //! Get the font format.
    /*! \return The font information. The size of the font should be disregarded, since
        it is internally for storage.
    */
    wxFont& GetFont( void ) { return m_font; }

    //! Find the filename of the current font.
    /*! Currently only implemented for Windows.
        \return The absolute filename. In case of error an empty string is returned
    */
    wxString FindFileName() const;

    // Documented in base class
    double GetWidth( wxChar c ) const;

    // Documented in base class
    double GetKerning( wxChar c1, wxChar c2 ) const;

    //! to set for load glyph from face, to do proper hinting
    void SetDeviceHeight( double sizeInPixels );

    double GetDeviceHeight() { return m_sizeInPixels; }

    void GetTextExtent( const wxString& string, double& w, double& h, double& descent, double& externalLeading ) const;

    bool GetVpath( wxChar c, a2dVpath& glyph, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX );

    // Documented in base class
    static void GetInfoList( a2dFontInfoList& list );

    // Documented in base class
    static a2dFontDcData* CreateFont( const a2dFontInfo& info, bool force = false );

#if wxART2D_USE_FREETYPE
    // Documented in base class
    void SetSize( double size );

    //! Get the freetype equivalent of the font
    a2dFont GetFreetypeFont();

    void SetFreetypeFont( a2dFont font );
#endif


protected:

    //! current text drawn its size in pixels to do proper hinting.
    double m_sizeInPixels;

    //! Font
    wxFont m_font;

    //! Width cache normalized to a font height of 1. This stores the 'normal' part of the alphabet.
    mutable double* m_widthcache;
    //! Width cache normalized to a font height of 1. This stores the extended characters.
    mutable DcWidthHash m_extwidthcache;

#if defined(__USE_WINAPI__)
    //! Kerning cache normalized to a font height of 1.
    mutable KERNINGPAIR* m_kerningcache;
    mutable map<int, int> m_kerningmap; //! faster access than linear search in m_kerningcache
    //! Number of kerning pairs.
    mutable int m_nkerningpairs;
#else
    //! Kerning cache normalized to a font height of 1.
    mutable DcKerningHash m_kerningcache;
#endif

#if wxART2D_USE_FREETYPE
    //! freetype equivalent of the font, used in AggDrawer.
    a2dFont m_freetypefont;
#endif
};


#define CHAR_CACHE_START wxChar(' ')
#define CHAR_CACHE_END wxChar('~')

a2dFontDcData::a2dFontDcData( const wxFont& font, double size )
    : a2dFontRefData( size )
{
    m_sizeInPixels = NORMFONT;
    m_fontinfo.SetType( wxT( "Device font" ) );
    m_widthcache = NULL;
#if defined(__USE_WINAPI__)
    m_kerningcache = NULL;
#endif
    SetFont( font );
}

a2dFontDcData::a2dFontDcData( double size,
                              wxFontFamily family,
                              wxFontStyle style,
                              wxFontWeight weight,
                              const bool underline, const wxString& faceName, wxFontEncoding encoding )
    : a2dFontRefData( size )
{
    m_sizeInPixels = NORMFONT;
    m_fontinfo.SetType( wxT( "Device font" ) );
    m_widthcache = NULL;
#if defined(__USE_WINAPI__)
    m_kerningcache = NULL;
#endif
    SetFont( wxFont( 1, family, style, weight, underline, faceName, encoding ) );
}

a2dFontDcData::a2dFontDcData( const a2dFontDcData& other )
    : a2dFontRefData( other )
{
    m_sizeInPixels = NORMFONT;
    m_widthcache = NULL;
#if defined(__USE_WINAPI__)
    m_kerningcache = NULL;
#endif
    SetFont( other.m_font );
}

a2dFontDcData::~a2dFontDcData()
{
    if ( m_widthcache )
        delete[] m_widthcache;
#if defined(__USE_WINAPI__)
    if ( m_kerningcache )
        delete[] m_kerningcache;
#endif
}

#define DcHeightCorrection 1.3

void a2dFontDcData::SetDeviceHeight( double sizeInPixels )
{
    if ( m_sizeInPixels != sizeInPixels )
    {
        m_sizeInPixels = sizeInPixels;
        if ( m_sizeInPixels < 1 )
            m_sizeInPixels = 1;

        m_font.SetPointSize( ( int ) m_sizeInPixels / DcHeightCorrection );
        SetFont( m_font );

#if wxART2D_USE_FREETYPE
        if ( m_freetypefont != *a2dNullFONT )
            m_freetypefont.SetDeviceHeight( m_sizeInPixels );
#endif
    }
}


void a2dFontDcData::SetFont( const wxFont& font )
{
    m_font = font;
    m_ok = m_font.Ok();

    //delete old cache
    while ( !m_extwidthcache.empty() )
    {
        DcWidthHash::iterator it = m_extwidthcache.begin();
        m_extwidthcache.erase( it );
    }

    //prefill cache;
    if ( !m_widthcache )
        m_widthcache = new double[CHAR_CACHE_END - CHAR_CACHE_START + 1];

    wxMemoryDC dc;
    wxFont fnt = m_font;
    double times10 = 1;
    // get text extend gives wrong results when pointsize is smaller tha around 2 pixes, so we sheet to make it 10 times larger
    if ( m_sizeInPixels / DcHeightCorrection < 2 )
        times10 *= 10.0;
    fnt.SetPointSize( ( int ) times10 * m_sizeInPixels / DcHeightCorrection );
    dc.SetFont( fnt );
    wxChar c;
    wxString string;
    wxCoord width, height, desc, lead;
    width = height = desc = lead = 0;

#if defined(__USE_WINAPI__)
#if wxCHECK_VERSION(2,9,0)
    wxMSWDCImpl* dcimpl = wxStaticCast( dc.GetImpl(), wxMSWDCImpl );
    WXHDC hdc = dcimpl->GetHDC();
#else
    WXHDC hdc = dc.GetHDC();
#endif
#endif

    wxCoord descmax = 0;
#if defined(__USE_WINAPI__)
    string = CHAR_CACHE_START;
    dc.GetTextExtent( string, &width, &height, &descmax, &lead );

    ABCFLOAT* ABCarray = new ABCFLOAT[CHAR_CACHE_END - CHAR_CACHE_START + 1];
    GetCharABCWidthsFloat( ( HDC ) hdc, CHAR_CACHE_START, CHAR_CACHE_END, ( LPABCFLOAT ) ABCarray );

    for ( c = CHAR_CACHE_START; c <= CHAR_CACHE_END; c++ )
    {
        ABCFLOAT a = ABCarray[c - CHAR_CACHE_START];
        float widthf = a.abcfA + a.abcfB + a.abcfC;
        widthf = widthf / m_sizeInPixels / times10;
        m_widthcache[c - CHAR_CACHE_START] = widthf ;
    }
    delete ABCarray;
#else
    for ( c = CHAR_CACHE_START; c <= CHAR_CACHE_END; c++ )
    {
        string = c;
        dc.GetTextExtent( string, &width, &height, &desc, &lead );
        descmax = wxMax( descmax, desc );
        float widthf = ( float ) width / m_sizeInPixels / times10;
        m_widthcache[c - CHAR_CACHE_START] = widthf;
    }
#endif

    //normalized to font size of 1.
    m_height = ( double ) height / m_sizeInPixels / times10;
    m_desc = ( double ) descmax / m_sizeInPixels / times10;
    m_lead = ( double ) lead / m_sizeInPixels / times10;

    if ( m_font.GetFaceName().IsEmpty() )
        m_fontinfo.SetName( wxT( "LiberationSans" ) );
    else
        m_fontinfo.SetName( m_font.GetFaceName() );
    wxString style = wxEmptyString;
    switch ( m_font.GetWeight() )
    {
        case wxFONTWEIGHT_NORMAL:
            if ( m_font.GetStyle() == wxFONTSTYLE_NORMAL )
                style = wxT( "Regular" );
            break;
        case wxFONTWEIGHT_BOLD:
            style = wxT( "Bold " );
            break;
        case wxFONTWEIGHT_LIGHT:
            style = wxT( "Light " );
            break;
    }
    switch ( m_font.GetStyle() )
    {
        case wxFONTSTYLE_NORMAL:
            break;
        case wxFONTSTYLE_ITALIC:
        case wxFONTSTYLE_SLANT:
            style += wxT( "Italic" );
            break;
    }
    m_fontinfo.SetStyle( style.Trim() );

#if wxART2D_USE_FREETYPE
    m_freetypefont = *a2dNullFONT;

#if wxART2D_USE_DC2FREETYPE
    wxString filename = FindFileName();
    if ( !filename.IsEmpty() )
    {
        m_freetypefont = a2dFont( filename, GetSize() );
    }
#endif

#endif

#if defined(__USE_WINAPI__)
    // Standard drawing method on Windows using DrawText does not use kerning, therefore
    // we have to get the values ourselves.
    if ( m_kerningcache )
    {
        delete[] m_kerningcache;
        m_nkerningpairs = 0;
    }
    m_nkerningpairs = ::GetKerningPairs( ( HDC ) hdc, 0, NULL );
    m_kerningcache = new KERNINGPAIR[ m_nkerningpairs ];
    m_nkerningpairs = ::GetKerningPairs( ( HDC ) hdc, m_nkerningpairs, m_kerningcache );

    // init map
    KERNINGPAIR* ptr = m_kerningcache;
    int i = m_nkerningpairs;
    while ( i-- )
    {
        m_kerningmap[(ptr->wFirst << 16) + ptr->wSecond] = ptr->iKernAmount;
        ptr++;
    }


#endif //defined(__USE_WINAPI__)
    dc.SetFont( wxNullFont );
}


void a2dFontDcData::GetTextExtent( const wxString& string, double& w,
                                   double& h, double& descent, double& externalLeading ) const
{
    a2dFontRefData::GetTextExtent( string, w, h, descent, externalLeading );
    return;

    if ( !m_ok )
    {
        w = h = descent = externalLeading = 0.0;
        return;
    }

    wxMemoryDC dc;
    wxFont fnt = m_font;
    double times10 = 1;
    // get text extend gives wrong results when pointsize is smaller tha around 2 pixes, so we sheet to make it 10 times larger
    if ( m_sizeInPixels / DcHeightCorrection < 2 )
        times10 *= 10.0;
    fnt.SetPointSize( ( int ) times10 * m_sizeInPixels / DcHeightCorrection );
    dc.SetFont( fnt );

    wxCoord width, height, desc, lead;
    width = height = desc = lead = 0;

    dc.GetTextExtent( string, &width, &height, &desc, &lead );
    w = GetSize() * width / m_sizeInPixels / times10;
    h = GetSize() * height / m_sizeInPixels / times10;
    descent = GetSize() * desc / m_sizeInPixels / times10;
    externalLeading = GetSize() * lead / m_sizeInPixels / times10;
}

wxString a2dFontDcData::FindFileName() const
{
    wxString filename;
#if defined(__USE_WINAPI__)
    wxString facename = m_font.GetFaceName();
    if ( m_font.GetWeight() == wxFONTWEIGHT_BOLD )
        facename += wxT( " Bold" );
    if ( m_font.GetStyle() == wxFONTSTYLE_ITALIC )
        facename += wxT( " Italic" );

    TCHAR displayname[_MAX_PATH];
    TCHAR fontfilename[_MAX_PATH];
    if( GetFontFile( facename.c_str(), displayname, _MAX_PATH - 1,
                     fontfilename, _MAX_PATH - 1 ) )
    {
        filename = fontfilename;
    }
#endif // defined( __USE_WINAPI__ )

    return filename;
}

#if wxART2D_USE_FREETYPE
void a2dFontDcData::SetSize( double size )
{
    a2dFontRefData::SetSize( size );
    if ( m_freetypefont != *a2dNullFONT )
        m_freetypefont.SetSize( size );
}
#endif

double a2dFontDcData::GetWidth( wxChar c ) const
{
    if ( !m_ok )
        return 0.0;

    // if character is a 'normal' ascii character, use lookup-table.
    if ( c >= CHAR_CACHE_START && c <= CHAR_CACHE_END )
        return GetSize() * m_widthcache[c - CHAR_CACHE_START];
    else
    {
        DcWidthHash::iterator it = m_extwidthcache.find( c );
        if ( it == m_extwidthcache.end() )
        {
            wxMemoryDC dc;
            wxFont font = m_font;
            font.SetPointSize( ( int ) m_sizeInPixels / DcHeightCorrection );
            dc.SetFont( font );
            wxCoord width, height, desc, lead;
            wxString string = c;
            dc.GetTextExtent( string, &width, &height, &desc, &lead );
            dc.SetFont( wxNullFont );
            m_extwidthcache[c] = ( double ) width / m_sizeInPixels;
            return GetSize() * width / m_sizeInPixels;
        }
        else
            return GetSize() * it->second;
    }
}

double a2dFontDcData::GetKerning( wxChar c1, wxChar c2 ) const
{
#if defined( __USE_WINAPI__ )
    map<int,int>::iterator it(m_kerningmap.find((c1 << 16) + c2));
    if (it != m_kerningmap.end())
        return GetSize() * (*it).second / m_sizeInPixels;
    else
    return 0.0;
#else // defined( __USE_WINAPI__ )
    wxString ab = c1;
    ab += c2;
    DcKerningHash::iterator it = m_kerningcache.find( ab );
    if ( it == m_kerningcache.end() )
    {
        wxMemoryDC dc;
        wxFont font = m_font;
        font.SetPointSize( ( int ) m_sizeInPixels / DcHeightCorrection );
        dc.SetFont( font );
        wxCoord width, height, desc, lead;
        dc.GetTextExtent( ab, &width, &height, &desc, &lead );
        dc.SetFont( wxNullFont );
        double kerning = ( double ) width / m_sizeInPixels - ( GetWidth( c1 ) + GetWidth( c2 ) ) / GetSize();
        m_kerningcache[ab] = kerning;
        return GetSize() * kerning;
    }
    else
        return GetSize() * it->second;
#endif // defined( __USE_WINAPI__ )
}

bool a2dFontDcData::GetVpath( wxChar c, a2dVpath& vpath, const a2dAffineMatrix& affine )
{
#if wxART2D_USE_FREETYPE
#if defined( __USE_WINAPI__ )
    /*
    bool flip_y = false;
    const char* cur_glyph = c;
    double x, y;

    const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;

    const char* end_poly = cur_glyph + th->cb;
    const char* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

    x = fx_to_dbl(th->pfxStart.x);
    y = fx_to_dbl(th->pfxStart.y);
    if(flip_y) y = -y;
    affine.Translate(&x, &y);
    vpath.MoveTo(value_type(dbl_to_int26p6(x)),
                 value_type(dbl_to_int26p6(y)));

    while(cur_poly < end_poly)
    {
        const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

        if (pc->wType == TT_PRIM_LINE)
        {
            int i;
            for (i = 0; i < pc->cpfx; i++)
            {
                x = fx_to_dbl(pc->apfx[i].x);
                y = fx_to_dbl(pc->apfx[i].y);
                if(flip_y) y = -y;
                affine.Translate(&x, &y);
                path.line_to(value_type(dbl_to_int26p6(x)),
                             value_type(dbl_to_int26p6(y)));
            }
        }

        if (pc->wType == TT_PRIM_QSPLINE)
        {
            int u;
            for (u = 0; u < pc->cpfx - 1; u++)  // Walk through points in spline
            {
                POINTFX pnt_b = pc->apfx[u];    // B is always the current point
                POINTFX pnt_c = pc->apfx[u+1];

                if (u < pc->cpfx - 2)           // If not on last spline, compute C
                {
                    // midpoint (x,y)
                    *(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
                    *(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
                }

                double x2, y2;
                x  = fx_to_dbl(pnt_b.x);
                y  = fx_to_dbl(pnt_b.y);
                x2 = fx_to_dbl(pnt_c.x);
                y2 = fx_to_dbl(pnt_c.y);
                if(flip_y) { y = -y; y2 = -y2; }
                affine.Translate(&x, &y);
                affine.Translate(&x2, &y2);
                path.curve3(value_type(dbl_to_int26p6(x)),
                            value_type(dbl_to_int26p6(y)),
                            value_type(dbl_to_int26p6(x2)),
                            value_type(dbl_to_int26p6(y2)));
            }
        }
        cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
    }
    */
    if ( m_freetypefont != *a2dNullFONT )
        return m_freetypefont.GetVpath( c, vpath, affine );
#else // defined( __USE_WINAPI__ )
    if ( m_freetypefont != *a2dNullFONT )
        return m_freetypefont.GetVpath( c, vpath, affine );
#endif // defined( __USE_WINAPI__ )
    return false;
#else
    return false;
#endif
}

//! a2dFontDcData auxiliry class to aid in enumerating fonts.
class DcFontEnumerator : public wxFontEnumerator
{
public:
    virtual bool OnFacename( const wxString& facename )
    {
        m_facenames.Add( facename );
        return true;
    }
    wxArrayString m_facenames;
};

void a2dFontDcData::GetInfoList( a2dFontInfoList& list )
{
    DcFontEnumerator enumerator;
    enumerator.EnumerateFacenames();
    for ( unsigned int i = 0; i < enumerator.m_facenames.GetCount(); i++ )
    {
        wxString fontname = enumerator.m_facenames.Item( i );
        list.Append( new a2dFontInfo( wxT( "Device font" ), fontname, wxT( "Regular" ), 1.0 ) );
        list.Append( new a2dFontInfo( wxT( "Device font" ), fontname, wxT( "Italic" ), 1.0 ) );
        list.Append( new a2dFontInfo( wxT( "Device font" ), fontname, wxT( "Bold" ), 1.0 ) );
        list.Append( new a2dFontInfo( wxT( "Device font" ), fontname, wxT( "Bold Italic" ), 1.0 ) );
        list.Append( new a2dFontInfo( wxT( "Device font" ), fontname, wxT( "Light" ), 1.0 ) );
        list.Append( new a2dFontInfo( wxT( "Device font" ), fontname, wxT( "Light Italic" ), 1.0 ) );
    }
}

a2dFontDcData* a2dFontDcData::CreateFont( const a2dFontInfo& info, bool force )
{
    wxFontStyle slant = wxFONTSTYLE_NORMAL;
    wxFontWeight weight = wxFONTWEIGHT_NORMAL;
    if ( force || info.GetType().Upper() == wxT( "DEVICE FONT" ) )
    {
        if ( info.GetStyle().Upper().Find( wxT( "ITALIC" ) ) != -1 )
            slant = wxFONTSTYLE_ITALIC;
        if ( info.GetStyle().Upper().Find( wxT( "BOLD" ) ) != -1 )
            weight = wxFONTWEIGHT_BOLD;
        if ( info.GetStyle().Upper().Find( wxT( "LIGHT" ) ) != -1 )
            weight = wxFONTWEIGHT_LIGHT;

        a2dFontDcData* font = new a2dFontDcData( info.GetSize(), wxFONTFAMILY_SWISS, slant, weight, false, info.GetName() );
        if ( font->Ok() )
            return font;
        else
            delete font;
    }
    return NULL;
}

#if wxART2D_USE_FREETYPE
a2dFont a2dFontDcData::GetFreetypeFont()
{
    return m_freetypefont;
}

void a2dFontDcData::SetFreetypeFont( a2dFont font )
{
    m_freetypefont = font;
}

#endif

//--------------------------------------------------------------------
// a2dFreetypeModule
//--------------------------------------------------------------------

class a2dFreetypeFace;


IMPLEMENT_DYNAMIC_CLASS( a2dFreetypeModule, wxModule )

a2dFreetypeModule::a2dFreetypeModule()
{
    AddDependency( CLASSINFO( a2dGeneralModule ) );
}

bool a2dFreetypeModule::OnInit()
{
#if wxART2D_USE_FREETYPE
    int error = FT_Init_FreeType( &g_freetypeLibrary );
    if ( error ) return false;
#endif
    return true;
}

void a2dFreetypeModule::OnExit()
{
    a2dFont::ClearFontCache();

#if wxART2D_USE_FREETYPE
    FT_Done_FreeType( g_freetypeLibrary );
    // Sometimes library is deleted before other fonts are deleted, therefore
    // set g_freetypeLibrary to NULL. Memory leakage to be solved.
    g_freetypeLibrary = NULL;
#endif
}

#if wxART2D_USE_FREETYPE
//----------------------------------------------------------------------------
// a2dFontFreetypeData
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// a2dFontFreetypeData
//----------------------------------------------------------------------------

//! Freetype font class.
/*! \class a2dFontFreetypeData stylebase.h artbase/stylebase.h
    \ingroup property style
    \ingroup textrelated
*/
class A2DARTBASEDLLEXP a2dFontFreetypeData
    : public a2dFontRefData
{
    friend class a2dFont;
public:

    virtual a2dFontType GetType( void ) const { return a2dFONT_FREETYPE; }

    //! Constructor.
    /*! \param filename Filename of the vector font.
        \param size Size in world coordinates.
    \param encoding endocing of font
    */
    a2dFontFreetypeData( const wxString& filename = wxT( "" ), double size = 0.0, wxFontEncoding encoding = wxFONTENCODING_DEFAULT, int faceindex = 0 );
#if defined(__USE_WINAPI__)
    //! Constructor.
    /*! This constructor tries to locate the font file, belonging to the wxFont class, which
        is then used for drawing. At present only a Windows version is implemented.
        \param font wxFont class, to be used as the vector font.
        \param size Size in world coordinates.
    \param encoding endocing of font
    */
    a2dFontFreetypeData( const wxFont& font, double size = 0.0, wxFontEncoding encoding = wxFONTENCODING_DEFAULT );
#endif // defined( __USE_WINAPI__ )

    //! Copy constructor.
    a2dFontFreetypeData( const a2dFontFreetypeData& other );

    //! Destructor.
    virtual ~a2dFontFreetypeData();

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

    void SetEncodingConverter( wxEncodingConverter* converter );

    //! Get the glyph of a single character.
    /*! \return The glyph of the character, in a coordinate space, normalized to a font size m_sizeInPixels.
    */
    const a2dGlyph* GetGlyph( wxChar c ) const;

    //! to set for load glyph from face, to do proper hinting
    void SetDeviceHeight( double sizeInPixels );

    double GetDeviceHeight() { return m_sizeInPixels; }

    void GetTextExtent( const wxString& string, double& w, double& h, double& descent, double& externalLeading ) const;

    // Documented in base class
    double GetWidth( wxChar c ) const;

    double GetGlyphHeight( wxChar c ) const;

    // Documented in base class
    double GetKerning( wxChar c1, wxChar c2 ) const;

    // Documented in base class
    bool GetVpath( wxChar c, a2dVpath& glyph, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX );

    // Documented in base class
    static void GetInfoList( a2dFontInfoList& list );

    // Documented in base class
    static a2dFontFreetypeData* CreateFont( const a2dFontInfo& info, bool force = false );

    //! Set the way glyphs are loaded by freetype. Important for small fonts.
    //! a2d_LOAD_DEFAULT;
    //! a2d_LOAD_DEFAULT | a2d_LOAD_NO_HINTING;
    //! a2d_LOAD_DEFAULT | a2d_LOAD_FORCE_AUTOHINT;
    static void SetLoadFlags( wxInt32 loadMethodMask ) { m_loadMethodMask = loadMethodMask; }

protected:

    static wxInt32 m_loadMethodMask;

    //! Filename of the vector font.
    wxString m_filename;

    //! Glyph Cache. This stores the 'normal' part of the alphabet.
    typedef vector< a2dGlyph > a2dGlyphCache;
    mutable a2dGlyphCache m_glyphcache;
    //! Glyph cache. This stores the extended characters.
    mutable a2dGlyphHash m_extglyphcache;

    //mutable map< int, a2dGlyphCacheHash > m_glyphcacheOnDevHeight;

#if wxART2D_USE_FREETYPE
    //! last character for glyph cache one character
    mutable wxChar   m_lastChar;
    //! Glyph cache one last character
    mutable a2dGlyph* m_a2dLastGlyph;
#endif // wxART2D_USE_FREETYPE

    //! freetypelib face struct.
    FT_Face m_face;

    //! encoding setting
    wxFontEncoding m_encoding;

    //! current text drawn its size in pixels to do proper hinting.
    double m_sizeInPixels;

    //! what font with index to load from file
    long m_faceindex;

    //! encoding converter
    wxEncodingConverter* m_converter;

    //! when character is not found in font, it is replaced by '?'
    void ClearCacheLinkes( void );
};

wxInt32 a2dFontFreetypeData::m_loadMethodMask = a2dFont::a2d_LOAD_DEFAULT;

a2dFontFreetypeData::a2dFontFreetypeData( const wxString& filename, double size, wxFontEncoding encoding, int faceindex )
    : a2dFontRefData( size )
{
    m_sizeInPixels = NORMFONT;
    m_fontinfo.SetType( wxT( "Freetype font" ) );
    m_face = NULL;
    m_converter = NULL;
    m_encoding = encoding;
#if wxART2D_USE_FREETYPE
    m_lastChar = ( wxChar ) - 1;
    m_a2dLastGlyph = NULL;
#endif

    if( m_encoding != wxFONTENCODING_DEFAULT )
    {
        m_converter = new wxEncodingConverter();
        wxASSERT( m_converter != NULL );
        if( !m_converter->Init( m_encoding, wxFONTENCODING_UNICODE ) )
        {
            delete m_converter;
            m_converter = NULL;
//   m_encoding = wxFONTENCODING_DEFAULT;
        }
    }
    m_faceindex = faceindex;
    SetFilename( filename );
}

#if defined(__USE_WINAPI__)
a2dFontFreetypeData::a2dFontFreetypeData( const wxFont& font, double size, wxFontEncoding encoding )
    : a2dFontRefData( size )
{
    m_sizeInPixels = NORMFONT;
    m_fontinfo.SetType( wxT( "Freetype font" ) );
    m_face = NULL;
    m_converter = NULL;
    m_encoding = encoding;
#if wxART2D_USE_FREETYPE
    m_lastChar = ( wxChar ) - 1;
    m_a2dLastGlyph = NULL;
#endif

    if( m_encoding != wxFONTENCODING_DEFAULT )
    {
        m_converter = new wxEncodingConverter();
        wxASSERT( m_converter != NULL );
        if( !m_converter->Init( m_encoding, wxFONTENCODING_UNICODE ) )
        {
            delete m_converter;
            m_converter = NULL;
            //   m_encoding = wxFONTENCODING_DEFAULT;
        }
    }

    wxString filename;
    wxString facename = font.GetFaceName();
    if ( font.GetWeight() == wxFONTWEIGHT_BOLD )
        facename += wxT( " Bold" );
    if ( font.GetStyle() == wxFONTSTYLE_ITALIC )
        facename += wxT( " Italic" );

    m_faceindex = 0;

    TCHAR displayname[10];
    TCHAR fontfilename[_MAX_PATH];

    if( GetFontFile( facename.c_str(), displayname, sizeof( displayname ),
                     fontfilename, sizeof( fontfilename ) ) )
    {
        SetFilename( fontfilename );
    }
}
#endif // defined( __USE_WINAPI__ )

a2dFontFreetypeData::a2dFontFreetypeData( const a2dFontFreetypeData& other )
    : a2dFontRefData( other )
{
    m_sizeInPixels = NORMFONT;
    m_face = NULL;
    m_converter = NULL;
    m_encoding = other.m_encoding;
    m_faceindex = other.m_faceindex;
#if wxART2D_USE_FREETYPE
    m_lastChar = ( wxChar ) - 1;
    m_a2dLastGlyph = NULL;
#endif

    if( m_encoding != wxFONTENCODING_DEFAULT )
    {
        m_converter = new wxEncodingConverter();
        wxASSERT( m_converter != NULL );
        if( !m_converter->Init( m_encoding, wxFONTENCODING_UNICODE ) )
        {
            delete m_converter;
            m_converter = NULL;
            //   m_encoding = wxFONTENCODING_DEFAULT;
        }
    }
    SetFilename( other.m_filename );
}

a2dFontFreetypeData::~a2dFontFreetypeData()
{
#if wxART2D_USE_FREETYPE
    // free cache.
    ClearCacheLinkes();
    m_extglyphcache.clear();
    // release face.
    if ( m_face && g_freetypeLibrary )
        FT_Done_Face( m_face );

#endif // wxART2D_USE_FREETYPE
    if( m_converter )
        delete m_converter;
}

void a2dFontFreetypeData::SetDeviceHeight( double sizeInPixels )
{
    if ( fabs(m_sizeInPixels - sizeInPixels) > 0.0001 )
    {
        m_sizeInPixels = sizeInPixels;
        if ( m_sizeInPixels < 1 )
            m_sizeInPixels = 1;

        //SetFilename( m_filename );

#if wxART2D_USE_FREETYPE
        int error;
        //The character widths and heights are specified in 1/64th of points.
        //A point is a physical distance, equaling 1/72th of an inch. Normally, it is not equivalent to a pixel.
		error = FT_Set_Pixel_Sizes( m_face, 0, ( int ) m_sizeInPixels );
        //error = FT_Set_Char_Size( m_face, 0, m_sizeInPixels * 64, 96, 96 );// screen dpi

        wxASSERT_MSG( error == 0 , _( "a2dText problem setting character size" ) );
        if ( error )
        {
            m_face = NULL;
            return;
        }
        const FT_Size_Metrics& size = m_face->size->metrics;
        m_height = ( size.ascender - size.descender ) / ( m_sizeInPixels * 64.0 );
        m_desc = -size.descender / ( m_sizeInPixels * 64.0 );
        m_lead = ( size.height - ( size.ascender - size.descender ) ) / ( m_sizeInPixels * 64.0 );

#else //wxART2D_USE_FREETYPE
        // reasonable estimate
        m_height = 1.6;
        m_desc = 0.3;
        m_lead = 0.0;
#endif // wxART2D_USE_FREETYPE

        // If cache did not exist, create it.
        if ( m_glyphcache.empty() )
        {
            m_glyphcache.reserve( CHAR_CACHE_END - CHAR_CACHE_START + 1 );
            for ( size_t i = CHAR_CACHE_START; i <= CHAR_CACHE_END; i++ )
                m_glyphcache.push_back( a2dGlyph() );
        }
        ClearCacheLinkes();
        //clear previous cache
        for ( size_t i = CHAR_CACHE_START; i <= CHAR_CACHE_END; i++ )
            if( m_glyphcache[i - CHAR_CACHE_START].m_glyph != NULL )
                FT_Done_Glyph( m_glyphcache[i - CHAR_CACHE_START].m_glyph );
        // Clear cache.
        for ( size_t j = CHAR_CACHE_START; j <= CHAR_CACHE_END; j++ )
            m_glyphcache[j - CHAR_CACHE_START].m_glyph = NULL;

        m_extglyphcache.clear();
    }
}

wxString a2dFontFreetypeData::GetFilename( bool filenameonly ) const
{
    if ( filenameonly )
    {
        wxFileName filename( m_filename );
        return filename.GetFullName();
    }
    else
        return m_filename;
}

void a2dFontFreetypeData::ClearCacheLinkes( void )
{
#if wxART2D_USE_FREETYPE
    m_lastChar = ( wxChar ) - 1;
    m_a2dLastGlyph = NULL;
#endif
    // The '?'is used as dummy character when not found in font.
    // So only release with FT_Done_Glyph once, the rest are links to the same glyph.
    int qpos = wxChar( '?' ) - CHAR_CACHE_START;
    if( m_glyphcache[qpos].m_glyph != NULL && m_glyphcache[qpos].m_glyph != ( FT_Glyph ) - 1 )
    {
        // the '?' was used, either in real text or as replacement for not found character.
        FT_Done_Glyph( m_glyphcache[qpos].m_glyph );
        size_t index = m_glyphcache[qpos].m_index;
        // now reset all uses of glyph '?'
        for ( size_t i = CHAR_CACHE_START; i <= CHAR_CACHE_END; i++ )
            if( m_glyphcache[i - CHAR_CACHE_START].m_index == index )
                m_glyphcache[i - CHAR_CACHE_START].m_glyph = NULL;

        a2dGlyphHash::iterator it = m_extglyphcache.begin();
        for( ; it != m_extglyphcache.end(); it++ )
        {
            if( it->second.m_index == index )
                it->second.m_glyph = NULL;
        }
    }
}

void a2dFontFreetypeData::SetFilename( const wxString& filename )
{
    if (m_filename == filename)
    {
        m_ok = true;
        return;
    }
    m_filename = filename;

#if wxART2D_USE_FREETYPE
    if ( m_face )
    {
        FT_Done_Face( m_face );
        m_face = NULL;
    }
    m_ok = false;

    // If cache did not exist, create it.
    if ( m_glyphcache.empty() )
    {
        m_glyphcache.reserve( CHAR_CACHE_END - CHAR_CACHE_START + 1 );
        for ( size_t i = CHAR_CACHE_START; i <= CHAR_CACHE_END; i++ )
            m_glyphcache.push_back( a2dGlyph() );
    }
    ClearCacheLinkes();
    //clear previous cache
    for ( size_t i = CHAR_CACHE_START; i <= CHAR_CACHE_END; i++ )
        if( m_glyphcache[i - CHAR_CACHE_START].m_glyph != NULL )
            FT_Done_Glyph( m_glyphcache[i - CHAR_CACHE_START].m_glyph );
    // Clear cache.
    for ( size_t j = CHAR_CACHE_START; j <= CHAR_CACHE_END; j++ )
        m_glyphcache[j - CHAR_CACHE_START].m_glyph = NULL;

    m_extglyphcache.clear();

    if ( filename.IsEmpty() )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Font file path is empty" ) );
        return;
    }

    wxString fname = a2dGlobals->GetFontPathList().FindValidPath( m_filename );
    if ( !::wxFileExists( fname ) )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for loading font" ), m_filename.c_str() );
        return;
    }

    FT_Error error;
    error = FT_New_Face( g_freetypeLibrary, fname.mb_str(), m_faceindex, &m_face );
    if ( error )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "file %s contains wrong font" ), m_filename.c_str() );
        m_face = NULL;
        return;
    }
    else if ( !( m_face->face_flags & FT_FACE_FLAG_SCALABLE ) )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "file %s contains non scalable font" ), m_filename.c_str() );
        m_face = NULL;
        return;
    }

    //The character widths and heights are specified in 1/64th of points.
    //A point is a physical distance, equaling 1/72th of an inch. Normally, it is not equivalent to a pixel.
    error = FT_Set_Pixel_Sizes( m_face, 0, ( int ) m_sizeInPixels );
    //error = FT_Set_Char_Size( m_face, 0, (int) m_sizeInPixels * 64, 96, 96 );// screen dpi

    wxASSERT_MSG( error == 0 , _( "a2dText problem setting character size" ) );
    if ( error )
    {
        m_face = NULL;
        return;
    }

    const FT_Size_Metrics& size = m_face->size->metrics;
    m_height = ( size.ascender - size.descender ) / ( m_sizeInPixels * 64.0 );
    m_desc = -size.descender / ( m_sizeInPixels * 64.0 );
    m_lead = ( size.height - ( size.ascender - size.descender ) ) / ( m_sizeInPixels * 64.0 );

    m_fontinfo.SetName( wxString::FromAscii( m_face->family_name ) );
    m_fontinfo.SetStyle( wxString::FromAscii( m_face->style_name ) );
    m_fontinfo.SetExtra( GetFilename( true ) );
#else //wxART2D_USE_FREETYPE
    // reasonable estimate
    m_height = 1.6;
    m_desc = 0.3;
    m_lead = 0.0;
#endif // wxART2D_USE_FREETYPE
    m_ok = true;
}

const a2dGlyph* a2dFontFreetypeData::GetGlyph( wxChar cinput ) const
{
#if wxART2D_USE_FREETYPE
    if( cinput == m_lastChar )
        return m_a2dLastGlyph;

    if ( !m_face )
        return NULL;

    // if character is a 'normal' ascii character, use lookup-table.
    if ( cinput >= CHAR_CACHE_START && cinput <= CHAR_CACHE_END )
    {
        a2dGlyph* a2dglyph = &m_glyphcache[cinput - CHAR_CACHE_START];
        FT_Glyph glyph = a2dglyph->m_glyph;
        if ( !glyph )
        {
            wchar_t wc = ( wchar_t )cinput;

            if( m_converter )
            {
#if wxUSE_WCHAR_T
                wxChar cinputstr[2];
                wchar_t wcstr[2];
                cinputstr[0] = cinput;
                cinputstr[1] = 0;
                m_converter->Convert( cinputstr, wcstr );
                wc = wcstr[0];
#else
                wc = m_converter->Convert( wxString( cinput ) ).Last();
#endif
            }

            FT_ULong c = ( FT_ULong ) wc;

            FT_UInt index = FT_Get_Char_Index( m_face, ( FT_ULong ) c );
            /* load the glyph image (in its native format) */
            int error;
            error = FT_Load_Glyph( m_face, index, m_loadMethodMask );
            wxASSERT_MSG( error == 0 , _( "Glyph could not be loaded" ) );
            if ( !error )
            {
                error = FT_Get_Glyph( m_face->glyph, &glyph );
                wxASSERT_MSG( error == 0 , _( "Glyph could not be get" ) );
                if ( error )
                    glyph = NULL;
            }
            if ( glyph )
                glyph->advance = m_face->glyph->advance;
            if ( !glyph )
            {
                glyph = ( FT_Glyph ) - 1;
                if ( c != wxChar( '?' ) )
                {
                    const a2dGlyph* glyphErr = GetGlyph( wxChar( '?' ) );
                    if( glyphErr )
                    {
                        index = a2dglyph->m_index;
                        glyph = glyphErr->m_glyph;
                    }
                }
            }
            a2dglyph->m_index = index;
            a2dglyph->m_glyph = glyph;
            a2dglyph->m_lsb = m_face->glyph->lsb_delta;
            a2dglyph->m_rsb = m_face->glyph->rsb_delta;
            a2dglyph->m_face = m_face;
        }
        if ( glyph == ( FT_Glyph ) - 1 )
            a2dglyph = NULL;

        m_lastChar = cinput;
        m_a2dLastGlyph = a2dglyph;

        return a2dglyph;
    }
    else
    {
        a2dGlyph* a2dglyph = &m_extglyphcache[cinput];
        FT_Glyph glyph = a2dglyph->m_glyph;
        if ( !glyph )
        {
            wchar_t wc = ( wchar_t )cinput;

            if( m_converter )
            {
#if wxUSE_WCHAR_T
                wxChar cinputstr[2];
                wchar_t wcstr[2];
                cinputstr[0] = cinput;
                cinputstr[1] = 0;
                m_converter->Convert( cinputstr, wcstr );
                wc = wcstr[0];
#else
                wc = m_converter->Convert( wxString( cinput ) ).Last();
#endif
            }
            FT_ULong c = ( FT_ULong ) wc;
            int error;
            FT_UInt index = FT_Get_Char_Index( m_face, ( FT_ULong ) c );
            /* load the glyph image (in its native format) */
            error = FT_Load_Glyph( m_face, index, m_loadMethodMask );
            wxASSERT_MSG( error == 0 , _( "Glyph could not be loaded" ) );
            if ( !error )
            {
                error = FT_Get_Glyph( m_face->glyph, &glyph );
                wxASSERT_MSG( error == 0 , _( "Glyph could not be get" ) );
                if ( error )
                    glyph = NULL;
            }
            if ( glyph )
                glyph->advance = m_face->glyph->advance;

            if ( !glyph )
            {
                glyph = ( FT_Glyph ) - 1;
                if ( c != wxChar( '?' ) )
                {
                    const a2dGlyph* glyphErr = GetGlyph( wxChar( '?' ) );
                    if( glyphErr )
                    {
                        index = glyphErr->m_index;
                        glyph = glyphErr->m_glyph;
                    }
                }
            }
            a2dglyph->m_index = index;
            a2dglyph->m_glyph = glyph;
            a2dglyph->m_lsb = m_face->glyph->lsb_delta;
            a2dglyph->m_rsb = m_face->glyph->rsb_delta;
            a2dglyph->m_face = m_face;
        }
        if ( glyph == ( FT_Glyph ) - 1 )
            a2dglyph = NULL;

        m_lastChar = cinput;
        m_a2dLastGlyph = a2dglyph;

        return a2dglyph;
    }
#else // wxART2D_USE_FREETYPE
    return NULL;
#endif // wxART2D_USE_FREETYPE
}

void a2dFontFreetypeData::GetTextExtent( const wxString& string, double& w,
        double& h, double& descent, double& externalLeading ) const
{
    if ( m_ok )
    {
        const a2dGlyph* a2dglyphprev = NULL;
        double x = 0.0;
        double dxstartrel = 0.0;
#if wxUSE_UNICODE
        const wxStringCharType *cp(string.wx_str());
#else
        const wxChar* cp(string.c_str());
#endif // wxUSE_UNICODE
        for ( size_t n = 0; n < string.Length(); n++ )
        {
            wxChar c = *cp++;
            const a2dGlyph* a2dglyph = GetGlyph( c );
            // The glyph is now grid fitted and hinted.
            if( !a2dglyph )
                return;

            if ( n > 0 )
            {
                FT_Face  face = a2dglyph->m_face;
                FT_Vector  kern;
                FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
                x += kern.x / 64.0;

                if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                    x -= 1;
                else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                    x += 1;

            }
            else
            {            
                FT_Glyph glyphimage = a2dglyph->m_glyph;
                FT_BBox  bbox;
                FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
                dxstartrel = bbox.xMin;
            }

            a2dglyphprev = a2dglyph;
            FT_Glyph glyphimage = a2dglyph->m_glyph;
            x += glyphimage->advance.x / 64.0;
        }
        if ( m_sizeInPixels == NORMFONT )
            x += 1; //bbox is a bit to small after hinting at device level, we add a view normalized pixel here.
        else
            x += 1; //bbox is a bit to small after hinting at device level, we add a X pixels here.
        // scale = GetSize() / m_sizeInPixels;
        w = GetSize() * ( x - dxstartrel ) / m_sizeInPixels;
        h = GetSize() * m_height;
        descent = GetSize() * m_desc;
        externalLeading = GetSize() * m_lead;
    }
    else
        w = h = descent = externalLeading = 0.0;

    /* less accurate because fo rounding
        if ( m_ok )
        {
            double x = 0.0;
            //double y = 0.0;
            for ( size_t n = 0; n < string.Length(); n++ )
            {
                if ( n > 0 )
                    x += GetKerning( string[n-1], string[n] );
                x += GetWidth( string[n] );
                //y = max( GetGlyphHeight( string[n] ), y );
            }
            w = x;
            //h = y;
            h = GetSize() * m_height;
            descent = GetSize() * m_desc;
            externalLeading = GetSize() * m_lead;
        }
        else
            w = h = descent = externalLeading = 0.0;
    */
}

double a2dFontFreetypeData::GetWidth( wxChar c ) const
{
#if wxART2D_USE_FREETYPE
    if ( m_face )
    {
        const a2dGlyph* a2dglyph = GetGlyph( c );
        if( a2dglyph )
        {
            FT_Glyph glyphimage = a2dglyph->m_glyph;
            return glyphimage->advance.x *  m_fontinfo.GetSize() / ( 64.0 * m_sizeInPixels );
        }
    }
    return 0.0;
#else //wxART2D_USE_FREETYPE
    // reasonable estimate
    return m_fontinfo.GetSize() * 0.25;
#endif //wxART2D_USE_FREETYPE
}

double a2dFontFreetypeData::GetGlyphHeight( wxChar c ) const
{
#if wxART2D_USE_FREETYPE
    if ( m_face )
    {
        const a2dGlyph* a2dglyph = GetGlyph( c );
        if( a2dglyph )
        {
            FT_Glyph glyphimage = a2dglyph->m_glyph;
            return m_face->glyph->metrics.height *  m_fontinfo.GetSize() / ( 64.0 * m_sizeInPixels );
        }
    }
    return 0.0;
#else //wxART2D_USE_FREETYPE
    // reasonable estimate
    return m_fontinfo.GetSize() * 0.25;
#endif //wxART2D_USE_FREETYPE
}

double a2dFontFreetypeData::GetKerning( wxChar cinput1, wxChar cinput2 ) const
{
#if wxART2D_USE_FREETYPE
    FT_Vector  kern;
    if ( !m_face )
        return 0.0;

    const a2dGlyph* a2dglyph1 = GetGlyph( cinput1 );
    if( !a2dglyph1 )
        return 0.0;
    const a2dGlyph* a2dglyph2 = GetGlyph( cinput2 );
    if( !a2dglyph2 )
        return 0.0;

    //if ( FT_Get_Kerning( m_face, a2dglyph1->m_index, a2dglyph2->m_index, FT_KERNING_UNFITTED, &kern ) )
    if ( FT_Get_Kerning( m_face, a2dglyph1->m_index, a2dglyph2->m_index, FT_KERNING_DEFAULT, &kern ) )
        return 0.0;
    else
    {
        double rsblsbshift = 0;

        if ( a2dglyph1->m_rsb - a2dglyph2->m_lsb >= 32 )
            rsblsbshift -= 64;
        else if (  a2dglyph1->m_rsb - a2dglyph2->m_lsb < -32 )
            rsblsbshift += 64;

        return ( ( rsblsbshift + kern.x ) * m_fontinfo.GetSize() ) / ( 64.0 * m_sizeInPixels );
    }
#else //wxART2D_USE_FREETYPE
    return 0.0;
#endif //wxART2D_USE_FREETYPE
}

static double fscale = 0;
static double fx = 0;
static double fy = 0;

int MoveToFunc( const FT_Vector*  to,  void* user )
{
    a2dVpath* vpath = ( a2dVpath* ) user;
    if ( vpath->size() )
        vpath->back()->SetClose( a2dPATHSEG_END_CLOSED );
    vpath->MoveTo( fx + fscale * to->x , fy + fscale * to->y );
    return 0;
}

int LineToFunc( const FT_Vector*  to,  void* user )
{
    a2dVpath* vpath = ( a2dVpath* ) user;
    vpath->LineTo( fx + fscale * to->x , fy + fscale * to->y );

    return 0;
}

int ConicToFunc( const FT_Vector*  control,
                 const FT_Vector*  to,
                 void*             user )
{
    a2dVpath* vpath = ( a2dVpath* ) user;
    vpath->QBCurveTo( fx + fscale * to->x, fy + fscale * to->y, fx + fscale * control->x, fy + fscale * control->y );

    return 0;
}

int CubicToFunc( const FT_Vector*  control1,
                 const FT_Vector*  control2,
                 const FT_Vector*  to,
                 void*             user )
{
    a2dVpath* vpath = ( a2dVpath* ) user;
    //vpath->LineTo( fx + fscale*to->x , fy + fscale*to->y );
    vpath->CBCurveTo( fx + fscale * to->x, fy + fscale * to->y, fx + fscale * control2->x, fy + fscale * control2->y,
                      fx + fscale * control2->x, fy + fscale * control2->y );

    return 0;
}


bool a2dFontFreetypeData::GetVpath( wxChar c, a2dVpath& vpath, const a2dAffineMatrix& affine )
{
#if wxART2D_USE_FREETYPE
    double x = affine.GetValue( 2, 0 );
    double y = affine.GetValue( 2, 1 );
    y += GetDescent();

    double scale = GetSize() / ( 64.0 * m_sizeInPixels );

    const a2dGlyph* a2dglyph = GetGlyph( c );
    if( !a2dglyph )
        return false;
    FT_Glyph glyph = a2dglyph->m_glyph;
    if ( glyph->format != FT_GLYPH_FORMAT_OUTLINE )
        return false;
    FT_Outline& outline = ( ( FT_OutlineGlyph ) glyph )->outline;

    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  point;
    FT_Vector*  limit;
    char*       tags;

    int   n;         // index of contour in outline
    int   first;     // index of first point in contour
    char  tag;       // current point's state

    first = 0;

    /*
        FT_Outline_Funcs segmentCallBack;
        segmentCallBack.move_to = &::MoveToFunc;
        segmentCallBack.line_to = &::LineToFunc;
        segmentCallBack.conic_to = &::ConicToFunc;
        segmentCallBack.cubic_to = &::CubicToFunc;
        segmentCallBack.shift = 0;
        segmentCallBack.delta = 0;
        fscale = scale;
        fx = x;
        fy = y;

        FT_Outline_Decompose( &outline, &segmentCallBack, &vpath );
        a2dAffineMatrix affinesize;
        affinesize.Scale( scale, scale, 0, 0 );

    */
    //as in FT_Outline_Decompose, but for wxArt2D
    for( n = 0; n < outline.n_contours; n++ )
    {
        int  last;  // index of last point in contour

        last  = outline.contours[n];
        limit = outline.points + last;

        v_start = outline.points[first];
        v_last  = outline.points[last];

        v_control = v_start;

        point = outline.points + first;
        tags  = outline.tags  + first;
        tag   = FT_CURVE_TAG( tags[0] );

        // A contour cannot start with a cubic control point!
        if( tag == FT_CURVE_TAG_CUBIC ) return false;

        // check first point to determine origin
        if( tag == FT_CURVE_TAG_CONIC )
        {
            // first point is conic control.  Yes, this happens.
            if( FT_CURVE_TAG( outline.tags[last] ) == FT_CURVE_TAG_ON )
            {
                // start at last point if it is on the curve
                v_start = v_last;
                limit--;
            }
            else
            {
                // if both first and last points are conic,
                // start at their middle and record its position
                // for closure
                v_start.x = ( v_start.x + v_last.x ) / 2;
                v_start.y = ( v_start.y + v_last.y ) / 2;

                v_last = v_start;
            }
            point--;
            tags--;
        }

        vpath.MoveTo( x + scale * v_start.x, y + scale * v_start.y );

        while( point < limit )
        {
            point++;
            tags++;

            tag = FT_CURVE_TAG( tags[0] );
            switch( tag )
            {
                case FT_CURVE_TAG_ON:  // emit a single line_to
                {
                    vpath.LineTo( x + scale * point->x, y + scale * point->y );
                    continue;
                }

                case FT_CURVE_TAG_CONIC:  // consume conic arcs
                {
                    v_control.x = point->x;
                    v_control.y = point->y;

Do_Conic:
                    if( point < limit )
                    {
                        FT_Vector vec;
                        FT_Vector v_middle;

                        point++;
                        tags++;
                        tag = FT_CURVE_TAG( tags[0] );

                        vec.x = point->x;
                        vec.y = point->y;

                        if( tag == FT_CURVE_TAG_ON )
                        {
                            //vpath.LineTo( x + scale * vec.x, y + scale * vec.y );
                            vpath.QBCurveTo( x + scale * vec.x, y + scale * vec.y, x + scale * v_control.x, y + scale * v_control.y );
                            continue;
                        }

                        if( tag != FT_CURVE_TAG_CONIC ) return false;

                        v_middle.x = ( v_control.x + vec.x ) / 2;
                        v_middle.y = ( v_control.y + vec.y ) / 2;

                        vpath.QBCurveTo( x + scale * v_middle.x, y + scale * v_middle.y, x + scale * v_control.x, y + scale * v_control.y  );

                        v_control = vec;
                        goto Do_Conic;
                    }
                    vpath.QBCurveTo( x + scale * v_start.x, y + scale * v_start.y, x + scale * v_control.x, y + scale * v_control.y );
                    vpath.Close();
                    goto Close;
                }

                default:  // FT_CURVE_TAG_CUBIC
                {
                    FT_Vector vec1, vec2;

                    if( point + 1 > limit || FT_CURVE_TAG( tags[1] ) != FT_CURVE_TAG_CUBIC )
                    {
                        return false;
                    }

                    vec1.x = point[0].x;
                    vec1.y = point[0].y;
                    vec2.x = point[1].x;
                    vec2.y = point[1].y;

                    point += 2;
                    tags  += 2;

                    if( point <= limit )
                    {
                        FT_Vector vec;

                        vec.x = point->x;
                        vec.y = point->y;

                        vpath.CBCurveTo( x + scale * vec1.x, y + scale * vec1.y,
                                         x + scale * vec2.x, y + scale * vec2.y,
                                         x + scale * vec.x, y + scale * vec.y );
                        continue;
                    }

                    vpath.CBCurveTo( x + scale * vec1.x, y + scale * vec1.y,
                                     x + scale * vec2.x, y + scale * vec2.y,
                                     x + scale * v_start.x, y + scale * v_start.y );
                    vpath.Close();
                    goto Close;
                }
            }
        }

        vpath.LineTo( x + scale * v_start.x, y + scale * v_start.y, true );
        vpath.Close();

Close:
        first = last + 1;
    }
    return true;
#else //wxART2D_USE_FREETYPE
    return false;
#endif //wxART2D_USE_FREETYPE
}

void a2dFontFreetypeData::GetInfoList( a2dFontInfoList& list )
{
#if wxART2D_USE_FREETYPE
    wxPathList& pathlist = a2dGlobals->GetFontPathList();
#if wxCHECK_VERSION(2,7,0)
    for ( wxPathList::iterator node = pathlist.begin(); node != pathlist.end(); node++ )
#else
    for ( wxPathList::compatibility_iterator node = pathlist.GetFirst(); node; node = node->GetNext() )
#endif
    {
        wxString path( node->GetData() );
        if ( !wxDir::Exists( path ) )
            continue;
        wxArrayString files;
        wxDir::GetAllFiles( path, &files, wxT( "*.ttf" ), wxDIR_FILES );
        for ( unsigned int i = 0; i < files.GetCount(); i++ )
        {
            FT_Error error;
            FT_Face face;
            error = FT_New_Face( g_freetypeLibrary, files.Item( i ).mb_str(), 0, &face );
            if ( error )
                continue;
            list.Append( new a2dFontInfo( wxT( "Freetype font" ),
                                          wxString::FromAscii( face->family_name ),
                                          wxString::FromAscii( face->style_name ),
                                          1.0, files.Item( i ) ) );
            FT_Done_Face( face );
        }
    }
#else
    wxUnusedVar( list );
#endif
}

a2dFontFreetypeData* a2dFontFreetypeData::CreateFont( const a2dFontInfo& info, bool force )
{

#if wxART2D_USE_FREETYPE
    if ( force || info.GetType().Upper() == wxT( "FREETYPE FONT" ) )
    {
        a2dFontFreetypeData* font = NULL;
        if ( !force && !info.GetExtra().IsEmpty() )
        {
            // try to open supplied filename
            font = new a2dFontFreetypeData( info.GetExtra(), info.GetSize() );
            if ( font )
            {
                if ( font->Ok() )
                    return font;
                else
                {
                    delete font;
                    return NULL;
                }
            }
        }
        a2dFontInfoList list;
        list.DeleteContents( true );
        GetInfoList( list );
        unsigned int i;
        // try to find exact fontname
        for ( i = 0; i < list.GetCount(); i++ )
        {
            a2dFontInfo* fi = list.Item( i )->GetData();
            if ( fi->GetName() == info.GetName() && fi->GetStyle() == info.GetStyle() )
            {
                font = new a2dFontFreetypeData( fi->GetExtra(), info.GetSize() );
                if ( font )
                {
                    if ( font->Ok() )
                        return font;
                    else
                        delete font;
                }
            }
        }
        font = NULL;

        int maxval = 0;
        // try to find similar font
        for ( i = 0; i < list.GetCount(); i++ )
        {
            a2dFontInfo* fi = list.Item( i )->GetData();
            int val = 0;
            // if fontname is part of fi.name ore fi.name = part of fontname.
            if ( fi->GetName() == info.GetName() )
                val = 4;
            else if ( fi->GetName().Upper() == info.GetName().Upper() )
                val = 3;
            else if ( ( fi->GetName().Upper().Find( info.GetName().Upper() ) != -1 )
                      || ( info.GetName().Upper().Find( fi->GetName().Upper() ) != -1 ) )
                val = 1;

            if ( val )
            {
                if ( ( info.GetStyle().Upper() == wxT( "BOLD" ) )
                        || ( info.GetStyle().Upper().Find( wxT( "BOLD" ) ) != -1 ) )
                    val++;
                if ( ( info.GetStyle().Upper() == wxT( "ITALIC" ) )
                        || ( info.GetStyle().Upper().Find( wxT( "ITALIC" ) ) != -1 ) )
                    val++;
                if ( ( info.GetStyle().Upper() == wxT( "REGULAR" ) )
                        || ( info.GetStyle().Upper().Find( wxT( "REGULAR" ) ) != -1 ) )
                    val++;

                if ( val > maxval )
                {
                    a2dFontFreetypeData* oldfont = font;
                    font = new a2dFontFreetypeData( fi->GetExtra(), info.GetSize() );
                    if ( font )
                    {
                        if ( font->Ok() )
                        {
                            maxval = val;
                            delete oldfont;
                        }
                        else
                        {
                            delete font;
                            font = oldfont;
                        }
                    }
                }
            }
        }
        if ( font )
            return font;
    }
#endif
    return NULL;
}

#endif

//----------------------------------------------------------------------------
// a2dFontStrokeData
//----------------------------------------------------------------------------

//! Stroke font class.
/*! \class a2dFontStroke stylebase.h artbase/stylebase.h
    Font class based on BGI fonts (*.CHR). These font once were part of the Borland
    BGI Toolkit. They can be found at several places on the internet. Look for bgifonts.zip
    and bgifont.zip. The latter also contains an editor for these fonts.

    \todo EW: Translate the native text encoding / unicode to codepage 850 for a2dFontStroke, since these are 'old' DOS fonts.
    \ingroup property style
    \ingroup text
*/
class A2DARTBASEDLLEXP a2dFontStrokeData
    : public a2dFontRefData
{
    friend class a2dFont;
public:

    virtual a2dFontType GetType( void ) const { return a2dFONT_STROKED; }

    //! Constructor.
    /*! \param filename Filename of the vector font.
        \param size Size in world coordinates.
        \param monoSpaced if true, all character will have eqaul distance
    */
    a2dFontStrokeData( const wxString& filename = wxT( "" ), double size = 0.0, bool monoSpaced = false, double weight = 0.0 );

    //! Copy constructor.
    a2dFontStrokeData( const a2dFontStrokeData& other );
    //! Destructor.
    virtual ~a2dFontStrokeData();

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
    inline double GetWeight() const { return m_weight; }
    //! Set the weight of the font.
    /*! \param weight The weight (in this case stroke width) in percentages of the font size. There are two exceptions.
        0: Draws the font with a pen of 1 pixel wide. -1: Draws the font with a normalized pen, as the font-designer intended it.
    */
    void SetWeight( double weight );

    //! Set the weight of the font, from a string.
    /*! \param weight One of: "Normalized Pen", "Pixel pen" or a string starting with the width in percentages, ending with %.
    */
    void SetWeight( const wxString& weight );

    // Documented in base class
    double GetWidth( wxChar c ) const;

    // Documented in base class
    double GetKerning( wxChar c1, wxChar c2 ) const;

    //! Get stroke width.
    /*!
        \return The stroke width in world coordinates. In case of a pixel font, 0.0 is returned.
        \sa GetWeight()
    */
    double GetStrokeWidth() const;

    //! Get the glyph of a single character.
    /*! \return The glyph of the character, in a coordinate space, normalized to the font size.
    */
    a2dStrokeGlyph GetGlyph( wxChar c ) const;

    // Documented in base class
    virtual bool GetVpath( wxChar c, a2dVpath& glyph, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX );

    // Documented in base class
    static void GetInfoList( a2dFontInfoList& list );

    // Documented in base class
    static a2dFontStrokeData* CreateFont( const a2dFontInfo& info, bool force = false );

protected:
    //! Filename of the font.
    wxString m_filename;

    //! Width storage.
    wxUint16* m_width;
    //! Glyph storage.
    a2dStrokeGlyph* m_characters;

    //! First character in font.
    wxUint16 m_beginchar;
    //! Number of characters in font.
    wxUint16 m_count;
    //! Font size in internal coordinates
    wxUint16 m_intsize;
    //! Font weight
    double m_weight;

    bool m_monoSpaced;
};

#define BGI_FONT_MAGIC "PK\b\bBGI "

#pragma pack(1)
//! \cond

// begin of a BGI font ( used in a2dFontStrokeData )
typedef struct
{
    char magic[8];
    char descr[108];
    char stringend;
    unsigned short headersize;
    char fname[5];
    unsigned short datasize;
    unsigned char majorversion, minorversion;
    unsigned char minmajorversion, minminorversion;
}
BGI_FONT_HEADER;

//! \cond
typedef struct
{
    char signature;
    unsigned short nchar;
    char undefined;
    unsigned char firstchar;
    unsigned short stroke_offset;
    char scan_flag;
    signed char org_top;
    signed char org_base;
    signed char org_desc;
    char unused[5];
}
BGI_FONT_DATA;
#pragma pack()
//! \endcond

a2dFontStrokeData::a2dFontStrokeData( const wxString& filename, double size, bool monoSpaced, double weight )
    : a2dFontRefData( size )
{
    m_monoSpaced = monoSpaced;
    m_fontinfo.SetType( wxT( "Stroke font" ) );
    m_width = NULL;
    m_characters = NULL;
    SetWeight( weight );
    SetFilename( filename );
}

a2dFontStrokeData::a2dFontStrokeData( const a2dFontStrokeData& other )
    : a2dFontRefData( other )
{
    m_fontinfo.SetType( wxT( "Stroke font" ) );
    m_width = NULL;
    m_weight = other.m_weight;
    m_characters = NULL;
    m_monoSpaced = other.m_monoSpaced;
    SetFilename( other.m_filename );
}

a2dFontStrokeData::~a2dFontStrokeData()
{
    if ( m_width )
        delete[] m_width;
    if ( m_characters )
    {
        for ( unsigned char i = 0; i < m_count; i++ )
        {
            a2dStrokeGlyph glyph = m_characters[i];
            while ( *glyph )
            {
                delete * glyph;
                glyph++;
            }
            free( m_characters[i] );
        }
        free( m_characters );
    }
}

wxString a2dFontStrokeData::GetFilename( bool filenameonly ) const
{
    if ( filenameonly )
    {
        wxFileName filename( m_filename );
        return filename.GetFullName();
    }
    else
        return m_filename;
}

void a2dFontStrokeData::SetFilename( const wxString& filename )
{
    m_ok = false;

    // delete previous font storage
    if ( m_width )
    {
        delete[] m_width;
        m_width = NULL;
    }
    if ( m_characters )
    {
        for ( unsigned char i = 0; i < m_count; i++ )
        {
            a2dStrokeGlyph glyph = m_characters[i];
            while ( *glyph )
            {
                delete * glyph;
                glyph++;
            }
            free( m_characters[i] );
        }
        free( m_characters );
        m_characters = NULL;
    }

    m_filename = filename;
    if ( m_filename.IsEmpty() )
        return;

    wxString fname = a2dGlobals->GetFontPathList().FindValidPath( m_filename );

    if ( fname.IsEmpty() )
        fname = m_filename; //make sure we get a name to at least say that this filename could not be found.

    wxFile file( fname );
    if ( !file.IsOpened() )
        return;

    BGI_FONT_HEADER bgifontheader;
    file.Read( &bgifontheader.magic, sizeof( bgifontheader.magic ) );
    size_t idx = 0;
    char c;
    file.Read( &c, 1 );
    while( !file.Eof() && idx < sizeof( bgifontheader.descr ) - 1 && c != 0x1a )
    {
        bgifontheader.descr[idx++] = c;
        file.Read( &c, 1 );
    }
    while( !file.Eof() && c != 0x1a )
        file.Read( &c, 1 );
    bgifontheader.descr[idx] = '\0';
    file.Read( &bgifontheader.headersize, sizeof( bgifontheader.headersize ) );
    // swapping for big-endian systems.
    bgifontheader.headersize = wxINT16_SWAP_ON_BE( bgifontheader.headersize );
    file.Read( &bgifontheader.fname, 4 );
    bgifontheader.fname[4] = '\0';
    file.Read( &bgifontheader.datasize, sizeof( bgifontheader.datasize ) );
    // swapping for big-endian systems.
    bgifontheader.datasize = wxINT16_SWAP_ON_BE( bgifontheader.datasize );
    file.Read( &bgifontheader.majorversion, 4 * sizeof( bgifontheader.majorversion ) );

    if ( memcmp( bgifontheader.magic, BGI_FONT_MAGIC, sizeof( bgifontheader.magic ) ) )
    {
        file.Close();
        return;
    }
    file.Seek( bgifontheader.headersize, wxFromStart );

    BGI_FONT_DATA bgifontdata;
    file.Read( &bgifontdata, sizeof( BGI_FONT_DATA ) );
    // swapping for big-endian systems.
    bgifontdata.nchar = wxUINT16_SWAP_ON_BE( bgifontdata.nchar );
    bgifontdata.stroke_offset = wxUINT16_SWAP_ON_BE( bgifontdata.stroke_offset );

    if ( bgifontdata.signature  != '+' )
    {
        file.Close();
        return;
    }

    m_count = bgifontdata.nchar;
    m_beginchar = bgifontdata.firstchar;

    m_intsize = bgifontdata.org_top - bgifontdata.org_base;
    double size = m_intsize;
    m_height = 1.11;
    //was
    //m_height = ( m_intsize - 2 * ( bgifontdata.org_desc - bgifontdata.org_base ) + 3 ) / size;
    m_desc = ( bgifontdata.org_base - bgifontdata.org_desc + 1.5 ) / size;
    m_lead = 0.0;

    // read width table.
    //m_width = new (unsigned char)[m_count];
    m_width = new wxUint16[m_count];
    file.Seek( bgifontheader.headersize + sizeof( BGI_FONT_DATA ) + sizeof( unsigned short ) * ( bgifontdata.nchar ), wxFromStart );
    unsigned int i;
    unsigned char w;
    for ( i = 0; i < m_count; i++ )
    {
        file.Read( &w, sizeof( unsigned char ) );
        m_width[i] = w;
    }

    // create character table
    m_characters = ( a2dStrokeGlyph* ) malloc( sizeof( a2dStrokeGlyph ) * m_count );
    for ( i = 0; i < m_count; i++ )
    {
        // read offset for character definition
        file.Seek( bgifontheader.headersize + sizeof( BGI_FONT_DATA ) + sizeof( unsigned short ) * i, wxFromStart );
        unsigned short char_offset;
        file.Read( &char_offset, sizeof( char_offset ) );
        // swapping for big-endian systems.
        char_offset = wxUINT16_SWAP_ON_BE( char_offset );
        // goto offset for character definition
        file.Seek( bgifontheader.headersize + bgifontdata.stroke_offset + char_offset, wxFromStart );

        bool penup = true;
        unsigned char opcode = 1;
        a2dVertexList* list = new a2dVertexList;
        int nlists = 0;
        m_characters[i] = ( a2dStrokeGlyph ) malloc( sizeof( a2dVertexList* ) );
        m_characters[i][0] = NULL;
        signed char penupx = 0;
        signed char penupy = 0;
        while ( opcode != 0 )
        {
            signed char x, y;
            file.Read( &x, sizeof( x ) );
            file.Read( &y, sizeof( y ) );
            if ( x & 0x80 ) opcode |= 0x2; else opcode &= ~0x2;
            if ( y & 0x80 ) opcode |= 0x1; else opcode &= ~0x1;
            if ( x & 0x40 ) x |= 0x80; else x &= ~0x80;
            if ( y & 0x40 ) y |= 0x80; else y &= ~0x80;

            switch( opcode )
            {
                case 3:
                    if ( penup )
                        list->push_back( new a2dLineSegment( penupx / size, ( penupy - bgifontdata.org_base ) / size + m_desc ) );

                    penup = false;
                    list->push_back( new a2dLineSegment( x / size, ( y - bgifontdata.org_base ) / size + m_desc ) );
                    break;
                case 2:
                    if ( !penup )
                    {
                        m_characters[i] = ( a2dStrokeGlyph ) realloc( m_characters[i], sizeof( a2dVertexList* ) * ( nlists + 2 ) );
                        m_characters[i][nlists + 1] = NULL;
                        m_characters[i][nlists] = list;
                        nlists++;
                        list = new a2dVertexList;
                    }
                    penup = true; penupx = x; penupy = y;
                    break;
                case 1:
                    break;
                default:
                    m_characters[i] = ( a2dStrokeGlyph ) realloc( m_characters[i], sizeof( a2dVertexList* ) * ( nlists + 2 ) );
                    m_characters[i][nlists + 1] = NULL;
                    m_characters[i][nlists] = list;
                    nlists++;
            }
        }
    }
    file.Close();
    m_ok = true;

    // rename known fonts.
    wxString fontname = wxString::FromAscii( bgifontheader.fname );
    if ( fontname == wxT( "EURO" ) )
        fontname = wxT( "EuroStyle" );
    else if ( fontname == wxT( "GOTH" ) )
        fontname = wxT( "Gothic" );
    else if ( fontname == wxT( "LCOM" ) )
        fontname = wxT( "Complex" );
    else if ( fontname == wxT( "LITT" ) )
        fontname = wxT( "Small" );
    else if ( fontname == wxT( "SANS" ) )
        fontname = wxT( "Sans serif" );
    else if ( fontname == wxT( "SCRI" ) )
        fontname = wxT( "Script" );
    else if ( fontname == wxT( "SIMP" ) )
        fontname = wxT( "Simple" );
    else if ( fontname == wxT( "TRIP" ) )
        fontname = wxT( "Triplex" );
    else if ( fontname == wxT( "TSCR" ) )
        fontname = wxT( "Triplex Script" );
    else if ( fontname == wxT( "NEN " ) )
        fontname = wxT( "NEN 3094" );
    m_fontinfo.SetName( fontname );

    m_fontinfo.SetExtra( GetFilename( true ) );
}

void a2dFontStrokeData::SetWeight( double weight )
{
    m_weight = weight;

    if ( m_weight == 0.0 )
        m_fontinfo.SetStyle( wxT( "Pixel Pen" ) );
    if ( m_weight == -1.0 )
        m_fontinfo.SetStyle( wxT( "Normalized Pen" ) );
    else
    {
        wxString style;
        style.Printf( wxT( "%.1f\x25 of height" ), m_weight );
        m_fontinfo.SetStyle( style );
    }
}

void a2dFontStrokeData::SetWeight( const wxString& weight )
{
    m_fontinfo.SetStyle( weight );
    if ( weight.Upper() == wxT( "PIXEL PEN" ) )
    {
        m_weight = 0.0;
    }
    else if ( weight.Upper() == wxT( "NORMALIZED PEN" ) )
    {
        m_weight = -1.0;
    }
    else
    {
        int idxend = weight.Find( wxChar( '%' ) );
        if ( idxend <= 0 )
            m_weight = -1;
        else
        {
            size_t idxstart = idxend;
            while ( idxstart > 0 && weight[idxstart - 1] == wxChar( ' ' ) )
                idxstart--;
            while ( idxstart > 0 && weight[idxstart - 1] != wxChar( ' ' ) )
                idxstart--;
            weight.Mid( idxstart, idxend - idxstart ).ToCDouble( &m_weight );
        }
    }
}

double a2dFontStrokeData::GetWidth( wxChar c ) const
{
    if ( !m_ok || !m_width )
        return 0.0;

    unsigned char c2 = c;
    if ( m_monoSpaced )
        c2 = 'm'; // this character seems to represent best a monospaced width that fits all other.

    if ( c2 < m_beginchar && c2 >= m_beginchar + m_count )
        c2 = '?';

    if ( c2 >= m_beginchar && c2 < m_beginchar + m_count )
    {
        double width = m_fontinfo.GetSize() * m_width[c2 - m_beginchar] / m_intsize;
        if ( m_monoSpaced )
            return width * 0.83;
        return width;
    }
    return 0.0;
}

double a2dFontStrokeData::GetStrokeWidth() const
{
    if ( !m_ok )
        return 0.0;

    if ( m_weight == -1.0 )
        return m_fontinfo.GetSize() * 1.25 / m_intsize;
    else if ( m_weight == 0.0 )
        return 0.0;
    else
        return m_fontinfo.GetSize() * m_weight * 0.01;
}

double a2dFontStrokeData::GetKerning( wxChar WXUNUSED( c1 ), wxChar WXUNUSED( c2 ) ) const
{
    // no kerning.
    return 0.0;
}

a2dStrokeGlyph a2dFontStrokeData::GetGlyph( wxChar c ) const
{
    if ( !m_ok || !m_characters )
        return NULL;

    unsigned char c2 = c;

    if ( c2 >= m_beginchar && c2 < m_beginchar + m_count )
        return m_characters[c2 - m_beginchar];
    else
    {
        c2 = '?';
        if ( c2 >= m_beginchar && c2 < m_beginchar + m_count )
            return m_characters[c2 - m_beginchar];
        else
            return NULL;
    }
}


bool a2dFontStrokeData::GetVpath( wxChar c, a2dVpath& vpath, const a2dAffineMatrix& affine )
{
    a2dVertexList** ptr = GetGlyph( c );

    // mirror text, depending on y-axis orientation
    a2dAffineMatrix affine2;
    // position glyph
    affine2.Scale( GetSize() );

    a2dAffineMatrix affine3 = affine;
    affine3 *= affine2;

    if ( ptr )
    {
        while ( *ptr )
        {
            a2dVertexList trans = a2dVertexList( **ptr );
            trans.Transform( affine3 );
            vpath.Add( trans, true, false );
            ptr++;
        }
    }
    return true;
}

void a2dFontStrokeData::GetInfoList( a2dFontInfoList& list )
{
    wxPathList& pathlist = a2dGlobals->GetFontPathList();
#if wxCHECK_VERSION(2,7,0)
    for ( wxPathList::iterator node = pathlist.begin(); node != pathlist.end(); node++ )
#else
    for ( wxPathList::compatibility_iterator node = pathlist.GetFirst(); node; node = node->GetNext() )
#endif
    {
        wxString path( node->GetData() );
        if ( !wxDir::Exists( path ) )
            continue;

        wxArrayString files;
        wxDir::GetAllFiles( path, &files, wxT( "*.chr" ), wxDIR_FILES );
        for ( unsigned int i = 0; i < files.GetCount(); i++ )
        {
            // check if file actually is a stroke font
            wxFile file( files.Item( i ) );
            if ( !file.IsOpened() )
                continue;

            BGI_FONT_HEADER bgifontheader;
            file.Read( &bgifontheader.magic, sizeof( bgifontheader.magic ) );
            size_t idx = 0;
            char c;
            file.Read( &c, 1 );
            while( !file.Eof() && idx < sizeof( bgifontheader.descr ) - 1 && c != 0x1a )
            {
                bgifontheader.descr[idx++] = c;
                file.Read( &c, 1 );
            }
            while( !file.Eof() && c != 0x1a )
                file.Read( &c, 1 );
            bgifontheader.descr[idx] = '\0';
            file.Read( &bgifontheader.headersize, sizeof( bgifontheader.headersize ) );
            // swapping for big-endian systems.
            bgifontheader.headersize = wxINT16_SWAP_ON_BE( bgifontheader.headersize );
            file.Read( &bgifontheader.fname, 4 );
            bgifontheader.fname[4] = '\0';
            file.Read( &bgifontheader.datasize, sizeof( bgifontheader.datasize ) );
            // swapping for big-endian systems.
            bgifontheader.datasize = wxINT16_SWAP_ON_BE( bgifontheader.datasize );
            file.Read( &bgifontheader.majorversion, 4 * sizeof( bgifontheader.majorversion ) );

            if ( memcmp( bgifontheader.magic, BGI_FONT_MAGIC, sizeof( bgifontheader.magic ) ) )
            {
                file.Close();
                continue;
            }
            file.Seek( bgifontheader.headersize, wxFromStart );

            BGI_FONT_DATA bgifontdata;
            file.Read( &bgifontdata, sizeof( BGI_FONT_DATA ) );
            // swapping for big-endian systems.
            bgifontdata.nchar = wxUINT16_SWAP_ON_BE( bgifontdata.nchar );
            bgifontdata.stroke_offset = wxUINT16_SWAP_ON_BE( bgifontdata.stroke_offset );

            if ( bgifontdata.signature  != '+' )
            {
                file.Close();
                continue;
            }

            // rename known fonts.
            wxString fontname = wxString::FromAscii( bgifontheader.fname );
            if ( fontname == wxT( "EURO" ) )
                fontname = wxT( "EuroStyle" );
            else if ( fontname == wxT( "GOTH" ) )
                fontname = wxT( "Gothic" );
            else if ( fontname == wxT( "LCOM" ) )
                fontname = wxT( "Complex" );
            else if ( fontname == wxT( "LITT" ) )
                fontname = wxT( "Small" );
            else if ( fontname == wxT( "SANS" ) )
                fontname = wxT( "Sans serif" );
            else if ( fontname == wxT( "SCRI" ) )
                fontname = wxT( "Script" );
            else if ( fontname == wxT( "SIMP" ) )
                fontname = wxT( "Simple" );
            else if ( fontname == wxT( "TRIP" ) )
                fontname = wxT( "Triplex" );
            else if ( fontname == wxT( "TSCR" ) )
                fontname = wxT( "Triplex Script" );
            else if ( fontname == wxT( "NEN " ) )
                fontname = wxT( "NEN 3094" );

            wxFileName filename( files.Item( i ) );
            wxString weightstring;
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( "Pixel Pen" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( "Normalized Pen" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( " 2% of height" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( " 5% of height" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( " 7% of height" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( "10% of height" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( "15% of height" ), 1.0, filename.GetFullName() ) );
            list.Append( new a2dFontInfo( wxT( "Stroke font" ), fontname, wxT( "20% of height" ), 1.0, filename.GetFullName() ) );
        }
    }
}

a2dFontStrokeData* a2dFontStrokeData::CreateFont( const a2dFontInfo& info, bool force )
{
    if ( force || info.GetType().Upper() == wxT( "STROKE FONT" ) )
    {
        a2dFontStrokeData* font = NULL;
        if ( !force && !info.GetExtra().IsEmpty() )
        {
            // try to open supplied filename
            font = new a2dFontStrokeData( info.GetExtra(), info.GetSize() );
            if ( font )
            {
                font->SetWeight( info.GetStyle() );
                if ( font->Ok() )
                    return font;
                else
                    delete font;
            }
        }
        a2dFontInfoList list;
        list.DeleteContents( true );
        GetInfoList( list );
        unsigned int i;
        // try to find exact fontname
        for ( i = 0; i < list.GetCount(); i++ )
        {
            a2dFontInfo* fi = list.Item( i )->GetData();
            // if fontname is part of fi.name or fi.name = part of fontname.
            if ( fi->GetName() == info.GetName() )
            {
                font = new a2dFontStrokeData( fi->GetExtra(), info.GetSize() );
                if ( font )
                {
                    font->SetWeight( info.GetStyle() );
                    if ( font->Ok() )
                        return font;
                    else
                        delete font;
                }
            }
        }
        // try to find similar fontname
        for ( i = 0; i < list.GetCount(); i++ )
        {
            a2dFontInfo* fi = list.Item( i )->GetData();
            // if fontname is part of fi.name ore fi.name = part of fontname.
            if ( ( fi->GetName().Upper().Find( info.GetName().Upper() ) != -1 )
                    || ( info.GetName().Upper().Find( fi->GetName().Upper() ) != -1 ) )
            {
                font = new a2dFontStrokeData( fi->GetExtra(), info.GetSize() );
                if ( font )
                {
                    font->SetWeight( info.GetStyle() );
                    if ( font->Ok() )
                        return font;
                    else
                        delete font;
                }
            }
        }
    }
    return NULL;
}

//----------------------------------------------------------------------------
// a2dFont
//----------------------------------------------------------------------------

map< wxString, a2dFont > a2dFont::ms_fontCache;

IMPLEMENT_ABSTRACT_CLASS( a2dFont, wxObject )

a2dFont::a2dFont()
{
}

a2dFont::a2dFont( const a2dFont& other )
    : wxObject( other )
{
}

wxString a2dFont::CreateString( const wxString& filename, double size, wxFontEncoding encoding, int weight )
{
    wxFileName file( filename );  

    wxString typestr = wxT( "Freetype font" );
    wxString sizestr = wxString::FromCDouble( size );

    wxString style = wxEmptyString;
    switch ( weight )
    {
        case wxFONTWEIGHT_NORMAL:
            //if ( style == wxFONTSTYLE_NORMAL )
                style = wxT( "Regular" );
            break;
        case wxFONTWEIGHT_BOLD:
            style = wxT( "Bold " );
            break;
        case wxFONTWEIGHT_LIGHT:
            style = wxT( "Light " );
            break;
    }
    /*
    switch ( m_font.GetStyle() )
    {
        case wxFONTSTYLE_NORMAL:
            break;
        case wxFONTSTYLE_ITALIC:
        case wxFONTSTYLE_SLANT:
            style += wxT( "Italic" );
            break;
    }
    */
    style = style.Trim();

    wxString description = typestr + wxT( "-" ) + file.GetName() + wxT( "-" ) + style
        + wxT( "-" ) + sizestr + wxT( "--" ) + file.GetFullName();
    return description;
}

wxString a2dFont::CreateString( const wxString& filename, double size, bool monoSpaced, double weight )
{
    wxString typestr = wxT( "Stroke font" );
    wxString weightstr;

    if ( weight == 0.0 )
        weightstr = wxT( "Pixel Pen" );
    if ( weight == -1.0 )
        weightstr = wxT( "Normalized Pen" );
    else
        weightstr.Printf( wxT( "%.1f\x25 of height" ), weight );

    wxString monostr;
    if ( monoSpaced ) monostr = wxT( "monoSpaced" );
    wxString sizestr = wxString::FromCDouble( size );
    wxString description = typestr + wxT( "-" ) + filename + wxT( "-" ) + weightstr
                           + wxT( "-" ) + sizestr + wxT( "-" ) +  monostr ;
    return description;
}

wxString a2dFont::CreateString( a2dFontType type, const wxFont& font, double size, wxFontEncoding encoding )
{
    wxString name = font.GetFaceName();
    wxString style = wxEmptyString;

    wxString typestr;
    switch ( type )
    {
        case a2dFONT_WXDC:
            typestr = wxT( "Device font" );
            break;
        case a2dFONT_FREETYPE:
            typestr = wxT( "Freetype font" );
            break;
        default:
            wxFAIL_MSG( _( "wrong font to set a wxFont to" ) );
            break;
    }

    switch ( font.GetWeight() )
    {
        case wxNORMAL:
            if ( font.GetStyle() == wxFONTSTYLE_NORMAL )
                style = wxT( "Regular" );
            break;
        case wxBOLD:
            style = wxT( "Bold " );
            break;
        case wxLIGHT:
            style = wxT( "Light " );
            break;
    }
    switch ( font.GetStyle() )
    {
        case wxNORMAL:
            break;
        case wxITALIC:
        case wxSLANT:
            style += wxT( "Italic" );
            break;
    }
    style.Trim();

    wxString sizestr = wxString::FromCDouble( size );
    wxString description = typestr + wxT( "-" ) + name + wxT( "-" ) + style.Trim()
                           + wxT( "-" ) + sizestr;

    return description;
}

a2dFont::a2dFont( const wxFont& font, double size, a2dFontType type, wxFontEncoding encoding )
{
    wxString fontstr = CreateString( type, font, size, encoding );
    if ( ms_fontCache.find( fontstr ) == ms_fontCache.end() )
    {
        switch ( type )
        {
            case a2dFONT_WXDC:
                m_refData = new a2dFontDcData( font, size );
                break;
#if wxART2D_USE_FREETYPE
            case a2dFONT_FREETYPE:
#if defined(__USE_WINAPI__)
                m_refData = new a2dFontFreetypeData( font, size, encoding );
#else
                wxFAIL_MSG( _( "freetype based on wxFont not enabled" ) );
#endif // defined( __USE_WINAPI__ )
                break;
#endif
            default:
                wxFAIL_MSG( _( "wrong font to set a wxFont to" ) );
                break;
        }
        ms_fontCache[fontstr] = *this;
    }
    else
        // re-use that cached font
        Ref( ms_fontCache[fontstr] );
}

a2dFont::a2dFont( double size,
                  wxFontFamily family,
                  wxFontStyle style,
                  wxFontWeight weight,
                  const bool underline, const wxString& faceName,
                  wxFontEncoding encoding )
{
    wxString fontstr = CreateString( a2dFONT_WXDC, wxFont( 1, family, style, weight, underline, faceName, encoding ), size, encoding );
    if ( ms_fontCache.find( fontstr ) == ms_fontCache.end() )
    {
        m_refData = new a2dFontDcData( size, family, style, weight, underline, faceName, encoding );
        ms_fontCache[fontstr] = *this;
    }
    else
        // re-use that cached font
        Ref( ms_fontCache[fontstr] );
}

#if wxART2D_USE_FREETYPE
a2dFont::a2dFont( const wxString& filename, double size, wxFontEncoding encoding, int faceindex )
{
    wxString fontstr = CreateString( filename, size, encoding, wxFONTWEIGHT_NORMAL );
    if ( ms_fontCache.find( fontstr ) == ms_fontCache.end() )
    {
        m_refData = new a2dFontFreetypeData( filename, size, encoding, faceindex );
        ms_fontCache[fontstr] = *this;
    }
    else
        // re-use that cached font
        Ref( ms_fontCache[fontstr] );
}
#endif

a2dFont::a2dFont( const wxString& filename, double size, bool monoSpaced, double weight )
{
    wxString fontstr = CreateString( filename, size, monoSpaced, weight );
    if ( ms_fontCache.find( fontstr ) == ms_fontCache.end() )
    {
        m_refData = new a2dFontStrokeData( filename, size, monoSpaced, weight );
        ms_fontCache[fontstr] = *this;
    }
    else
    {
        a2dFont cached = ms_fontCache[fontstr];
        // re-use that cached font
        Ref( cached );
    }
}

a2dFont::~a2dFont()
{
}

wxString a2dFont::CreateString()
{
    if ( m_refData )
        return ( ( a2dFontRefData* )GetRefData() )->m_fontinfo.CreateString();
    return wxEmptyString;
}

bool a2dFont::Ok() const
{
    if ( m_refData )
        return ( ( a2dFontRefData* )GetRefData() )->Ok();
    return false;
}

const a2dFontInfo& a2dFont::GetFontInfo() const
{
    if ( m_refData )
        return ( ( a2dFontRefData* )GetRefData() )->m_fontinfo;
    static a2dFontInfo dummy = a2dFontInfo();
    return dummy;
}

void a2dFont::SetSize( double size )
{
    AllocExclusive();

    if ( m_refData )
        ( ( a2dFontRefData* )GetRefData() )->SetSize( size );
}

double a2dFont::GetSize() const
{
    if ( m_refData )
        return ( ( a2dFontRefData* )GetRefData() )->GetSize();
    return 0.0;
}

double a2dFont::GetKerning( wxChar c1, wxChar c2 ) const
{
    if ( m_refData )
        return ( ( a2dFontRefData* )GetRefData() )->GetKerning( c1, c2 );
    return 0.0;
}

double a2dFont::GetStrokeWidth() const
{
    switch ( GetType() )
    {
        case a2dFONT_STROKED:
            return ( ( const a2dFontStrokeData* )m_refData )->GetStrokeWidth();
        default:
            return 0.0;
            break;
    }
    return 0.0;
}

a2dStrokeGlyph a2dFont::GetGlyphStroke( wxChar c ) const
{
    switch ( GetType() )
    {
        case a2dFONT_STROKED:
            return ( ( a2dFontStrokeData* )GetRefData() )->GetGlyph( c );
        default:
            wxFAIL_MSG( _( "need stroked font a2dFontStrokeData" ) );
            break;
    }
    return NULL;
}

const a2dGlyph* a2dFont::GetGlyphFreetype( wxChar c ) const
{
    switch ( GetType() )
    {
#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
        case a2dFONT_WXDC:
        {
            a2dFont ft = ( ( a2dFontDcData* )GetRefData() )->GetFreetypeFont();
            return ( ( a2dFontFreetypeData* )ft.GetRefData() )->GetGlyph( c );
            break;
        }
#endif
		case a2dFONT_FREETYPE:
#if wxART2D_USE_FREETYPE
            return ( ( a2dFontFreetypeData* )GetRefData() )->GetGlyph( c );
#endif
	        break;
        default:
            wxFAIL_MSG( _( "need stroked font a2dFontFreetypeData" ) );
            break;
    }
    return NULL;
}

void a2dFont::SetDeviceHeight( double sizeInPixels )
{
    switch ( GetType() )
    {
        case a2dFONT_WXDC:
        {
            ( ( a2dFontDcData* )GetRefData() )->SetDeviceHeight( sizeInPixels );
            break;
        }
        case a2dFONT_FREETYPE:
#if wxART2D_USE_FREETYPE
            ( ( a2dFontFreetypeData* )GetRefData() )->SetDeviceHeight( sizeInPixels );
#endif
            break;
        default:
            break;
    }
}

double a2dFont::GetDeviceHeight()
{
    switch ( GetType() )
    {
#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
        case a2dFONT_WXDC:
        {
            return ( ( a2dFontDcData* )GetRefData() )->GetDeviceHeight();
            break;
        }
#endif
        case a2dFONT_FREETYPE:
#if wxART2D_USE_FREETYPE
            return ( ( a2dFontFreetypeData* )GetRefData() )->GetDeviceHeight();
#endif
        default:
            return 0;
            break;
    }
}

#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
a2dFont a2dFont::GetFreetypeFont()
{
    switch ( GetType() )
    {
        case a2dFONT_WXDC:
            return ( ( a2dFontDcData* )GetRefData() )->GetFreetypeFont();
            break;
        default:
            wxFAIL_MSG( _( "need wxDc font a2dFontDcData" ) );
            break;
    }
    return *a2dNullFONT;
}

void a2dFont::SetFreetypeFont( a2dFont font )
{
    switch ( GetType() )
    {
        case a2dFONT_WXDC:
            ( ( a2dFontDcData* )GetRefData() )->SetFreetypeFont( font );
            break;
        default:
            wxFAIL_MSG( _( "need wxDc font a2dFontDcData" ) );
            break;
    }
}
#endif

wxFont& a2dFont::GetFont( void )
{
    switch ( GetType() )
    {
        case a2dFONT_WXDC:

            return ( ( a2dFontDcData* )GetRefData() )->m_font;
        default:
            wxFAIL_MSG( _( "need wxDc font a2dFontDcData" ) );
            break;
    }
    return wxNullFont;
}

a2dBoundingBox a2dFont::GetTextExtent( const wxString& string,
                                       int alignment, double* w, double* h,
                                       double* descent, double* externalLeading ) const
{
    double gte_x, gte_y, gte_w, gte_h, gte_desc, gte_lead;
    GetTextExtent( string, gte_w, gte_h, gte_desc, gte_lead );

    // Determine Y-Offset
    if ( alignment & wxMINY )
        gte_y = 0;
    else if ( alignment & wxMAXY )
        gte_y = -gte_h;
    else if ( alignment & wxBASELINE )
        gte_y = -gte_desc;
    else if ( alignment & wxBASELINE_CONTRA )
        gte_y = gte_desc - gte_h;
    else
        gte_y = -gte_h / 2.0; //centre

    // Determine X-offset
    gte_x = -gte_w / 2.0; //centre
    if ( alignment & wxMINX )
        gte_x += gte_w / 2.0;
    if ( alignment & wxMAXX )
        gte_x -= gte_w / 2.0;

    // set return variables.
    if ( w ) * w = gte_w;
    if ( h ) * h = gte_h;
    if ( descent ) * descent = gte_desc;
    if ( externalLeading ) * externalLeading = gte_lead;

    return a2dBoundingBox( gte_x, gte_y, gte_x + gte_w, gte_y + gte_h );
}

void a2dFont::GetTextExtent( const wxString& string, double& w,
                             double& h, double& descent, double& externalLeading ) const
{
    return ( ( a2dFontRefData* )GetRefData() )->GetTextExtent( string, w, h, descent, externalLeading );
}

void a2dFont::GetTextExtentWH( const wxString& string, double& w, double& h ) const
{
    double descent, externalLeading;
    return ( ( a2dFontRefData* )GetRefData() )->GetTextExtent( string, w, h, descent, externalLeading );
}

bool a2dFont::GetPartialTextExtents  ( const wxString&   text,  wxArrayInt&   widths  )   const
{
    return true;
}

double a2dFont::GetLineHeight() const
{
    return ( ( a2dFontRefData* )GetRefData() )->GetLineHeight();
}

double a2dFont::GetDescent() const
{
    return ( ( a2dFontRefData* )GetRefData() )->GetDescent();
}

double a2dFont::GetWidth( wxChar c )
{
    return ( ( a2dFontRefData* )GetRefData() )->GetWidth( c );
}

double a2dFont::GetExternalLead() const
{
    return ( ( a2dFontRefData* )GetRefData() )->GetExternalLead();
}

bool a2dFont::GetVpath( wxChar c, a2dVpath& vpath, const a2dAffineMatrix& affine )
{
    return ( ( a2dFontRefData* )GetRefData() )->GetVpath( c, vpath, affine );
}

void a2dFont::GetVpath( const wxString& text, a2dVpath& vpath, double x, double y, int alignment, bool yaxis, const a2dAffineMatrix& affine )
{
    // Get the bounding box, including alignment displacement.
    a2dBoundingBox bbox = GetTextExtent( text, alignment );
    bbox.Translate( x, y );

    // Get unaligned coordinates. By default these functions
    // use the lowerleft corner of the boundingbox, hence GetMinX() and GetMinY().
    x = bbox.GetMinX();
    y = bbox.GetMinY();

    // mirror text, depending on y-axis orientation
    const double h = GetLineHeight();
    a2dAffineMatrix affine2;
    if ( !yaxis )
    {
        affine2.Translate( 0.0, -h );
        affine2.Mirror( true, false );
    }
    // position text
    affine2.Translate( x, y );

    a2dAffineMatrix affine3 = affine;
    affine3 *= affine2;

    wxChar c = 0;
    double w = 0.0;
    const size_t n = text.Length();
    for ( size_t i = 0; i < n; i++ )
    {
        const wxChar oldc = c;
        c = text[i];
        if ( i > 0 )
            affine3 *= a2dAffineMatrix( w + GetKerning( oldc, c ), 0.0 );
        w = GetWidth( c );

        GetVpath( c, vpath, affine3 );
    }
}

void a2dFont::GetInfoList( a2dFontInfoList& list )
{
#if wxART2D_USE_FREETYPE
    a2dFontFreetypeData::GetInfoList( list );
#endif
    a2dFontStrokeData::GetInfoList( list );
    a2dFontDcData::GetInfoList( list );
}

a2dFont a2dFont::CreateFont( const a2dFontInfo& info, bool force )
{
    a2dFontRefData* fontdata = NULL;
    a2dFont font;

    wxString fontstr = info.CreateString();
    if ( ms_fontCache.find( fontstr ) != ms_fontCache.end() )
    {
        // re-use that cached font
        font.Ref( ms_fontCache[fontstr] );
        return font;
    }

    // try to find exact font
#if wxART2D_USE_FREETYPE
    fontdata = ( a2dFontRefData* ) a2dFontFreetypeData::CreateFont( info );
#endif
    if ( !fontdata )
        fontdata = ( a2dFontRefData* ) a2dFontStrokeData::CreateFont( info );
    if ( !fontdata )
        fontdata = ( a2dFontRefData* ) a2dFontDcData::CreateFont( info );

    if ( !fontdata && !force )
    {
        font = a2dFont();
        font.m_refData = fontdata;
        return font;
    }

    // try to find similar font, whatever type
#if wxART2D_USE_FREETYPE
    if ( !fontdata )
        fontdata = ( a2dFontRefData* ) a2dFontFreetypeData::CreateFont( info, force );
#endif
    if ( !fontdata )
        fontdata = ( a2dFontRefData* ) a2dFontStrokeData::CreateFont( info, force );
    if ( !fontdata )
        fontdata = ( a2dFontRefData* ) a2dFontDcData::CreateFont( info, force );

    if ( !fontdata )
    {
        // just use default font
        font = *a2dDEFAULT_CANVASFONT;
        font.SetSize( info.GetSize() );
    }
    else
    {
        font = a2dFont();
        font.m_refData = fontdata;
    }

    ms_fontCache[fontstr] = font;

    return font;
}

a2dFontType a2dFont::GetType( void ) const
{
    if ( !m_refData )
        return a2dFONT_NULL;
    return ( ( a2dFontRefData* ) m_refData )->GetType();
}

wxObjectRefData* a2dFont::CreateRefData() const
{
    switch ( GetType() )
    {
        case a2dFONT_NULL:
            return new a2dFontDcData(); //dummy to be removed soon
        case a2dFONT_WXDC:
            return new a2dFontDcData();
#if wxART2D_USE_FREETYPE
        case a2dFONT_FREETYPE:
            return new a2dFontFreetypeData();
#endif
        case a2dFONT_STROKED:
            return new a2dFontStrokeData();
        default:
            break;
    }
    return NULL;
}

wxObjectRefData* a2dFont::CloneRefData( const wxObjectRefData* data ) const
{
    switch ( ( ( a2dFontRefData* ) data )->GetType() )
    {
        case a2dFONT_NULL:
            return NULL;
        case a2dFONT_WXDC:
            return new a2dFontDcData( *( ( const a2dFontDcData* )data ) );
#if wxART2D_USE_FREETYPE
        case a2dFONT_FREETYPE:
            return new a2dFontFreetypeData( *( ( const a2dFontFreetypeData* )data ) );
#endif
        case a2dFONT_STROKED:
            return new a2dFontStrokeData( *( ( const a2dFontStrokeData* )data ) );
        default:
            break;
    }
    return NULL;
}

void a2dFont::SetLoadFlags( wxInt32 loadMethodMask )
{
#if wxART2D_USE_FREETYPE
    a2dFontFreetypeData::SetLoadFlags( loadMethodMask );
#endif
}


void a2dFont::SetFilename( const wxString& filename )
{
    switch ( ( ( a2dFontRefData* ) m_refData )->GetType() )
    {
#if wxART2D_USE_FREETYPE
        case a2dFONT_FREETYPE:
            ( ( a2dFontFreetypeData* )m_refData )->SetFilename( filename );
            break;
#endif
        case a2dFONT_STROKED:
            ( ( a2dFontStrokeData* )m_refData )->SetFilename( filename );
            break;
        default:
            wxFAIL_MSG( _( "wrong font to set a filename" ) );
            break;
    }
}

wxString a2dFont::GetFilename( bool filenameonly ) const
{
    switch ( ( ( a2dFontRefData* ) m_refData )->GetType() )
    {
#if wxART2D_USE_FREETYPE
        case a2dFONT_FREETYPE:
            return ( ( const a2dFontFreetypeData* )m_refData )->GetFilename( filenameonly );
#endif
        case a2dFONT_STROKED:
            return ( ( const a2dFontStrokeData* )m_refData )->GetFilename( filenameonly );
        default:
            break;
    }
    return wxT( "" );
}

double a2dFont::GetWeight() const
{
    switch ( ( ( a2dFontRefData* ) m_refData )->GetType() )
    {
        case a2dFONT_STROKED:
            return ( ( const a2dFontStrokeData* )m_refData )->GetWeight();
        default:
            break;
    }
    return 0.0;
}

void a2dFont::SetWeight( double weight )
{
    if ( GetWeight() == weight )
        return;

    AllocExclusive();

    switch ( ( ( a2dFontRefData* ) m_refData )->GetType() )
    {
        case a2dFONT_STROKED:
            ( ( a2dFontStrokeData* )m_refData )->SetWeight( weight );
        default:
            break;
    }
}

void a2dFont::SetWeight( const wxString& weight )
{
    AllocExclusive();

    switch ( ( ( a2dFontRefData* ) m_refData )->GetType() )
    {
        case a2dFONT_STROKED:
            ( ( a2dFontStrokeData* )m_refData )->SetWeight( weight );
        default:
            break;
    }
}


A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFill*, a2dBLACK_FILL );
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFill*, a2dWHITE_FILL );
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFill*, a2dTRANSPARENT_FILL );

//! used to remove fill style in objects
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFill*, a2dNullFILL );

A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFill*, a2dINHERIT_FILL );
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFill*, a2dSELECT_FILL );

A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dStroke*,  a2dBLACK_STROKE );
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dStroke*,  a2dWHITE_STROKE );
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dStroke*,  a2dTRANSPARENT_STROKE );

//! used to remove stroke style in objects
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dStroke*,  a2dNullSTROKE );

A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dStroke*,  a2dINHERIT_STROKE );

//! used layer wxLAYER_SELECTED in to show selected object (and nested objects)
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dStroke*,  a2dLAYER_SELECT_STROKE );

//! used to remove font style in objects
A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFont*, a2dNullFONT );

A2DARTBASEDLLEXP a2dDECLARE_GLOBAL_VAR( const a2dFont*, a2dDEFAULT_CANVASFONT );

A2DARTBASEDLLEXP void a2dCanvasInitializeStockObjects ()
{
    a2dBLACK_FILL = new a2dFill( wxColour( 0, 0, 0 ) );

    a2dWHITE_FILL = new a2dFill( wxColour( 255, 255, 255 ) );

    a2dTRANSPARENT_FILL =  new a2dFill( wxColour( 255, 255, 255 ), a2dFILL_TRANSPARENT );

    a2dNullFILL = new a2dFill();

    a2dINHERIT_FILL = new a2dFill( wxColour( 0, 0, 0 ) );

    a2dBLACK_STROKE = new a2dStroke( wxColour( 0, 0, 0 ), 0.0 );

    a2dWHITE_STROKE = new a2dStroke( wxColour( 255, 255, 255 ), 0.0 );

    a2dTRANSPARENT_STROKE = new a2dStroke( wxColour( 255, 255, 255 ), 0.0, a2dSTROKE_TRANSPARENT );

    a2dNullSTROKE = new a2dStroke();

    a2dINHERIT_STROKE = new a2dStroke( wxColour( 0, 0, 0 ), 0 );

    a2dSELECT_FILL =  new a2dFill( wxColour( 0, 0, 0 ), a2dFILL_TRANSPARENT );

    a2dLAYER_SELECT_STROKE = new a2dStroke( wxColour( wxT( "YELLOW" ) ), 2, a2dSTROKE_SOLID );

    a2dNullFONT = new a2dFont();

    a2dDEFAULT_CANVASFONT = new a2dFont( 100.0, wxFONTFAMILY_SWISS );
}

A2DARTBASEDLLEXP void a2dCanvasDeleteStockObjects ()
{
    delete a2dBLACK_FILL;

    delete a2dWHITE_FILL;

    delete a2dTRANSPARENT_FILL;

    delete a2dNullFILL;

    delete a2dINHERIT_FILL;

    delete a2dBLACK_STROKE;

    delete a2dWHITE_STROKE;

    delete a2dTRANSPARENT_STROKE;

    delete a2dNullSTROKE;

    delete a2dINHERIT_STROKE;

    delete a2dSELECT_FILL;

    delete a2dLAYER_SELECT_STROKE;

    delete a2dNullFONT;

    delete a2dDEFAULT_CANVASFONT;

}

//----------------------------------------------------------------------------
// a2dFill stuff
//----------------------------------------------------------------------------

//! convert a style enum to a string
wxString FillStyle2String( a2dFillStyle style )
{
    switch ( style )
    {
        case a2dFILL_SOLID: return wxT( "a2dFILL_SOLID" );
        case a2dFILL_TRANSPARENT: return wxT( "a2dFILL_TRANSPARENT" );
        case a2dFILL_BDIAGONAL_HATCH: return wxT( "a2dFILL_BDIAGONAL_HATCH" );
        case a2dFILL_CROSSDIAG_HATCH: return wxT( "a2dFILL_CROSSDIAG_HATCH" );
        case a2dFILL_FDIAGONAL_HATCH: return wxT( "a2dFILL_FDIAGONAL_HATCH" );
        case a2dFILL_CROSS_HATCH: return wxT( "a2dFILL_CROSS_HATCH" );
        case a2dFILL_HORIZONTAL_HATCH: return wxT( "a2dFILL_HORIZONTAL_HATCH" );
        case a2dFILL_VERTICAL_HATCH: return wxT( "a2dFILL_VERTICAL_HATCH" );
        case a2dFILL_STIPPLE: return wxT( "a2dFILL_STIPPLE" );
        case a2dFILL_STIPPLE_MASK_OPAQUE: return wxT( "a2dFILL_STIPPLE_MASK_OPAQUE" );
        case a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT: return wxT( "a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT" );
        case a2dFILL_GRADIENT_FILL_XY_LINEAR: return wxT( "a2dFILL_GRADIENT_FILL_XY_LINEAR" );
        case a2dFILL_GRADIENT_FILL_XY_RADIAL: return wxT( "a2dFILL_GRADIENT_FILL_XY_RADIAL" );
        case a2dFILL_GRADIENT_FILL_XY_DROP: return wxT( "a2dFILL_GRADIENT_FILL_XY_DROP" );
        case a2dFILL_INHERIT: return wxT( "a2dFILL_INHERIT" );
        case a2dFILL_LAYER: return wxT( "a2dFILL_LAYER" );
        case a2dFILL_NULLFILL: return wxT( "a2dFILL_NULLFILL" );
        case a2dFILL_TWOCOL_BDIAGONAL_HATCH: return wxT( "a2dFILL_TWOCOL_BDIAGONAL_HATCH" );
        case a2dFILL_TWOCOL_CROSSDIAG_HATCH: return wxT( "a2dFILL_TWOCOL_CROSSDIAG_HATCH" );
        case a2dFILL_TWOCOL_FDIAGONAL_HATCH: return wxT( "a2dFILL_TWOCOL_FDIAGONAL_HATCH" );
        case a2dFILL_TWOCOL_CROSS_HATCH: return wxT( "a2dFILL_TWOCOL_CROSS_HATCH" );
        case a2dFILL_TWOCOL_HORIZONTAL_HATCH: return wxT( "a2dFILL_TWOCOL_HORIZONTAL_HATCH" );
        case a2dFILL_TWOCOL_VERTICAL_HATCH: return wxT( "a2dFILL_TWOCOL_VERTICAL_HATCH" );

        default:
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid style" ) );
    }
    return wxT( "a2dFILL_NULLFILL" );
}

//! convert a string to a style enum
a2dFillStyle FillString2Style( const wxString& stylestr )
{
    if ( stylestr == wxT( "a2dFILL_SOLID" ) )
        return a2dFILL_SOLID;
    if ( stylestr == wxT( "a2dFILL_TRANSPARENT" ) )
        return a2dFILL_TRANSPARENT;
    if ( stylestr == wxT( "a2dFILL_BDIAGONAL_HATCH" ) )
        return a2dFILL_BDIAGONAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_CROSSDIAG_HATCH" ) )
        return a2dFILL_CROSSDIAG_HATCH;
    if ( stylestr == wxT( "a2dFILL_FDIAGONAL_HATCH" ) )
        return a2dFILL_FDIAGONAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_CROSS_HATCH" ) )
        return a2dFILL_CROSS_HATCH;
    if ( stylestr == wxT( "a2dFILL_HORIZONTAL_HATCH" ) )
        return a2dFILL_HORIZONTAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_VERTICAL_HATCH" ) )
        return a2dFILL_VERTICAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_STIPPLE" ) )
        return a2dFILL_STIPPLE;
    if ( stylestr == wxT( "a2dFILL_STIPPLE_MASK_OPAQUE" ) )
        return a2dFILL_STIPPLE_MASK_OPAQUE;
    if ( stylestr == wxT( "a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT" ) )
        return a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT;
    if ( stylestr == wxT( "a2dFILL_GRADIENT_FILL_XY_LINEAR" ) )
        return a2dFILL_GRADIENT_FILL_XY_LINEAR;
    if ( stylestr == wxT( "a2dFILL_GRADIENT_FILL_XY_RADIAL" ) )
        return a2dFILL_GRADIENT_FILL_XY_RADIAL;
    if ( stylestr == wxT( "a2dFILL_GRADIENT_FILL_XY_DROP" ) )
        return a2dFILL_GRADIENT_FILL_XY_DROP;
    if ( stylestr == wxT( "a2dFILL_INHERIT" ) )
        return a2dFILL_INHERIT;
    if ( stylestr == wxT( "a2dFILL_LAYER" ) )
        return a2dFILL_LAYER;
    if ( stylestr == wxT( "a2dFILL_NULLFILL" ) )
        return a2dFILL_NULLFILL;

    if ( stylestr == wxT( "a2dFILL_TWOCOL_BDIAGONAL_HATCH" ) )
        return a2dFILL_TWOCOL_BDIAGONAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_TWOCOL_CROSSDIAG_HATCH" ) )
        return a2dFILL_TWOCOL_CROSSDIAG_HATCH;
    if ( stylestr == wxT( "a2dFILL_TWOCOL_FDIAGONAL_HATCH" ) )
        return a2dFILL_TWOCOL_FDIAGONAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_TWOCOL_CROSS_HATCH" ) )
        return a2dFILL_TWOCOL_CROSS_HATCH;
    if ( stylestr == wxT( "a2dFILL_TWOCOL_HORIZONTAL_HATCH" ) )
        return a2dFILL_TWOCOL_HORIZONTAL_HATCH;
    if ( stylestr == wxT( "a2dFILL_TWOCOL_VERTICAL_HATCH" ) )
        return a2dFILL_TWOCOL_VERTICAL_HATCH;

    if ( !stylestr.IsEmpty() ) //old style settings
    {
        long num;
        stylestr.ToLong( &num );
        return ( a2dFillStyle ) num;
    }

    a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid style %s" ), stylestr.c_str() );

    return a2dFILL_NULLFILL;
}


//----------------------------------------------------------------------------
// a2dFillRefData
//----------------------------------------------------------------------------

//! Internal to a2dFill for refcounting in wxObject style
/*!
    \ingroup style
*/
class A2DARTBASEDLLEXP a2dFillRefData : public wxObjectRefData
{
public:

    virtual a2dFillType GetType( void ) const { return a2dFILL_NULL; }

    //!constructor
    a2dFillRefData()
    {
        m_fillOn = true;
    }

    a2dFillRefData( const a2dFillRefData& other )
        : wxObjectRefData()
    {
        m_fillOn = other.m_fillOn;
    }

#if wxART2D_USE_CVGIO
    //! Save settings.
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite ) = 0;
    //! Load settings.
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts ) = 0;
#endif //wxART2D_USE_CVGIO

    //! if true filling is done, else transparent fill
    bool m_fillOn;
};

//----------------------------------------------------------------------------
// a2dOneColourFillData
//----------------------------------------------------------------------------

//! One Colour Fill
/*!
A second colour is not saved, but when asked for it return wxBLACK

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dOneColourFillData : public a2dFillRefData
{
private:

    friend class a2dFill;

public:

    virtual a2dFillType GetType( void ) const { return a2dFILL_ONE_COLOUR; }

    //!constructor
    a2dOneColourFillData( void );

    a2dOneColourFillData( const a2dOneColourFillData& fill );

    //!constructor
    a2dOneColourFillData( const wxColour& col, a2dFillStyle style = a2dFILL_SOLID );

    a2dOneColourFillData( const wxBrush& brush );

    //!destructor
    ~a2dOneColourFillData( void ) {}

    static a2dOneColourFillData* Create( const wxColour& col, a2dFillStyle style = a2dFILL_SOLID );

    void SetColour( const wxColour& col ) { m_colour1 = col; }

    //! return colour
    wxColour GetColour() const { return m_colour1; }

    a2dFillStyle GetStyle( void ) const { return m_style; }

    void SetStyle( a2dFillStyle style ) {  m_style = style; }

protected:

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! style of Fill
    a2dFillStyle m_style;

    wxColour      m_colour1;
};

a2dOneColourFillData* a2dOneColourFillData::Create( const wxColour& col, a2dFillStyle style )
{
    return new a2dOneColourFillData( col, style );
}

a2dOneColourFillData::a2dOneColourFillData( void )
    : a2dFillRefData()
{
    m_style = a2dFILL_SOLID;
    m_colour1 = *wxBLACK;
}

a2dOneColourFillData::a2dOneColourFillData( const wxColour& col, a2dFillStyle style )
    : a2dFillRefData()
{
    m_colour1 = col;
    if ( style == a2dFILL_SOLID ||
            style == a2dFILL_TRANSPARENT ||
            style == a2dFILL_BDIAGONAL_HATCH ||
            style == a2dFILL_CROSSDIAG_HATCH ||
            style == a2dFILL_FDIAGONAL_HATCH ||
            style == a2dFILL_CROSS_HATCH ||
            style == a2dFILL_HORIZONTAL_HATCH ||
            style == a2dFILL_VERTICAL_HATCH ||
            style == a2dFILL_NULLFILL
       )
        m_style = style;
    else
        wxFAIL_MSG( _( "invalid style for a2dOneColourFillData" ) );
}

a2dOneColourFillData::a2dOneColourFillData( const wxBrush& brush )
    : a2dFillRefData()
{
    if ( brush.GetStyle() == wxBRUSHSTYLE_SOLID )
    {
        m_colour1 = wxColour( brush.GetColour() );
        m_style = a2dFILL_SOLID;
    }
    else if ( brush.IsHatch() )
    {
        m_colour1 = wxColour( brush.GetColour() );

        switch( brush.GetStyle() )
        {
            case wxBRUSHSTYLE_BDIAGONAL_HATCH :
                m_style = a2dFILL_BDIAGONAL_HATCH;
                break ;
            case wxBRUSHSTYLE_CROSSDIAG_HATCH :
                m_style = a2dFILL_CROSSDIAG_HATCH;
                break ;
            case wxBRUSHSTYLE_FDIAGONAL_HATCH :
                m_style = a2dFILL_FDIAGONAL_HATCH;
                break ;
            case wxBRUSHSTYLE_CROSS_HATCH :
                m_style = a2dFILL_CROSS_HATCH;
                break ;
            case wxBRUSHSTYLE_HORIZONTAL_HATCH :
                m_style = a2dFILL_HORIZONTAL_HATCH;
                break ;
            case wxBRUSHSTYLE_VERTICAL_HATCH :
                m_style = a2dFILL_VERTICAL_HATCH;
                break ;
        }
    }
    else if ( brush.GetStyle() == wxBRUSHSTYLE_TRANSPARENT )
        m_style = a2dFILL_TRANSPARENT;
}

a2dOneColourFillData::a2dOneColourFillData( const a2dOneColourFillData& other )
    : a2dFillRefData( other )
{
    m_style = other.m_style;
    m_colour1 = other.m_colour1;
}

#if wxART2D_USE_CVGIO
void a2dOneColourFillData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_style = ( a2dFillStyle ) FillString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dFILL_SOLID" ) ) );
        SetStyle( m_style );

        wxString fillColour =  parser.GetAttributeValue( wxT( "colour" ) );
        if ( !fillColour.IsEmpty() )
        {
            if ( fillColour.GetChar( 0 ) == wxT( '#' ) )
                m_colour1 = HexToColour( fillColour.After( wxT( '#' ) ) );
            else
            {
                m_colour1 = wxTheColourDatabase->Find( fillColour );
                if ( !m_colour1.Ok() )
                    m_colour1 =  *wxBLACK;
            }
        }
        else
            m_colour1 = *wxBLACK;
    }
    else
    {}
}

void a2dOneColourFillData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dFILL_SOLID )
            out.WriteAttribute( wxT( "style" ),  FillStyle2String( GetStyle() ) );
        if ( m_style != a2dFILL_TRANSPARENT )
        {
            wxString brushColour = wxTheColourDatabase->FindName( GetColour() );

            if ( brushColour.IsEmpty() )
                out.WriteAttribute( wxT( "colour" ), wxT( "#" ) + ColourToHex( GetColour() ) );
            else
                out.WriteAttribute( wxT( "colour" ), brushColour );
        }
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dTwoColourFillData
//----------------------------------------------------------------------------

//!To set a two colour fill.
/*!
It defines a second colour for a fill used for the background of
text.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dTwoColourFillData : public a2dFillRefData
{

private:

    friend class a2dFill;

protected:

    //!constructor
    a2dTwoColourFillData( void );

    a2dTwoColourFillData( const a2dTwoColourFillData& fill );

    //!constructor
    a2dTwoColourFillData( const wxColour& col, const wxColour& col2, a2dFillStyle style );

    //!destructor
    ~a2dTwoColourFillData( void ) {}

    a2dFillStyle GetStyle( void ) const { return m_style; }

    void SetStyle( a2dFillStyle style ) {  m_style = style; }

    //! set colour 2 used for gradient and wxSTIPPLE_MASK_OPAQUE filling.
    void SetColour2( const wxColour& col ) { m_colour2 = col; }

    //! return colour 2
    wxColour GetColour2() const { return m_colour2; }

    //! set colour  used for gradient and wxSTIPPLE_MASK_OPAQUE filling.
    void SetColour( const wxColour& col ) { m_colour1 = col; }

    //! return colour
    wxColour GetColour() const { return m_colour1; }

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! used for gradient fill or text background or opaque filling
    wxColour      m_colour1;

    //! used for gradient fill or text background or opaque filling
    wxColour      m_colour2;

    //! style of Fill
    a2dFillStyle m_style;
};

a2dTwoColourFillData::a2dTwoColourFillData( void )
    : a2dFillRefData()
{
    m_colour1 = *wxBLACK;
    m_colour2 = *wxBLACK;
}

a2dTwoColourFillData::a2dTwoColourFillData( const wxColour& col, const wxColour& col2, a2dFillStyle style )
    : a2dFillRefData( )
{
    m_colour1 = col;
    m_colour2 = col2;
    m_style = style;
}

a2dTwoColourFillData::a2dTwoColourFillData( const a2dTwoColourFillData& other )
    : a2dFillRefData( other )
{
    m_style = other.m_style;
    m_colour1 = other.m_colour1;
    m_colour2 = other.m_colour2;
}

#if wxART2D_USE_CVGIO
void a2dTwoColourFillData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString fillColour =  parser.GetAttributeValue( wxT( "colour" ) );
        if ( fillColour.GetChar( 0 ) == wxT( '#' ) )
            m_colour1 = HexToColour( fillColour.After( wxT( '#' ) ) );
        else
        {
            m_colour1 = wxTheColourDatabase->Find( fillColour );
            if ( !m_colour1.Ok() )
                m_colour1 =  *wxBLACK;
        }
        fillColour =  parser.GetAttributeValue( wxT( "colour2" ) );
        if ( !fillColour.IsEmpty() )
        {
            if ( fillColour.GetChar( 0 ) == wxT( '#' ) )
                m_colour2 = HexToColour( fillColour.After( wxT( '#' ) ) );
            else
            {
                m_colour2 = wxTheColourDatabase->Find( fillColour );
                if ( !m_colour2.Ok() )
                    m_colour2 =  *wxBLACK;
            }
        }
    }
    else
    {}
}

void a2dTwoColourFillData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dFILL_TRANSPARENT )
        {
            wxString brushColour = wxTheColourDatabase->FindName( GetColour() );

            if ( brushColour.IsEmpty() )
                out.WriteAttribute( wxT( "colour" ), wxT( "#" ) + ColourToHex( GetColour() ) );
            else
                out.WriteAttribute( wxT( "colour" ), brushColour );

            brushColour = wxTheColourDatabase->FindName( GetColour2() );
            if ( brushColour.IsEmpty() )
                out.WriteAttribute( wxT( "colour2" ), wxT( "#" ) + ColourToHex( GetColour2() ) );
            else
                out.WriteAttribute( wxT( "colour2" ), brushColour );
        }
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dHatchFillData
//----------------------------------------------------------------------------

//! Two colour Hatch Fill
/*!
    The hatch if drawn in the foreground colour and, colour2 is used for the background.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dHatchFillData: public a2dTwoColourFillData
{

private:

    friend class a2dFill;

public:

    virtual a2dFillType GetType( void ) const { return a2dFILL_HATCH_TWO_COLOUR; }

protected:

    //!constructor
    a2dHatchFillData( void );

    //!constructor
    a2dHatchFillData( const a2dHatchFillData& other );

    //!constructor
    /*!
        \param col foreground color
        \param col2 background color
    */
    a2dHatchFillData( const wxColour& col, const wxColour& col2, a2dFillStyle style );

    //!destructor
    ~a2dHatchFillData( void ) {}

    void SetStyle( a2dFillStyle style );

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

};

a2dHatchFillData::a2dHatchFillData( void ): a2dTwoColourFillData()
{
    m_style = a2dFILL_TWOCOL_BDIAGONAL_HATCH;
}

a2dHatchFillData::a2dHatchFillData( const wxColour& col, const wxColour& col2, a2dFillStyle style )
    : a2dTwoColourFillData( col, col2, style )
{
    SetStyle( style );
}

a2dHatchFillData::a2dHatchFillData( const a2dHatchFillData& other )
    : a2dTwoColourFillData( other )
{
}

void a2dHatchFillData::SetStyle( a2dFillStyle style )
{
    if (
        style == a2dFILL_TRANSPARENT ||
        style == a2dFILL_TWOCOL_BDIAGONAL_HATCH ||
        style == a2dFILL_TWOCOL_CROSSDIAG_HATCH ||
        style == a2dFILL_TWOCOL_FDIAGONAL_HATCH ||
        style == a2dFILL_TWOCOL_CROSS_HATCH ||
        style == a2dFILL_TWOCOL_HORIZONTAL_HATCH ||
        style == a2dFILL_TWOCOL_VERTICAL_HATCH
    )
        m_style = style;
    else
        wxFAIL_MSG( _( "invalid style for a2dHatchFillData" ) );
}

#if wxART2D_USE_CVGIO
void a2dHatchFillData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dTwoColourFillData::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dFILL_TWOCOL_BDIAGONAL_HATCH )
            out.WriteAttribute( wxT( "style" ),  FillStyle2String( m_style ) );
    }
    else
    {}
}

void a2dHatchFillData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dTwoColourFillData::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_style = ( a2dFillStyle ) FillString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dFILL_TWOCOL_BDIAGONAL_HATCH" ) ) );
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dLinearGradientFillData
//----------------------------------------------------------------------------

//!Linear gradient Fill
/*!
gradient filling using lines changing in colour from brush colour to colour2
There is a start and end point for the gradient fill, those points are relative to the parent
of the object containing the fill, and defined in world coordinates.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dLinearGradientFillData: public a2dTwoColourFillData
{

private:

    friend class a2dFill;

public:

    virtual a2dFillType GetType( void ) const { return a2dFILL_GRADIENT_FILL_LINEAR; }

protected:

    //!constructor
    a2dLinearGradientFillData( void );

    //!constructor
    a2dLinearGradientFillData( const a2dLinearGradientFillData& other );

    //!constructor
    /*!
        \param col first color
        \param col2 second color
    */
    a2dLinearGradientFillData(
        const wxColour& col, const wxColour& col2,
        double x1, double y1, double x2, double y2 );

    //!destructor
    ~a2dLinearGradientFillData( void ) {}

    //!start for gradient fill colour change
    void SetStart( double x, double y );

    //!stop for gradient fill colour change
    void SetStop( double x, double y );

    //!Get start for gradient fill colour change
    a2dPoint2D GetStart() { return a2dPoint2D( m_x1, m_y1 ); }

    //!Get stop for gradient fill colour change
    a2dPoint2D GetStop() { return a2dPoint2D( m_x2, m_y2 ); }

    void Init( a2dAffineMatrix* cworld, double minx, double miny, double maxx, double maxy, double centroidx, double centroidy  );

    void ColourXY( a2dDrawer2D* drawer, int x1, int x2, int y );

    wxUint32 GetStamp() { return m_gradientstamp; }

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    double m_x1, m_y1, m_x2, m_y2;

    wxUint32 m_gradientstamp;
};

a2dLinearGradientFillData::a2dLinearGradientFillData( void ): a2dTwoColourFillData()
{
    m_style = a2dFILL_GRADIENT_FILL_XY_LINEAR;
    m_x1 = 0;
    m_y1 = 0;
    m_x2 = 0;
    m_y2 = 0;
}

a2dLinearGradientFillData::a2dLinearGradientFillData(
    const wxColour& col, const wxColour& col2,
    double x1, double y1, double x2, double y2 )
    : a2dTwoColourFillData( col, col2, a2dFILL_GRADIENT_FILL_XY_LINEAR )
{
    m_x1 = x1;
    m_y1 = y1;
    m_x2 = x2;
    m_y2 = y2;
}

a2dLinearGradientFillData::a2dLinearGradientFillData( const a2dLinearGradientFillData& other )
    : a2dTwoColourFillData( other )
{
    m_x1 = other.m_x1;
    m_y1 = other.m_y1;
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;
}

void a2dLinearGradientFillData::SetStart( double x, double y )
{
    m_x1 = x;
    m_y1 = y;
}

void a2dLinearGradientFillData::SetStop( double x, double y )
{
    m_x2 = x;
    m_y2 = y;
}

#if wxART2D_USE_CVGIO
void a2dLinearGradientFillData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dTwoColourFillData::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dFILL_GRADIENT_FILL_XY_LINEAR )
            out.WriteAttribute( wxT( "style" ),  FillStyle2String( m_style ) );
        out.WriteAttribute( wxT( "x1" ), m_x1 );
        out.WriteAttribute( wxT( "y1" ), m_y1 );
        out.WriteAttribute( wxT( "x2" ), m_x2 );
        out.WriteAttribute( wxT( "y2" ), m_y2 );
    }
    else
    {}
}

void a2dLinearGradientFillData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dTwoColourFillData::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_style = ( a2dFillStyle ) FillString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dFILL_GRADIENT_FILL_XY_LINEAR" ) ) );

        m_x1 = parser.GetAttributeValueDouble( wxT( "x1" ) ) ;
        m_y1 = parser.GetAttributeValueDouble( wxT( "y1" ) ) ;
        m_x2 = parser.GetAttributeValueDouble( wxT( "x2" ) ) ;
        m_y2 = parser.GetAttributeValueDouble( wxT( "y2" ) ) ;
    }
    else
    {}
}

#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dRadialGradientFillData
//----------------------------------------------------------------------------
//!radial gradient Fill
/*!
gradient filling using circles changing in colour from colour1 to colour2
There is a start and end point for the gradient fill, those points are relative to the parent
of the object containing the fill, and defined in world coordinates.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dRadialGradientFillData: public a2dTwoColourFillData
{

private:

    friend class a2dFill;

public:

    virtual a2dFillType GetType( void ) const { return a2dFILL_GRADIENT_FILL_RADIAL; }

protected:

    //!constructor
    a2dRadialGradientFillData( void );

    //!constructor
    a2dRadialGradientFillData( const a2dRadialGradientFillData& other );

    //!constructor
    /*!
        \param col first color
        \param col2 second color
    */
    a2dRadialGradientFillData(
        const wxColour& col, const wxColour& col2,
        double xfc, double yfc, double xe, double ye, double radius );

    //!destructor
    ~a2dRadialGradientFillData( void ) {}

    //!center end for gradient fill colour change
    void SetCenter( double x, double y );

    //!radius for gradient fill colour change
    void SetRadius( double radius );

    //!focal point, startpoint for gradient fill
    void SetFocal( double x, double y );

    //!Get start for gradient fill colour change
    a2dPoint2D GetCenter() { return a2dPoint2D( m_xe, m_ye ); }

    //!Get start for gradient fill colour change
    a2dPoint2D GetFocal() { return a2dPoint2D( m_xe, m_ye ); }

    //!Get stop for gradient fill colour change
    double GetRadius() { return m_radius; }

    void Init( a2dAffineMatrix* cworld, double minx, double miny, double maxx, double maxy, double centroidx, double centroidy  );

    void ColourXY( a2dDrawer2D* drawer, int x1, int x2, int y );

    wxUint32 GetStamp() { return m_gradientstamp; }

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //a2dAffineMatrix* m_usertodevice;

    double m_radius;

    //!relative x as end for gradient fill
    double m_xe;

    //!relative y as end for gradient fill
    double m_ye;

    //!relative start and focal point x
    double m_xfc;

    //!relative start and focal point y
    double m_yfc;

    wxUint32 m_gradientstamp;
};

a2dRadialGradientFillData::a2dRadialGradientFillData( void ): a2dTwoColourFillData()
{
    m_style = a2dFILL_GRADIENT_FILL_XY_RADIAL;
    m_xe = 0;
    m_ye = 0;
    m_xfc = 0;
    m_yfc = 0;
    m_radius = 0;
}

a2dRadialGradientFillData::a2dRadialGradientFillData(
    const wxColour& col, const wxColour& col2,
    double xfc, double yfc, double xe, double ye, double radius )
    : a2dTwoColourFillData( col, col2, a2dFILL_GRADIENT_FILL_XY_RADIAL )
{
    m_xe = xe;
    m_ye = ye;
    m_xfc = xfc;
    m_yfc = yfc;
    m_radius = radius;
}

a2dRadialGradientFillData::a2dRadialGradientFillData( const a2dRadialGradientFillData& other )
    : a2dTwoColourFillData( other )
{
    m_xe = other.m_xe;
    m_ye = other.m_ye;
    m_xfc = other.m_xfc;
    m_yfc = other.m_yfc;
    m_radius = other.m_radius;
}

void a2dRadialGradientFillData::SetCenter( double x, double y )
{
    m_xe = x;
    m_ye = y;
}

void a2dRadialGradientFillData::SetRadius( double radius )
{
    m_radius = radius;
}

//!focal point, startpoint for gradient fill
void a2dRadialGradientFillData::SetFocal( double x, double y )
{
    m_xfc = x;
    m_yfc = y;
}

#if wxART2D_USE_CVGIO
void a2dRadialGradientFillData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dTwoColourFillData::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dFILL_GRADIENT_FILL_XY_RADIAL )
            out.WriteAttribute( wxT( "style" ),  FillStyle2String( m_style ) );
        out.WriteAttribute( wxT( "cx" ), m_xe );
        out.WriteAttribute( wxT( "cy" ), m_ye );
        out.WriteAttribute( wxT( "r" ), m_radius );
        out.WriteAttribute( wxT( "fx" ), m_xfc );
        out.WriteAttribute( wxT( "fy" ), m_yfc );
    }
    else
    {}
}

void a2dRadialGradientFillData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dTwoColourFillData::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_style = ( a2dFillStyle ) FillString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dFILL_GRADIENT_FILL_XY_RADIAL" ) ) );

        m_xe = parser.GetAttributeValueDouble( wxT( "cx" ) ) ;
        m_ye = parser.GetAttributeValueDouble( wxT( "cy" ) ) ;
        m_radius = parser.GetAttributeValueDouble( wxT( "r" ) ) ;
        m_xfc = parser.GetAttributeValueDouble( wxT( "fx" ) ) ;
        m_yfc = parser.GetAttributeValueDouble( wxT( "fy" ) ) ;
    }
    else
    {}

}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dPatternFillData
//----------------------------------------------------------------------------

//!Filling using a wxbitmap
/*!
To Fill shapes with a bitmap. a2dFILL_STIPPLE
In case a wxMask is set, to do transparent stipple drawing use style a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT.
The fill colour1 will be used for non Transparent parts in the brush bitmap,
while other parts will be transparent.
In case a wxMask is set, use style wxSTIPPLE_MASK_OPAQUE to use colour1 and colour2 to fill with
the wxMask in those colours.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dPatternFillData: public a2dTwoColourFillData
{
private:

    friend class a2dFill;

public:

    virtual a2dFillType GetType( void ) const { return a2dFILL_BITMAP; }

protected:

    //!constructor
    a2dPatternFillData();

    //!constructor
    a2dPatternFillData( const a2dPatternFillData& other );

    //!constructor
    /*!
     \param stipple bitmap for stippling the primitive drawn.
     \param style can be a2dFILL_STIPPLE a2dFILL_STIPPLE_MASK_OPAQUE, a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT

    */
    a2dPatternFillData( const wxBitmap& stipple, a2dFillStyle style = a2dFILL_STIPPLE );

    a2dPatternFillData( const wxFileName& filename, a2dFillStyle style = a2dFILL_STIPPLE );

    //!destructor
    ~a2dPatternFillData( void ) {}

    //! set stipple style
    void SetStipple( const wxBitmap& stipple );

    const wxBitmap& GetStipple() const { return m_stipple; }

    //! can contain the filename if pattern was read froma file
    /*!
        Certain input formats ( Like CVG ) can hold the path to the pattern.
    */
    wxString GetFileName() { return m_filename; }

    a2dFillStyle GetStyle() const;

    void SetStyle( a2dFillStyle style );

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    wxBitmap m_stipple;

    //! if the pattern was loaded from a file, this contains the path to this file.
    //! a2dGlobals->GetImagePathList() contains the search path for the file.
    wxString m_filename;

};

a2dPatternFillData::a2dPatternFillData()
{
    m_stipple = wxNullBitmap;
    m_style = a2dFILL_STIPPLE;
    SetStyle( m_style );
}

a2dPatternFillData::a2dPatternFillData( const wxBitmap& stipple, a2dFillStyle style )
{
    m_stipple = stipple;

    if ( style == a2dFILL_STIPPLE_MASK_OPAQUE || style == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        wxASSERT_MSG( m_stipple.GetMask(), wxT( "require a mask for a2dFILL_STIPPLE_MASK_OPAQUE or a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT" ) );
    }
    SetStyle( style );
}

a2dPatternFillData::a2dPatternFillData( const wxFileName& filename, a2dFillStyle style )
{
    m_filename = filename.GetFullPath();

    wxImage image;
    if ( m_filename.IsEmpty() )
    {
        wxLogError( _( "CVG : bitmap file for a2dPatternFill not specified or empty" ) );
        image = wxImage( 32, 32 );
    }
    else
    {
        wxString filename = a2dGlobals->GetImagePathList().FindValidPath( m_filename );
        if ( filename.IsEmpty() )
        {
            wxLogError( _( "CVG : Could not load bitmap \"%s\" for a2dPatternFill from path \"%s\" " ), m_filename.c_str(), a2dGlobals->GetImagePathList().GetAsString().c_str() );
            image = wxImage( 32, 32 );
        }
        else if ( ! image.LoadFile( filename, wxBITMAP_TYPE_BMP ) )
        {
            wxLogError( _( "CVG : Could not load bitmap %s for a2dPatternFill" ), filename.c_str() );
            image = wxImage( 32, 32 );
        }
    }
    m_stipple = wxBitmap( image );

    if ( style == a2dFILL_STIPPLE_MASK_OPAQUE || style == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        wxColour white( wxT( "WHITE" ) );
        wxColour black( wxT( "BLACK" ) );

        wxMask* monochrome_mask = new wxMask( m_stipple, black );
        m_stipple.SetMask( monochrome_mask );
    }
    SetStyle( style );
}

void a2dPatternFillData::SetStipple( const wxBitmap& stipple )
{
    m_stipple = stipple;
}

a2dPatternFillData::a2dPatternFillData( const a2dPatternFillData& other ): a2dTwoColourFillData( other )
{
    m_stipple = other.m_stipple;
}

a2dFillStyle a2dPatternFillData::GetStyle() const
{
    return m_style;
}

void a2dPatternFillData::SetStyle( a2dFillStyle style )
{
    if ( style == a2dFILL_TRANSPARENT )
        m_style = a2dFILL_TRANSPARENT;
    else if ( style == a2dFILL_STIPPLE_MASK_OPAQUE ||
              style == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT
            )
    {
        if ( !m_stipple.GetMask() )
        {
            wxMask* mask = new wxMask( m_stipple, GetColour() );
            m_stipple.SetMask( mask );
        }
        m_style = style;
    }
    else if ( style == a2dFILL_STIPPLE )
        m_style = style;
    else
        wxFAIL_MSG( _( "invalid style for a2dPatternFillData" ) );
}

#if wxART2D_USE_CVGIO
void a2dPatternFillData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dTwoColourFillData::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString filename = parser.GetAttributeValue( wxT( "filename" ), wxT( "" ) );
        wxImage image;
        if ( filename.IsEmpty() )
        {
            wxLogError( _( "CVG : bitmap file for a2dPatternFillData not specified or empty" ) );
            image = wxImage( 32, 32 );
        }
        else
        {
            m_filename = a2dGlobals->GetImagePathList().FindValidPath( filename );
            if ( m_filename.IsEmpty() )
            {
                wxLogError( _( "CVG : Could not load bitmap \"%s\" for a2dPatternFillData from path \"%s\" " ), filename.c_str(), a2dGlobals->GetImagePathList().GetAsString().c_str() );
                image = wxImage( 32, 32 );
            }
            else if ( ! image.LoadFile( m_filename, wxBITMAP_TYPE_BMP ) )
            {
                wxLogError( _( "CVG : Could not load bitmap %s for a2dPatternFillData" ), m_filename.c_str() );
                image = wxImage( 32, 32 );
            }
        }
        m_stipple = wxBitmap( image );

        m_style = ( a2dFillStyle ) FillString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dFILL_STIPPLE" ) ) );
        SetStyle( m_style );

        if ( m_style == a2dFILL_STIPPLE_MASK_OPAQUE || m_style == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
        {
            wxColour white( wxT( "WHITE" ) );
            wxColour black( wxT( "BLACK" ) );

            wxMask* monochrome_mask = new wxMask( m_stipple, black );
            m_stipple.SetMask( monochrome_mask );
        }
    }
    else
    {}
}

void a2dPatternFillData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dTwoColourFillData::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dFILL_STIPPLE )
            out.WriteAttribute( wxT( "style" ),  FillStyle2String( GetStyle() ) );

        wxString filename = a2dGlobals->GetImagePathList().FindValidPath( m_filename );
        if ( filename.IsEmpty() )
        {
            // We can/may not save/use the original files, because the pattern may have changed inside the appl.
            // If available use the name of the orginal file.
            wxString fileTosave;

            wxString* layerfile = a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "wxart2dlayers" ) );
            wxString* layerfiledir = a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "layerFileSavePath" ) );

            if ( layerfile && !layerfile->IsEmpty() )
            {
                if ( !wxDirExists( *layerfiledir ) )
                    wxMkdir( *layerfiledir );
                wxString mess = *layerfiledir + *layerfile + wxT( " directory does not exist, and could not be created" );
                wxASSERT_MSG( wxDirExists( *layerfiledir ) , mess );

                fileTosave = *layerfiledir + wxFILE_SEP_PATH;
            }

            if ( m_filename.IsEmpty() )
            {
                m_filename.Printf( _T( "canpat_%ld.bmp" ), wxGenNewId() );
                fileTosave += m_filename;
            }
            else
                fileTosave += wxFileNameFromPath( m_filename );
            bool isOke = m_stipple.SaveFile( fileTosave, wxBITMAP_TYPE_BMP );
            if ( !isOke )
                wxLogError( _( "Could not save bitmap %s for a2dPatternFillData" ), fileTosave.c_str() );
            out.WriteAttribute( wxT( "filename" ), wxFileNameFromPath( m_filename ) );
        }
        else
            out.WriteAttribute( wxT( "filename" ), wxFileNameFromPath( m_filename ) );
    }
    else
    {}

}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dFill
//----------------------------------------------------------------------------

a2dFill::a2dFill( void )
{
}

a2dFill::a2dFill( const wxColour& col, a2dFillStyle style )
{
    m_refData = new a2dOneColourFillData( col, style );
}

a2dFill::a2dFill( const wxColour& col, const wxColour& col2, a2dFillStyle style )
{
    if ( style == a2dFILL_GRADIENT_FILL_XY_LINEAR )
    {
        m_refData = new a2dLinearGradientFillData( col, col2, 0, 0, 0, 0 );
    }
    else if ( style == a2dFILL_GRADIENT_FILL_XY_RADIAL )
    {
        m_refData = new a2dRadialGradientFillData( col, col2, 0, 0, 0, 0, 0 );
    }
    else if ( style >= a2dFIRST_TWOCOL_HATCH && style <= a2dLAST_TWOCOL_HATCH )
    {
        m_refData = new a2dHatchFillData( col, col2, style );
    }
    else if ( style == a2dFILL_STIPPLE ||
              style == a2dFILL_STIPPLE_MASK_OPAQUE ||
              style == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT
            )
    {
        m_refData = new a2dPatternFillData( wxNullBitmap, style );
    }
    else
    {
        wxFAIL_MSG( _( "invalid style for in a2dFill for two colours" ) );
    }
}

a2dFill::a2dFill( const wxColour& col, const wxColour& col2,
                  double x1, double y1, double x2, double y2 )
{
    m_refData = new a2dLinearGradientFillData( col, col2, x1, y1, x2, y2 );
}

a2dFill::a2dFill( const wxColour& col, const wxColour& col2,
                  double xfc, double yfc, double xe, double ye, double radius )
{
    m_refData = new a2dRadialGradientFillData( col, col2, xfc, yfc, xe, ye, radius );
}

a2dFill::a2dFill( const a2dFill& fill )
    : wxObject( fill )
{
}

a2dFill::a2dFill( const wxBitmap& stipple, a2dFillStyle style )
{
    m_refData = new a2dPatternFillData( stipple, style );
}

a2dFill::a2dFill( const wxBitmap& stipple, const wxColour& col, const wxColour& col2, a2dFillStyle style )
{
    m_refData = new a2dPatternFillData( stipple, style );
    SetColour( col );
    SetColour2( col2 );
}

a2dFill::a2dFill( const wxFileName& filename, a2dFillStyle style )
{
    m_refData = new a2dPatternFillData( filename, style );
}

a2dFill::a2dFill( const wxFileName& filename, const wxColour& col, const wxColour& col2, a2dFillStyle style )
{
    m_refData = new a2dPatternFillData( filename, style );
    SetColour( col );
    SetColour2( col2 );
}

a2dFill::a2dFill( const wxBrush& brush )
{
    if ( brush == wxNullBrush )
    {
    }
    else if (  brush.GetStyle() == wxBRUSHSTYLE_TRANSPARENT  || brush.GetStyle() == wxBRUSHSTYLE_SOLID || brush.IsHatch() )
        m_refData = new a2dOneColourFillData( brush );
    else if (  brush.GetStyle() == wxBRUSHSTYLE_STIPPLE )
    {
        m_refData = new a2dPatternFillData( *brush.GetStipple(), a2dFILL_STIPPLE );
        ( ( a2dPatternFillData* )m_refData )->SetColour( brush.GetColour() );
    }
    else if (  brush.GetStyle() == wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE  )
    {
        m_refData = new a2dPatternFillData( *brush.GetStipple(), a2dFILL_STIPPLE_MASK_OPAQUE );
        ( ( a2dPatternFillData* )m_refData )->SetColour( brush.GetColour() );
    }
}

a2dFill::~a2dFill( void )
{
}

a2dFillType a2dFill::GetType( void ) const
{
    if ( !m_refData )
        return a2dFILL_NULL;
    return ( ( a2dFillRefData* ) m_refData )->GetType();
}


a2dFill& a2dFill::UnShare()
{
    wxObject::UnShare();
    return *this;
}

wxObjectRefData* a2dFill::CreateRefData() const
{
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            return new a2dOneColourFillData(); //dummy to be removed soon
        case a2dFILL_ONE_COLOUR:
            return new a2dOneColourFillData();
        case a2dFILL_HATCH_TWO_COLOUR:
            return new a2dHatchFillData();
        case a2dFILL_GRADIENT_FILL_LINEAR:
            return new a2dLinearGradientFillData();
        case a2dFILL_GRADIENT_FILL_RADIAL:
            return new a2dRadialGradientFillData();
        case a2dFILL_BITMAP:
            return new a2dPatternFillData();
        default:
            break;
    }
    return NULL;
}

wxObjectRefData* a2dFill::CloneRefData( const wxObjectRefData* data ) const
{
    switch ( ( ( a2dFillRefData* ) data )->GetType() )
    {
        case a2dFILL_NULL:
            return NULL;
        case a2dFILL_ONE_COLOUR:
            return new a2dOneColourFillData( *( ( const a2dOneColourFillData* )data ) );
        case a2dFILL_HATCH_TWO_COLOUR:
            return new a2dHatchFillData( *( ( const a2dHatchFillData* )data ) );
        case a2dFILL_GRADIENT_FILL_LINEAR:
            return new a2dLinearGradientFillData( *( ( const a2dLinearGradientFillData* )data ) );
        case a2dFILL_GRADIENT_FILL_RADIAL:
            return new a2dRadialGradientFillData( *( ( const a2dRadialGradientFillData* )data ) );
        case a2dFILL_BITMAP:
            return new a2dPatternFillData( *( ( const a2dPatternFillData* )data ) );
        default:
            break;
    }
    return NULL;
}

a2dPoint2D a2dFill::GetStart() const
{
    wxASSERT_MSG( GetType() == a2dFILL_GRADIENT_FILL_LINEAR, _T( "No ref data" ) );
    return ( ( a2dLinearGradientFillData* )GetRefData() )->GetStart();
}

a2dPoint2D a2dFill::GetStop() const
{
    wxASSERT_MSG( GetType() == a2dFILL_GRADIENT_FILL_LINEAR, _T( "No ref data" ) );
    return ( ( a2dLinearGradientFillData* )GetRefData() )->GetStop();
}

wxUint32 a2dFill::GetStamp() const
{
    switch ( GetType() )
    {
        case a2dFILL_GRADIENT_FILL_LINEAR:
            return ( ( a2dLinearGradientFillData* )GetRefData() )->GetStamp();
        case a2dFILL_GRADIENT_FILL_RADIAL:
            return ( ( a2dRadialGradientFillData* )GetRefData() )->GetStamp();
        default:
            wxFAIL_MSG( _T( "Not a gradient fill" ) );
    }
    return 0;
}

a2dPoint2D a2dFill::GetCenter() const
{
    wxASSERT_MSG( GetType() == a2dFILL_GRADIENT_FILL_RADIAL, _T( "No ref data" ) );
    return ( ( a2dRadialGradientFillData* )GetRefData() )->GetCenter();
}

a2dPoint2D a2dFill::GetFocal() const
{
    wxASSERT_MSG( GetType() == a2dFILL_GRADIENT_FILL_RADIAL, _T( "No ref data" ) );
    return ( ( a2dRadialGradientFillData* )GetRefData() )->GetFocal();
}

double a2dFill::GetRadius() const
{
    wxASSERT_MSG( GetType() == a2dFILL_GRADIENT_FILL_RADIAL, _T( "No ref data" ) );
    return ( ( a2dRadialGradientFillData* )GetRefData() )->GetRadius();
}

void a2dFill::SetAlpha( wxUint8 val )
{
    AllocExclusive();

    switch ( GetType() )
    {
        case a2dFILL_NULL:
            UnRef();
            break;
        case a2dFILL_ONE_COLOUR:
        {
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            wxColour newcol = ( ( a2dOneColourFillData* )GetRefData() )->GetColour();
            newcol.Set( newcol.Red(), newcol.Green(), newcol.Blue(), val );
            ( ( a2dOneColourFillData* )GetRefData() )->SetColour( newcol );
            break;
        }
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
        {
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            wxColour newcol = ( ( a2dTwoColourFillData* )GetRefData() )->GetColour();
            newcol.Set( newcol.Red(), newcol.Green(), newcol.Blue(), val );
            ( ( a2dTwoColourFillData* )GetRefData() )->SetColour( newcol );
            break;
        }
        default:
            break;
    }
}

wxUint8 a2dFill::GetAlpha() const
{
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            return a2dFILL_NULLFILL;
        case a2dFILL_ONE_COLOUR:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dOneColourFillData* )GetRefData() )->GetColour().Alpha();
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dTwoColourFillData* )GetRefData() )->GetColour().Alpha();
            break;
        default:
            break;
    }
    return 255;
}


a2dFillStyle a2dFill::GetStyle() const
{
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            return a2dFILL_NULLFILL;
        case a2dFILL_ONE_COLOUR:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dOneColourFillData* )GetRefData() )->GetStyle();
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dTwoColourFillData* )GetRefData() )->GetStyle();
            break;
        default:
            break;
    }
    return a2dFILL_NULLFILL;
}

void a2dFill::SetStyle( a2dFillStyle style )
{
    a2dFillStyle cur_style = GetStyle();
    if ( cur_style == style )
        return;

    AllocExclusive();

    a2dFillType type = GetTypeForStyle( style );

    if ( type == GetType() )
    {
        switch ( GetType() )
        {
            case a2dFILL_NULL:
                UnRef();
                break;
            case a2dFILL_ONE_COLOUR:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dOneColourFillData* )GetRefData() )->SetStyle( style );
                break;
            case a2dFILL_HATCH_TWO_COLOUR:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dHatchFillData* )GetRefData() )->SetStyle( style );
                break;
            case a2dFILL_GRADIENT_FILL_LINEAR:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dLinearGradientFillData* )GetRefData() )->SetStyle( style );
                break;
            case a2dFILL_GRADIENT_FILL_RADIAL:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dRadialGradientFillData* )GetRefData() )->SetStyle( style );
                break;
            case a2dFILL_BITMAP:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dPatternFillData* )GetRefData() )->SetStyle( style );
                break;
            default:
                break;
        }
    }
    else
    {
        a2dFillRefData* newrefdata = NULL;
        switch ( type )
        {
            case a2dFILL_NULL:
                break;
            case a2dFILL_ONE_COLOUR:
                newrefdata = new a2dOneColourFillData( GetColour(), style );
                break;
            case a2dFILL_HATCH_TWO_COLOUR:
                newrefdata = new a2dHatchFillData( GetColour(), GetColour2(), style );
                break;
            case a2dFILL_GRADIENT_FILL_LINEAR:
                newrefdata = new a2dLinearGradientFillData( GetColour(), GetColour2(), 0, 0, 0, 0 );
                break;
            case a2dFILL_GRADIENT_FILL_RADIAL:
                newrefdata = new a2dRadialGradientFillData( GetColour(), GetColour2(), 0, 0, 0, 0, 0 );
                break;
            case a2dFILL_BITMAP:
                newrefdata = new a2dPatternFillData( wxNullBitmap, style );
                break;
            default:
                break;
        }
        UnRef();
        m_refData = newrefdata;
    }
}

a2dFillType a2dFill::GetTypeForStyle( a2dFillStyle style ) const
{
    if ( style == a2dFILL_NULLFILL || style == a2dFILL_LAYER || style == a2dFILL_INHERIT )
        return a2dFILL_NULL;
    else if ( style == a2dFILL_TRANSPARENT && GetType() != a2dFILL_NULL )
        return GetType();
    else if ( style == a2dFILL_SOLID || style == a2dFILL_TRANSPARENT || style == a2dFILL_NULLFILL )
        return a2dFILL_ONE_COLOUR;
    else if ( style >= a2dFIRST_TWOCOL_HATCH && style <= a2dLAST_TWOCOL_HATCH )
        return a2dFILL_HATCH_TWO_COLOUR;
    else if ( style >= a2dFIRST_HATCH && style <= a2dLAST_HATCH )
        return a2dFILL_ONE_COLOUR;
    else if ( style == a2dFILL_GRADIENT_FILL_XY_LINEAR )
        return a2dFILL_GRADIENT_FILL_LINEAR;
    else if ( style == a2dFILL_GRADIENT_FILL_XY_RADIAL )
        return a2dFILL_GRADIENT_FILL_RADIAL;
    else if ( style == a2dFILL_STIPPLE ||
              style == a2dFILL_STIPPLE_MASK_OPAQUE ||
              style == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT
            )
        return a2dFILL_BITMAP;
    else
        wxFAIL_MSG( _( "invalid style for in a2dFill" ) );

    return a2dFILL_NULL;
}

void a2dFill::SetColour2( const wxColour& col )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            UnRef();
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            ( ( a2dTwoColourFillData* )GetRefData() )->SetColour2( col );
            break;
        default:
            break;
    }
}

wxColour a2dFill::GetColour2() const
{
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dTwoColourFillData* )GetRefData() )->GetColour2();
            break;
        default:
            break;
    }
    return wxColour( 0, 0, 0, 0 ); //*wxBLACK;
}

void a2dFill::SetColour( const wxColour& col )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            UnRef();
            break;
        case a2dFILL_ONE_COLOUR:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            ( ( a2dOneColourFillData* )GetRefData() )->SetColour( col );
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            ( ( a2dTwoColourFillData* )GetRefData() )->SetColour( col );
            break;
        default:
            break;
    }
}

wxColour a2dFill::GetColour() const
{
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            break;
        case a2dFILL_ONE_COLOUR:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dOneColourFillData* )GetRefData() )->GetColour();
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
        case a2dFILL_GRADIENT_FILL_LINEAR:
        case a2dFILL_GRADIENT_FILL_RADIAL:
        case a2dFILL_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dTwoColourFillData* )GetRefData() )->GetColour();
            break;
        default:
            break;
    }
    return *wxBLACK;
}

void a2dFill::SetStipple( const wxBitmap& stipple )
{
    AllocExclusive();
    if ( GetType() == a2dFILL_BITMAP )
        ( ( a2dPatternFillData* )m_refData )->SetStipple( stipple );
    else
    {
        UnRef();
        m_refData = new a2dPatternFillData( stipple, a2dFILL_STIPPLE );
    }
}

const wxBitmap& a2dFill::GetStipple() const
{
    if ( GetType() == a2dFILL_BITMAP )
    {
        wxASSERT_MSG( m_refData, _T( "No ref data" ) );
        return ( ( const a2dPatternFillData* )m_refData )->GetStipple();
    }
    else
    {
        return wxNullBitmap;
    }
}

void a2dFill::SetFilling( bool OnOff )
{
    if ( ( ( a2dFillRefData* )GetRefData() )->m_fillOn == OnOff )
        return;

    AllocExclusive();
    ( ( a2dFillRefData* )GetRefData() )->m_fillOn = OnOff;
}

bool a2dFill::GetFilling() const
{
    switch ( GetType() )
    {
        case a2dFILL_NULL:
            return false;
    }
    return ( ( a2dFillRefData* )GetRefData() )->m_fillOn;
}

#if wxART2D_USE_CVGIO
void a2dFill::Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
{
    a2dFillType type;

    parser.Require( START_TAG, wxT( "o" ) );

    wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
    if ( classname == wxT( "a2dOneColourFill" ) )
        type = a2dFILL_ONE_COLOUR;
    else if ( classname == wxT( "a2dLinearGradientFill" ) )
        type = a2dFILL_GRADIENT_FILL_LINEAR;
    else if ( classname == wxT( "a2dRadialGradientFill" ) )
        type = a2dFILL_GRADIENT_FILL_RADIAL;
    else if ( classname == wxT( "a2dPatternFill" ) )
        type = a2dFILL_BITMAP;
    else
    {
        wxString stype = parser.GetAttributeValue( wxT( "type" ), wxT( "a2dFILL_NULL" ) );
        if ( stype == wxT( "a2dFILL_NULL" ) )
            type = a2dFILL_NULL;
        else if ( stype == wxT( "a2dFILL_ONE_COLOUR" ) )
            type = a2dFILL_ONE_COLOUR;
        else if ( stype == wxT( "a2dFILL_HATCH_TWO_COLOUR" ) )
            type = a2dFILL_HATCH_TWO_COLOUR;
        else if ( stype == wxT( "a2dFILL_GRADIENT_FILL_LINEAR" ) )
            type = a2dFILL_GRADIENT_FILL_LINEAR;
        else if ( stype == wxT( "a2dFILL_GRADIENT_FILL_RADIAL" ) )
            type = a2dFILL_GRADIENT_FILL_RADIAL;
        else if ( stype == wxT( "a2dFILL_BITMAP" ) )
            type = a2dFILL_BITMAP;
    }

    switch ( type )
    {
        case a2dFILL_NULL:
            break;
        case a2dFILL_ONE_COLOUR:
            m_refData = new a2dOneColourFillData( *wxBLACK );
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
            m_refData = new a2dHatchFillData( *wxBLACK, *wxBLACK, a2dFILL_TWOCOL_BDIAGONAL_HATCH );
            break;
        case a2dFILL_GRADIENT_FILL_LINEAR:
            m_refData = new a2dLinearGradientFillData( *wxBLACK, *wxBLACK, 0, 0, 0, 0 );
            break;
        case a2dFILL_GRADIENT_FILL_RADIAL:
            m_refData = new a2dRadialGradientFillData( *wxBLACK, *wxBLACK, 0, 0, 0, 0, 0 );
            break;
        case a2dFILL_BITMAP:
            m_refData = new a2dPatternFillData( wxNullBitmap );
            break;
        default:
            break;
    }

    if ( GetRefData() )
    {
        ( ( a2dFillRefData* )GetRefData() )->DoLoad( parent, parser, a2dXmlSer_attrib );
        parser.Next();

        ( ( a2dFillRefData* )GetRefData() )->DoLoad( parent, parser, a2dXmlSer_Content );
    }
    else
        parser.Next();

    parser.Require( END_TAG, wxT( "o" ) );
    parser.Next();
}

void a2dFill::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out , a2dObjectList* towrite )
{
    out.WriteStartElementAttributes( wxT( "o" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );

    switch ( GetType() )
    {
        case a2dFILL_NULL:
            break;
        case a2dFILL_ONE_COLOUR:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dFILL_ONE_COLOUR" ) );
            break;
        case a2dFILL_HATCH_TWO_COLOUR:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dFILL_HATCH_TWO_COLOUR" ) );
            break;
        case a2dFILL_GRADIENT_FILL_LINEAR:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dFILL_GRADIENT_FILL_LINEAR" ) );
            break;
        case a2dFILL_GRADIENT_FILL_RADIAL:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dFILL_GRADIENT_FILL_RADIAL" ) );
            break;
        case a2dFILL_BITMAP:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dFILL_BITMAP" ) );
            break;
        default:
            break;
    }

    if ( GetRefData() )
        ( ( a2dFillRefData* )GetRefData() )->DoSave( parent, out, a2dXmlSer_attrib, towrite );
    out.WriteEndAttributes();
    if ( GetRefData() )
        ( ( a2dFillRefData* )GetRefData() )->DoSave( parent, out, a2dXmlSer_Content, towrite );
    out.WriteEndElement();
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dStrokeRefData
//----------------------------------------------------------------------------

//! Internal to a2dStroke for refcounting in wxObject style
/*!
    \ingroup style
*/
class A2DARTBASEDLLEXP a2dStrokeRefData : public wxObjectRefData
{
public:

    virtual a2dStrokeType GetType( void ) const { return a2dSTROKE_NULL; }

    //!constructor
    a2dStrokeRefData()
    {
    }

    a2dStrokeRefData( const a2dStrokeRefData& other )
        : wxObjectRefData()
    {
    }

#if wxART2D_USE_CVGIO
    //! Save settings.
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite ) = 0;
    //! Load settings.
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts ) = 0;
#endif //wxART2D_USE_CVGIO

};

//----------------------------------------------------------------------------
// a2dOneColourStrokeData
//----------------------------------------------------------------------------

//! convert a style enum to a string
wxString StrokeStyle2String( a2dStrokeStyle style )
{
    switch ( style )
    {
        case a2dSTROKE_SOLID: return wxT( "a2dSTROKE_SOLID" );
        case a2dSTROKE_TRANSPARENT: return wxT( "a2dSTROKE_TRANSPARENT" );
        case a2dSTROKE_DOT: return wxT( "a2dSTROKE_DOT" );
        case a2dSTROKE_LONG_DASH: return wxT( "a2dSTROKE_LONG_DASH" );
        case a2dSTROKE_SHORT_DASH: return wxT( "a2dSTROKE_SHORT_DASH" );
        case a2dSTROKE_DOT_DASH: return wxT( "a2dSTROKE_DOT_DASH" );
        case a2dSTROKE_USER_DASH: return wxT( "a2dSTROKE_USER_DASH" );
        case a2dSTROKE_BDIAGONAL_HATCH: return wxT( "a2dSTROKE_BDIAGONAL_HATCH" );
        case a2dSTROKE_CROSSDIAG_HATCH: return wxT( "a2dSTROKE_CROSSDIAG_HATCH" );
        case a2dSTROKE_FDIAGONAL_HATCH: return wxT( "a2dSTROKE_FDIAGONAL_HATCH" );
        case a2dSTROKE_CROSS_HATCH: return wxT( "a2dSTROKE_CROSS_HATCH" );
        case a2dSTROKE_HORIZONTAL_HATCH: return wxT( "a2dSTROKE_HORIZONTAL_HATCH" );
        case a2dSTROKE_VERTICAL_HATCH: return wxT( "a2dSTROKE_VERTICAL_HATCH" );
        case a2dSTROKE_STIPPLE: return wxT( "a2dSTROKE_STIPPLE" );
        case a2dSTROKE_STIPPLE_MASK_OPAQUE: return wxT( "a2dSTROKE_STIPPLE_MASK_OPAQUE" );
        case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT: return wxT( "a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT" );
            //case a2dSTROKE_GRADIENT_STROKE_XY_LINEAR: return wxT("a2dSTROKE_GRADIENT_FILL_XY_LINEAR");
            //case a2dSTROKE_GRADIENT_STROKE_XY_RADIAL: return wxT("a2dSTROKE_GRADIENT_FILL_XY_RADIAL");
            //case a2dSTROKE_GRADIENT_STROKE_XY_DROP: return wxT("a2dSTROKE_GRADIENT_FILL_XY_DROP");
        case a2dSTROKE_INHERIT: return wxT( "a2dSTROKE_INHERIT" );
        case a2dSTROKE_LAYER: return wxT( "a2dSTROKE_LAYER" );
        case a2dSTROKE_NULLSTROKE: return wxT( "a2dSTROKE_NULLSTROKE" );

        default:
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid style" ) );
    }
    return wxT( "a2dSTROKE_NULLFILL" );
}

//! convert a string to a style enum
a2dStrokeStyle StrokeString2Style( const wxString& stylestr )
{
    if ( stylestr == wxT( "a2dSTROKE_SOLID" ) )
        return a2dSTROKE_SOLID;
    if ( stylestr == wxT( "a2dSTROKE_TRANSPARENT" ) )
        return a2dSTROKE_TRANSPARENT;
    if ( stylestr == wxT( "a2dSTROKE_DOT" ) )
        return a2dSTROKE_DOT;
    if ( stylestr == wxT( "a2dSTROKE_LONG_DASH" ) )
        return a2dSTROKE_LONG_DASH;
    if ( stylestr == wxT( "a2dSTROKE_SHORT_DASH" ) )
        return a2dSTROKE_SHORT_DASH;
    if ( stylestr == wxT( "a2dSTROKE_DOT_DASH" ) )
        return a2dSTROKE_DOT_DASH;
    if ( stylestr == wxT( "a2dSTROKE_USER_DASH" ) )
        return a2dSTROKE_USER_DASH;
    if ( stylestr == wxT( "a2dSTROKE_BDIAGONAL_HATCH" ) )
        return a2dSTROKE_BDIAGONAL_HATCH;
    if ( stylestr == wxT( "a2dSTROKE_CROSSDIAG_HATCH" ) )
        return a2dSTROKE_CROSSDIAG_HATCH;
    if ( stylestr == wxT( "a2dSTROKE_FDIAGONAL_HATCH" ) )
        return a2dSTROKE_FDIAGONAL_HATCH;
    if ( stylestr == wxT( "a2dSTROKE_CROSS_HATCH" ) )
        return a2dSTROKE_CROSS_HATCH;
    if ( stylestr == wxT( "a2dSTROKE_HORIZONTAL_HATCH" ) )
        return a2dSTROKE_HORIZONTAL_HATCH;
    if ( stylestr == wxT( "a2dSTROKE_VERTICAL_HATCH" ) )
        return a2dSTROKE_VERTICAL_HATCH;
    if ( stylestr == wxT( "a2dSTROKE_STIPPLE" ) )
        return a2dSTROKE_STIPPLE;
    if ( stylestr == wxT( "a2dSTROKE_STIPPLE_MASK_OPAQUE" ) )
        return a2dSTROKE_STIPPLE_MASK_OPAQUE;
    if ( stylestr == wxT( "a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT" ) )
        return a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT;
    /*
        if ( stylestr == wxT("a2dSTROKE_GRADIENT_STROKE_XY_LINEAR") )
            return a2dSTROKE_GRADIENT_FILL_XY_LINEAR;
        if ( stylestr == wxT("a2dSTROKE_GRADIENT_STROKE_XY_RADIAL") )
            return a2dSTROKE_GRADIENT_FILL_XY_RADIAL;
        if ( stylestr == wxT("a2dSTROKE_GRADIENT_STROKE_XY_DROP") )
            return a2dSTROKE_GRADIENT_FILL_XY_DROP;
    */
    if ( stylestr == wxT( "a2dSTROKE_INHERIT" ) )
        return a2dSTROKE_INHERIT;
    if ( stylestr == wxT( "a2dSTROKE_LAYER" ) )
        return a2dSTROKE_LAYER;
    if ( stylestr == wxT( "a2dSTROKE_NULLSTROKE" ) )
        return a2dSTROKE_NULLSTROKE;

    if ( !stylestr.IsEmpty() ) //old style settings
    {
        long num;
        stylestr.ToLong( &num );
        return ( a2dStrokeStyle ) num;
    }

    a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid style %s" ), stylestr.c_str() );

    return a2dSTROKE_NULLSTROKE;
}


//! One Colour Fill
/*!
A second colour is not saved, but when asked for it return wxBLACK

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dOneColourStrokeData : public a2dStrokeRefData
{

private:

    friend class a2dStroke;

public:

    virtual a2dStrokeType GetType( void ) const { return a2dSTROKE_ONE_COLOUR; }

protected:

    //!constructor
    a2dOneColourStrokeData( void );

    a2dOneColourStrokeData( const a2dOneColourStrokeData& fill );

    //!constructor
    a2dOneColourStrokeData( const wxColour& col, a2dStrokeStyle style = a2dSTROKE_SOLID );

    a2dOneColourStrokeData( const wxPen& stroke );

    //!destructor
    ~a2dOneColourStrokeData( void ) {}

    static a2dOneColourStrokeData* Create( const wxColour& col, a2dStrokeStyle style = a2dSTROKE_SOLID );

    void SetColour( const wxColour& col ) { m_colour1 = col; }

    //! return colour
    wxColour GetColour() const { return m_colour1; }

    void SetJoin( wxPenJoin join_style ) { m_join = join_style; }

    void SetCap( wxPenCap cap_style ) { m_cap = cap_style; }

    wxPenCap GetCap() { return m_cap; }

    wxPenJoin GetJoin() { return m_join; }

    //!Set width of stroke in world coordinates.
    void SetWidth( float width )  { m_width = width; }

    //!Get width of stroke in world or device coordinates.
    //!(depending on pixel or non pixel stroke).
    virtual float GetWidth() const  { return m_width; }

    virtual float GetExtend() const { return m_width; }

    //!set pixelstoke flag, stroke width is defined in pixels else in worldcoordinates
    void SetPixelStroke( bool pixelstroke ) { m_pixelstroke = pixelstroke; }

    //!if the width is pixels or not.
    inline bool GetPixelStroke() const { return m_pixelstroke; }

    a2dStrokeStyle GetStyle( void ) const;

    void SetStyle( a2dStrokeStyle style );

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    wxColour m_colour1;

    //!width of stroke in world coordinates
    float m_width;

    //!set if stroke is in pixels
    bool  m_pixelstroke;

    //!Sets the pen join style, which may be one of wxJOIN_BEVEL, wxJOIN_ROUND (default) and wxJOIN_MITER.
    wxPenJoin m_join;

    //!Sets the pen cap style, which may be one of wxCAP_ROUND (default), wxCAP_PROJECTING and wxCAP_BUTT.
    wxPenCap m_cap;

    a2dDash* m_dash;

    a2dStrokeStyle m_style;

};

a2dOneColourStrokeData* a2dOneColourStrokeData::Create( const wxColour& col, a2dStrokeStyle style )
{
    return new a2dOneColourStrokeData( col, style );
}

a2dOneColourStrokeData::a2dOneColourStrokeData( void )
    : a2dStrokeRefData()
{
    m_style = a2dSTROKE_SOLID;
    m_colour1 = *wxBLACK;
    m_join = wxJOIN_ROUND;
    m_cap = wxCAP_ROUND;

    m_pixelstroke = false;
    m_width = 0.0;

}

a2dOneColourStrokeData::a2dOneColourStrokeData( const wxColour& col, a2dStrokeStyle style )
    : a2dStrokeRefData()
{
    m_colour1 = col;
    m_join = wxJOIN_ROUND;
    m_cap = wxCAP_ROUND;

    m_pixelstroke = false;
    m_width = 0.0;

    switch( style )
    {
        case a2dSTROKE_SOLID:
        case a2dSTROKE_TRANSPARENT:
        case a2dSTROKE_DOT:
        case a2dSTROKE_DOT_DASH:
        case a2dSTROKE_LONG_DASH:
        case a2dSTROKE_SHORT_DASH:
        case a2dSTROKE_USER_DASH:
        case a2dSTROKE_BDIAGONAL_HATCH:
        case a2dSTROKE_CROSSDIAG_HATCH:
        case a2dSTROKE_FDIAGONAL_HATCH:
        case a2dSTROKE_CROSS_HATCH:
        case a2dSTROKE_HORIZONTAL_HATCH:
        case a2dSTROKE_VERTICAL_HATCH:
        case a2dSTROKE_NULLSTROKE:
            m_style = style;
            break;

        case a2dSTROKE_STIPPLE: //  Use the stipple bitmap.
        case a2dSTROKE_STIPPLE_MASK_OPAQUE:
        case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT:
            m_style = style;
            break;

        default:
            wxFAIL_MSG( _( "invalid style for a2dStroke" ) );
    }
}

a2dOneColourStrokeData::a2dOneColourStrokeData( const wxPen& stroke )
    : a2dStrokeRefData()
{
    m_join = stroke.GetJoin();
    m_cap = stroke.GetCap();

    m_colour1 = stroke.GetColour();

    m_pixelstroke = true;
    if ( stroke != wxNullPen )
    {
        m_width = ( float ) stroke.GetWidth();
        switch( stroke.GetStyle() )
        {
            case wxSOLID: m_style = a2dSTROKE_SOLID;
                break;
            case wxTRANSPARENT: m_style = a2dSTROKE_TRANSPARENT;
                break;
            case wxDOT: m_style = a2dSTROKE_DOT;
                break;
            case wxLONG_DASH: m_style = a2dSTROKE_LONG_DASH;
                break;
            case wxSHORT_DASH: m_style = a2dSTROKE_SHORT_DASH;
                break;
            case wxDOT_DASH: m_style = a2dSTROKE_DOT_DASH;
                break;
            case wxBDIAGONAL_HATCH: m_style = a2dSTROKE_BDIAGONAL_HATCH;
                break;
            case wxCROSSDIAG_HATCH: m_style = a2dSTROKE_CROSSDIAG_HATCH;
                break;
            case wxFDIAGONAL_HATCH: m_style = a2dSTROKE_FDIAGONAL_HATCH;
                break;
            case wxCROSS_HATCH: m_style = a2dSTROKE_CROSS_HATCH;
                break;
            case wxHORIZONTAL_HATCH: m_style = a2dSTROKE_HORIZONTAL_HATCH;
                break;
            case wxVERTICAL_HATCH: m_style = a2dSTROKE_VERTICAL_HATCH;
                break;
            default:
                wxFAIL_MSG( _( "invalid style for a2dOneColourStroke" ) );
        }
    }
    else
    {
        m_width = 0;
    }
}

a2dOneColourStrokeData::a2dOneColourStrokeData( const a2dOneColourStrokeData& other )
    : a2dStrokeRefData( other )
{
    m_colour1 = other.m_colour1;
    m_style = other.m_style;
    m_join = other.m_join;
    m_cap = other.m_cap;
    m_pixelstroke = other.m_pixelstroke;
    m_width = other.m_width;
    m_dash = other.m_dash;
}

a2dStrokeStyle a2dOneColourStrokeData::GetStyle() const
{
    return m_style;
}

void a2dOneColourStrokeData::SetStyle( a2dStrokeStyle style )
{
    switch( style )
    {
        case a2dSTROKE_SOLID:
        case a2dSTROKE_TRANSPARENT:
        case a2dSTROKE_DOT:
        case a2dSTROKE_DOT_DASH:
        case a2dSTROKE_LONG_DASH:
        case a2dSTROKE_SHORT_DASH:
        case a2dSTROKE_USER_DASH:
        case a2dSTROKE_BDIAGONAL_HATCH:
        case a2dSTROKE_CROSSDIAG_HATCH:
        case a2dSTROKE_FDIAGONAL_HATCH:
        case a2dSTROKE_CROSS_HATCH:
        case a2dSTROKE_HORIZONTAL_HATCH:
        case a2dSTROKE_VERTICAL_HATCH:
        case a2dSTROKE_NULLSTROKE:
            m_style = style;
            break;

        case a2dSTROKE_STIPPLE: //  Use the stipple bitmap.
        case a2dSTROKE_STIPPLE_MASK_OPAQUE:
        case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT:
            m_style = style;
            break;

        default:
            wxFAIL_MSG( _( "invalid style for a2dStroke" ) );
    }
}


#if wxART2D_USE_CVGIO
void a2dOneColourStrokeData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dSTROKE_SOLID )
            out.WriteAttribute( wxT( "style" ),  StrokeStyle2String( GetStyle() ) );

        if ( m_style != a2dSTROKE_TRANSPARENT )
        {
			switch ( m_cap ) 
			{
				case wxCAP_ROUND:
					//default
	                //out.WriteAttribute( wxT( "cap" ), "round" );
					break;
				case wxCAP_PROJECTING:
	                out.WriteAttribute( wxT( "cap" ), "projecting" );
					break;
				case wxCAP_BUTT:
	                out.WriteAttribute( wxT( "cap" ), "butt" );
					break;
				default:
					break;
			}
			switch ( m_join ) 
			{
				case wxJOIN_BEVEL:
	                out.WriteAttribute( wxT( "join" ), "bevel" );
					break;
				case wxJOIN_MITER:
	                out.WriteAttribute( wxT( "join" ), "miter" );
					break;
				case wxJOIN_ROUND:
					//default
	                //out.WriteAttribute( wxT( "join" ), "round" );
					break;
				default:
					break;
			}
            wxString strokeColour = wxTheColourDatabase->FindName( m_colour1 );
            if ( strokeColour.IsEmpty() )
            {
                out.WriteAttribute( wxT( "colour" ), wxT( "#" ) + ColourToHex( m_colour1 ) );
            }
            else
            {
                out.WriteAttribute( wxT( "colour" ), strokeColour );
            }

            if ( GetPixelStroke() )
            {
                out.WriteAttribute( wxT( "pixelwidth" ), GetPixelStroke() );
            }

            if ( m_width != 0 )
            {
                out.WriteAttribute( wxT( "width" ), m_width );
            }
        }
    }
    else
    {}
}

void a2dOneColourStrokeData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_style = ( a2dStrokeStyle ) StrokeString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dSTROKE_SOLID" ) ) );
        SetStyle( m_style );
		wxString cap =  parser.GetAttributeValue( wxT( "cap" ) );
		if ( cap == "round")
			m_cap = wxCAP_ROUND;
		else if ( cap == "projecting")
			m_cap = wxCAP_PROJECTING;
		else if ( cap == "butt")
			m_cap = wxCAP_BUTT;

		wxString join =  parser.GetAttributeValue( wxT( "join" ) );
		if ( join == "bevel")
			m_join = wxJOIN_BEVEL;
		else if ( join == "miter")
			m_join = wxJOIN_MITER;
		else if ( join == "round")
			m_join = wxJOIN_ROUND;

        wxString strokeColour =  parser.GetAttributeValue( wxT( "colour" ) );
        if ( !strokeColour.IsEmpty() )
        {
            if ( strokeColour.GetChar( 0 ) == wxT( '#' ) )
            {
                m_colour1 = HexToColour( strokeColour.After( wxT( '#' ) ) );
            }
            else
            {
                m_colour1 = wxTheColourDatabase->Find( strokeColour );
                if ( !m_colour1.Ok() )
                    m_colour1 =  *wxBLACK;
            }
        }
        else
            m_colour1 = *wxBLACK;

        m_pixelstroke = parser.GetAttributeValueBool( wxT( "pixelwidth" ), false );

        m_width = ( float ) parser.GetAttributeValueDouble( wxT( "width" ) );
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dPatternStrokeData
//----------------------------------------------------------------------------

//!Filling using a wxbitmap
/*!
To Fill shapes with a bitmap. a2dSTROKE_STIPPLE
In case a wxMask is set, to do transparent stipple drawing use style a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT.
The fill colour1 will be used for non Transparent parts in the brush bitmap,
while other parts will be transparent.
In case a wxMask is set, use style wxSTIPPLE_MASK_OPAQUE to use colour1 and colour2 to fill with
the wxMask in those colours.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dPatternStrokeData: public a2dOneColourStrokeData
{

private:

    friend class a2dStroke;

public:

    virtual a2dStrokeType GetType( void ) const { return a2dSTROKE_BITMAP; }

protected:

    //!constructor
    a2dPatternStrokeData();

    //!constructor
    a2dPatternStrokeData( const a2dPatternStrokeData& other );

    //!constructor
    /*!
     \param stipple bitmap for stippling the primitive drawn.
     \param style can be a2dSTROKE_STIPPLE a2dSTROKE_STIPPLE_MASK_OPAQUE, a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT

    */
    a2dPatternStrokeData( const wxBitmap& stipple, a2dStrokeStyle style = a2dSTROKE_STIPPLE );

    a2dPatternStrokeData( const wxFileName& filename, a2dStrokeStyle style = a2dSTROKE_STIPPLE );

    //!destructor
    ~a2dPatternStrokeData( void ) {}

    //! set stipple style
    void SetStipple( const wxBitmap& stipple ) { m_stipple = stipple; }

    const wxBitmap& GetStipple() const { return m_stipple; }

    //! can contain the filename if pattern was read froma file
    /*!
        Certain input formats ( Like CVG ) can hold the path to the pattern.
    */
    wxString GetFileName() { return m_filename; }

    a2dStrokeStyle GetStyle( void ) const;

    void SetStyle( a2dStrokeStyle style );

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    wxBitmap m_stipple;

    //! if the pattern was loaded from a file, this contains the path to this file.
    //! a2dGlobals->GetImagePathList() contains the search path for the file.
    wxString m_filename;
};

a2dPatternStrokeData::a2dPatternStrokeData()
{
    m_stipple = wxNullBitmap;
    m_style = a2dSTROKE_STIPPLE;
    SetStyle( m_style );
}

a2dPatternStrokeData::a2dPatternStrokeData( const wxBitmap& stipple, a2dStrokeStyle style )
{
    m_stipple = stipple;

    wxASSERT_MSG( m_stipple.Ok(), wxT( "Stipple Bitmap not oke" ) );
    if ( style == a2dSTROKE_STIPPLE_MASK_OPAQUE || style == a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        wxASSERT_MSG( m_stipple.GetMask(), wxT( "require a mask for a2dSTROKE_STIPPLE_MASK_OPAQUE or a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT" ) );
    }
    SetStyle( style );
}

a2dPatternStrokeData::a2dPatternStrokeData( const wxFileName& filename, a2dStrokeStyle style )
{
    m_filename = filename.GetFullPath();

    wxImage image;
    if ( m_filename.IsEmpty() )
    {
        wxLogError( _( "CVG : bitmap file for a2dPatternFill not specified or empty" ) );
        image = wxImage( 32, 32 );
    }
    else
    {
        wxString filename = a2dGlobals->GetImagePathList().FindValidPath( m_filename );
        if ( filename.IsEmpty() )
        {
            wxLogError( _( "CVG : Could not load bitmap \"%s\" for a2dPatternStroke from path \"%s\" " ), m_filename.c_str(), a2dGlobals->GetImagePathList().GetAsString().c_str() );
            image = wxImage( 32, 32 );
        }
        else if ( ! image.LoadFile( filename, wxBITMAP_TYPE_BMP ) )
        {
            wxLogError( _( "CVG : Could not load bitmap %s for a2dPatternStroke" ), filename.c_str() );
            image = wxImage( 32, 32 );
        }
    }
    m_stipple = wxBitmap( image );

    if ( m_style == a2dSTROKE_STIPPLE_MASK_OPAQUE || m_style == a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        wxColour white( wxT( "WHITE" ) );
        wxColour black( wxT( "BLACK" ) );

        wxMask* monochrome_mask = new wxMask( m_stipple, black );
        m_stipple.SetMask( monochrome_mask );
    }
    SetStyle( style );
}

a2dPatternStrokeData::a2dPatternStrokeData( const a2dPatternStrokeData& other ): a2dOneColourStrokeData( other )
{
    m_stipple = other.m_stipple;
}

a2dStrokeStyle a2dPatternStrokeData::GetStyle() const
{
    return m_style;
}

void a2dPatternStrokeData::SetStyle( a2dStrokeStyle style )
{
    if ( style == a2dSTROKE_TRANSPARENT )
        m_style = a2dSTROKE_TRANSPARENT;
    if ( style == a2dSTROKE_STIPPLE_MASK_OPAQUE ||
            style == a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT
       )
    {
        if ( !m_stipple.GetMask() )
        {
            wxMask* mask = new wxMask( m_stipple, GetColour() );
            m_stipple.SetMask( mask );
        }
        m_style = style;
    }
    else if ( style == a2dSTROKE_STIPPLE )
        m_style = style;
    else
        wxFAIL_MSG( _( "invalid style for a2dPatternStrokeData" ) );
}

#if wxART2D_USE_CVGIO
void a2dPatternStrokeData::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dOneColourStrokeData::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString filename = parser.GetAttributeValue( wxT( "filename" ), wxT( "" ) );
        wxImage image;
        if ( filename.IsEmpty() )
        {
            wxLogError( _( "CVG : bitmap file for a2dPatternStrokeData not specified or empty" ) );
            image = wxImage( 32, 32 );
        }
        else
        {
            m_filename = a2dGlobals->GetImagePathList().FindValidPath( filename );
            if ( m_filename.IsEmpty() )
            {
                wxLogError( _( "CVG : Could not load bitmap \"%s\" for a2dPatternStrokeData from path \"%s\" " ), filename.c_str(), a2dGlobals->GetImagePathList().GetAsString().c_str() );
                image = wxImage( 32, 32 );
            }
            else if ( ! image.LoadFile( m_filename, wxBITMAP_TYPE_BMP ) )
            {
                wxLogError( _( "CVG : Could not load bitmap %s for a2dPatternStrokeData" ), m_filename.c_str() );
                image = wxImage( 32, 32 );
            }
        }
        m_stipple = wxBitmap( image );

        m_style = ( a2dStrokeStyle ) StrokeString2Style( parser.GetAttributeValue( wxT( "style" ), wxT( "a2dSTROKE_STIPPLE" ) ) );
        SetStyle( m_style );

        if ( m_style == a2dSTROKE_STIPPLE_MASK_OPAQUE || m_style == a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT )
        {
            wxColour white( wxT( "WHITE" ) );
            wxColour black( wxT( "BLACK" ) );

            wxMask* monochrome_mask = new wxMask( m_stipple, black );
            m_stipple.SetMask( monochrome_mask );
        }
    }
    else
    {}
}

void a2dPatternStrokeData::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dOneColourStrokeData::DoSave( parent, out, xmlparts, towrite );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_style != a2dSTROKE_STIPPLE )
            out.WriteAttribute( wxT( "style" ),  StrokeStyle2String( GetStyle() ) );

        wxString filename = a2dGlobals->GetImagePathList().FindValidPath( m_filename );
        if ( filename.IsEmpty() )
        {
            // We can/may not save/use the original files, because the pattern may have changed inside the appl.
            // If available use the name of the orginal file.
            wxString fileTosave;

            wxString* layerfile = a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "wxart2dlayers" ) );
            wxString* layerfiledir = a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "layerFileSavePath" ) );

            if ( layerfile && !layerfile->IsEmpty() )
            {
                if ( !wxDirExists( *layerfiledir ) )
                    wxMkdir( *layerfiledir );
                wxString mess = *layerfiledir + *layerfile + wxT( " directory does not exist, and could not be created" );
                wxASSERT_MSG( wxDirExists( *layerfiledir ) , mess );

                fileTosave = *layerfiledir + wxFILE_SEP_PATH;
            }

            if ( m_filename.IsEmpty() )
            {
                m_filename.Printf( _T( "canpat_%ld.bmp" ), wxGenNewId() );
                fileTosave += m_filename;
            }
            else
                fileTosave += wxFileNameFromPath( m_filename );
            bool isOke = m_stipple.SaveFile( fileTosave, wxBITMAP_TYPE_BMP );
            if ( !isOke )
                wxLogError( _( "Could not save bitmap %s for a2dPatternStrokeData" ), fileTosave.c_str() );
            out.WriteAttribute( wxT( "filename" ), wxFileNameFromPath( m_filename ) );
        }
        else
            out.WriteAttribute( wxT( "filename" ), wxFileNameFromPath( m_filename ) );
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dStroke
//----------------------------------------------------------------------------

a2dStroke::a2dStroke( void )
{
}

a2dStroke::a2dStroke( const wxColour& col, a2dStrokeStyle style )
{
    m_refData = new a2dOneColourStrokeData( col, style );
}

a2dStroke::a2dStroke( const wxColour& col, double width, a2dStrokeStyle style )
{
    m_refData = new a2dOneColourStrokeData( col, style );
    ( ( a2dOneColourStrokeData* )GetRefData() )->SetWidth( width );
}

a2dStroke::a2dStroke( const wxColour& col, int width, a2dStrokeStyle style )
{
    m_refData = new a2dOneColourStrokeData( col, style );
    ( ( a2dOneColourStrokeData* )GetRefData() )->SetWidth( width );
    ( ( a2dOneColourStrokeData* )GetRefData() )->SetPixelStroke( true );
}

a2dStroke::a2dStroke( const wxBitmap& stipple, a2dStrokeStyle style )
{
    m_refData = new a2dPatternStrokeData( stipple, style );
}

a2dStroke::a2dStroke( const wxBitmap& stipple, const wxColour& col, const wxColour& col2, a2dStrokeStyle style )
{
    m_refData = new a2dPatternStrokeData( stipple, style );
    SetColour( col );
    //SetColour2( col2 );
}

a2dStroke::a2dStroke( const wxFileName& filename, a2dStrokeStyle style )
{
    m_refData = new a2dPatternStrokeData( filename, style );
}

a2dStroke::a2dStroke( const wxFileName& filename, const wxColour& col, const wxColour& col2, a2dStrokeStyle style )
{
    m_refData = new a2dPatternStrokeData( filename, style );
    SetColour( col );
    //SetColour2( col2 );
}

a2dStroke::a2dStroke( const wxPen& stroke )
{
    m_refData = new a2dOneColourStrokeData( stroke );
}

a2dStroke::a2dStroke( const a2dStroke& Stroke )
    : wxObject( Stroke )
{
}


a2dStroke::~a2dStroke( void )
{
}

a2dStroke& a2dStroke::UnShare()
{
    wxObject::UnShare();
    return *this;
}

a2dStrokeType a2dStroke::GetType( void ) const
{
    if ( !m_refData )
        return a2dSTROKE_NULL;
    return ( ( a2dStrokeRefData* ) m_refData )->GetType();
}

wxObjectRefData* a2dStroke::CreateRefData() const
{
    return new a2dOneColourStrokeData(); //dummy will be removed soon if not needed.
}

wxObjectRefData* a2dStroke::CloneRefData( const wxObjectRefData* data ) const
{
    switch ( ( ( a2dStrokeRefData* ) data )->GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return new a2dOneColourStrokeData( *( ( const a2dOneColourStrokeData* )data ) );
            break;
        case a2dSTROKE_BITMAP:
            return new a2dPatternStrokeData( *( ( const a2dPatternStrokeData* )data ) );
            break;
        default:
            break;
    }
    return NULL;
}

void a2dStroke::SetStipple( const wxBitmap& stipple )
{
    AllocExclusive();
    if ( GetType() == a2dSTROKE_BITMAP )
        ( ( a2dPatternStrokeData* )m_refData )->SetStipple( stipple );
    else
    {
        UnRef();
        m_refData = new a2dPatternStrokeData( stipple, a2dSTROKE_STIPPLE );
    }
}

const wxBitmap& a2dStroke::GetStipple() const
{
    if ( GetType() == a2dSTROKE_BITMAP )
    {
        wxASSERT_MSG( m_refData, _T( "No ref data" ) );
        return ( ( const a2dPatternStrokeData* )m_refData )->GetStipple();
    }
    else
    {
        return wxNullBitmap;
    }
}

void a2dStroke::SetAlpha( wxUint8 val )
{
    AllocExclusive();

    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
        {
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            wxColour newcol = ( ( a2dOneColourStrokeData* )GetRefData() )->GetColour();
            newcol.Set( newcol.Red(), newcol.Green(), newcol.Blue(), val );
            ( ( a2dOneColourStrokeData* )GetRefData() )->SetColour( newcol );
            break;
        }
        case a2dSTROKE_BITMAP:
        {
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            wxColour newcol = ( ( a2dOneColourStrokeData* )GetRefData() )->GetColour();
            newcol.Set( newcol.Red(), newcol.Green(), newcol.Blue(), val );
            ( ( a2dPatternStrokeData* )GetRefData() )->SetColour( newcol );
            break;
        }
        default:
            break;
    }
}

wxUint8 a2dStroke::GetAlpha() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            return a2dSTROKE_NULLSTROKE;
        case a2dSTROKE_ONE_COLOUR:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetColour().Alpha();
            break;
        case a2dSTROKE_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetColour().Alpha();
            break;
        default:
            break;
    }
    return 255;
}


void a2dStroke::SetColour( const wxColour& col )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
            ( ( a2dOneColourStrokeData* )GetRefData() )->SetColour( col );
            break;
        case a2dSTROKE_BITMAP:
            ( ( a2dPatternStrokeData* )GetRefData() )->SetColour( col );
            break;
        default:
            break;
    }
}

wxColour a2dStroke::GetColour() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetColour();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetColour();
            break;
        default:
            break;
    }
    return *wxBLACK;
}

void a2dStroke::SetColour2( const wxColour& col )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            break;
        case a2dSTROKE_BITMAP:
            wxASSERT_MSG( m_refData, _T( "No ref data" ) );
            //((a2dPatternStrokeData*)GetRefData())->SetColour2( col );
            ( ( a2dPatternStrokeData* )GetRefData() )->SetColour( col );
            break;
        default:
            break;
    }
}

wxColour a2dStroke::GetColour2() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            break;
        case a2dSTROKE_BITMAP:
            //return ((a2dPatternStrokeData*)GetRefData())->GetColour2();
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetColour();
            break;
        default:
            break;
    }
    return *wxBLACK;
}

void a2dStroke::SetJoin( wxPenJoin join_style )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
            ( ( a2dOneColourStrokeData* )GetRefData() )->SetJoin( join_style );
            break;
        case a2dSTROKE_BITMAP:
            ( ( a2dPatternStrokeData* )GetRefData() )->SetJoin( join_style );
            break;
        default:
            break;
    }
}

void a2dStroke::SetCap( wxPenCap cap_style )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
            ( ( a2dOneColourStrokeData* )GetRefData() )->SetCap( cap_style );
            break;
        case a2dSTROKE_BITMAP:
            ( ( a2dPatternStrokeData* )GetRefData() )->SetCap( cap_style );
            break;
        default:
            break;
    }
}

wxPenCap a2dStroke::GetCap() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetCap();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetCap();
            break;
        default:
            break;
    }
    return wxCAP_ROUND;
}

wxPenJoin a2dStroke::GetJoin() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetJoin();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetJoin();
            break;
        default:
            break;
    }
    return wxJOIN_ROUND;
}

void a2dStroke::SetWidth( float width )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
            ( ( a2dOneColourStrokeData* )GetRefData() )->SetWidth( width );
            break;
        case a2dSTROKE_BITMAP:
            ( ( a2dPatternStrokeData* )GetRefData() )->SetWidth( width );
            break;
        default:
            break;
    }
}

float a2dStroke::GetWidth() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetWidth();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetWidth();
            break;
        default:
            break;
    }
    return 0;
}
float a2dStroke::GetExtend() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetExtend();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetExtend();
            break;
        default:
            break;
    }
    return 0;
}

void a2dStroke::SetPixelStroke( bool pixelstroke )
{
    AllocExclusive();
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            UnRef();
            break;
        case a2dSTROKE_ONE_COLOUR:
            ( ( a2dOneColourStrokeData* )GetRefData() )->SetPixelStroke( pixelstroke );
            break;
        case a2dSTROKE_BITMAP:
            ( ( a2dPatternStrokeData* )GetRefData() )->SetPixelStroke( pixelstroke );
            break;
        default:
            break;
    }
}

bool a2dStroke::GetPixelStroke() const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetPixelStroke();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetPixelStroke();
            break;
        default:
            break;
    }
    return false;
}

a2dStrokeStyle a2dStroke::GetStyle( void ) const
{
    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            return a2dSTROKE_NULLSTROKE;
            break;
        case a2dSTROKE_ONE_COLOUR:
            return ( ( a2dOneColourStrokeData* )GetRefData() )->GetStyle();
            break;
        case a2dSTROKE_BITMAP:
            return ( ( a2dPatternStrokeData* )GetRefData() )->GetStyle();
            break;
        default:
            break;
    }
    return a2dSTROKE_NULLSTROKE;
}

void a2dStroke::SetStyle( a2dStrokeStyle style )
{
    a2dStrokeStyle cur_style = GetStyle();
    if ( cur_style == style )
        return;
    AllocExclusive();

    a2dStrokeType type = GetTypeForStyle( style );

    if ( type == GetType() )
    {
        switch ( type )
        {
            case a2dSTROKE_NULL:
                UnRef();
                break;
            case a2dSTROKE_ONE_COLOUR:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dOneColourStrokeData* )GetRefData() )->SetStyle( style );
                break;
                /*
                        case a2dSTROKE_GRADIENT_FILL_LINEAR:
                            wxASSERT_MSG( m_refData, _T("No ref data") );
                            ((a2dLinearGradientFillData*)GetRefData())->SetStyle( style );
                            break;
                        case a2dSTROKE_GRADIENT_FILL_RADIAL:
                            wxASSERT_MSG( m_refData, _T("No ref data") );
                            ((a2dRadialGradientFillData*)GetRefData())->SetStyle( style );
                            break;
                */
            case a2dSTROKE_BITMAP:
                wxASSERT_MSG( m_refData, _T( "No ref data" ) );
                ( ( a2dPatternStrokeData* )GetRefData() )->SetStyle( style );
                break;
            default:
                break;
        }
    }
    else
    {
        a2dStrokeRefData* newrefdata = NULL;
        switch ( type )
        {
            case a2dSTROKE_NULL:
                break;
            case a2dSTROKE_ONE_COLOUR:
            {
                a2dOneColourStrokeData* onenewrefdata = new a2dOneColourStrokeData( GetColour(), style );
                newrefdata = onenewrefdata;
                onenewrefdata->SetColour( GetColour() );
                onenewrefdata->SetWidth( GetWidth() );
                onenewrefdata->SetJoin( GetJoin() );
                onenewrefdata->SetCap( GetCap() );
                onenewrefdata->SetPixelStroke( GetPixelStroke() );
                break;
            }
            /*
                    case a2dSTROKE_GRADIENT_FILL_LINEAR:
                        newrefdata = new a2dLinearGradientStrokeData( GetColour(), GetColour2(), 0, 0, 0, 0 );
                        break;
                    case a2dSTROKE_GRADIENT_FILL_RADIAL:
                        newrefdata = new a2dRadialGradientStrokeData( GetColour(), GetColour2(), 0, 0, 0, 0, 0 );
                        break;
            */
            case a2dSTROKE_BITMAP:
            {
                a2dPatternStrokeData* patnewrefdata = new a2dPatternStrokeData( wxNullBitmap, style );
                newrefdata = patnewrefdata;
                patnewrefdata->SetColour( GetColour() );
                patnewrefdata->SetWidth( GetWidth() );
                patnewrefdata->SetJoin( GetJoin() );
                patnewrefdata->SetCap( GetCap() );
                patnewrefdata->SetPixelStroke( GetPixelStroke() );
                //patnewrefdata->SetColour2( GetColour2() );
                break;
            }
            default:
                break;
        }
        UnRef();
        m_refData = newrefdata;
    }
}

a2dStrokeType a2dStroke::GetTypeForStyle( a2dStrokeStyle style ) const
{
    if ( style == a2dSTROKE_NULLSTROKE || style == a2dSTROKE_LAYER || style == a2dSTROKE_INHERIT )
        return a2dSTROKE_NULL;
    if ( style == a2dSTROKE_TRANSPARENT && GetType() != a2dSTROKE_NULL )
        return GetType();
    else if ( style == a2dSTROKE_SOLID ||
              style == a2dSTROKE_TRANSPARENT ||
              style == a2dSTROKE_DOT ||
              style == a2dSTROKE_DOT_DASH ||
              style == a2dSTROKE_LONG_DASH ||
              style == a2dSTROKE_SHORT_DASH ||
              style == a2dSTROKE_USER_DASH ||
              style == a2dSTROKE_BDIAGONAL_HATCH ||
              style == a2dSTROKE_CROSSDIAG_HATCH ||
              style == a2dSTROKE_FDIAGONAL_HATCH ||
              style == a2dSTROKE_CROSS_HATCH ||
              style == a2dSTROKE_HORIZONTAL_HATCH ||
              style == a2dSTROKE_VERTICAL_HATCH ||
              style == a2dSTROKE_NULLSTROKE
            )
        return a2dSTROKE_ONE_COLOUR;
    /*
        else if ( style == a2dSTROKE_GRADIENT_FILL_XY_LINEAR )
            return a2dSTROKE_GRADIENT_FILL_LINEAR;
        else if ( style == a2dSTROKE_GRADIENT_FILL_XY_RADIAL )
            return a2dSTROKE_GRADIENT_FILL_RADIAL;
    */
    else if ( style == a2dSTROKE_STIPPLE ||
              style == a2dSTROKE_STIPPLE_MASK_OPAQUE ||
              style == a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT
            )
        return a2dSTROKE_BITMAP;
    else
        wxFAIL_MSG( _( "invalid style for in a2dStroke" ) );

    return a2dSTROKE_NULL;
}


#if wxART2D_USE_CVGIO
void a2dStroke::Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
{
    a2dStrokeType type;

    parser.Require( START_TAG, wxT( "o" ) );

    wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
    if ( classname == wxT( "a2dOneColourStroke" ) )
        type = a2dSTROKE_ONE_COLOUR;
    else if ( classname == wxT( "a2dPatternFill" ) )
        type = a2dSTROKE_BITMAP;
    else
    {
        wxString stype = parser.GetAttributeValue( wxT( "type" ), wxT( "a2dSTROKE_NULL" ) );
        if ( stype == wxT( "a2dSTROKE_NULL" ) )
            type = a2dSTROKE_NULL;
        if ( stype == wxT( "a2dSTROKE_ONE_COLOUR" ) )
            type = a2dSTROKE_ONE_COLOUR;
        else if ( stype == wxT( "a2dSTROKE_BITMAP" ) )
            type = a2dSTROKE_BITMAP;
    }

    switch ( type )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            m_refData = new a2dOneColourStrokeData( *wxBLACK );
            break;
        case a2dSTROKE_BITMAP:
            m_refData = new a2dPatternStrokeData( wxNullBitmap );
            break;
        default:
            break;
    }

    if ( GetRefData() )
    {
        ( ( a2dStrokeRefData* )GetRefData() )->DoLoad( parent, parser, a2dXmlSer_attrib );
        parser.Next();
        ( ( a2dStrokeRefData* )GetRefData() )->DoLoad( parent, parser, a2dXmlSer_Content );
    }
    else
        parser.Next();

    parser.Require( END_TAG, wxT( "o" ) );
    parser.Next();
}

void a2dStroke::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite )
{
    out.WriteStartElementAttributes( wxT( "o" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );

    switch ( GetType() )
    {
        case a2dSTROKE_NULL:
            break;
        case a2dSTROKE_ONE_COLOUR:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dSTROKE_ONE_COLOUR" ) );
            break;
        case a2dSTROKE_BITMAP:
            out.WriteAttribute( wxT( "type" ), wxT( "a2dSTROKE_BITMAP" ) );
            break;
        default:
            break;
    }
    if ( GetRefData() )
        ( ( a2dStrokeRefData* )GetRefData() )->DoSave( parent, out, a2dXmlSer_attrib, towrite );

    out.WriteEndAttributes();
    if ( GetRefData() )
        ( ( a2dStrokeRefData* )GetRefData() )->DoSave( parent, out, a2dXmlSer_Content, towrite );

    out.WriteEndElement();
}
#endif //wxART2D_USE_CVGIO



//===========================================================================
// a2dExtStroke
//===========================================================================

bool a2dExtStroke::HasIndividualAttributes(a2dStrokeStyle style)
{
	if( (style == a2dSTROKE_TRANSPARENT) || (style == a2dSTROKE_LAYER) ||
	    (style == a2dSTROKE_NULLSTROKE)  || (style == a2dSTROKE_INHERIT) )
	{
		return false;
	}
	return true;
}

a2dExtStroke::a2dExtStroke()
	: m_Default(), m_Style(), m_Color(), m_Width(), m_Cap(), m_Join(), m_bStyle(false), m_bInitialised(false), 
	  m_bEditable(false), m_bColor(false), m_bWidth(false), m_bCap(false), m_bJoin(false)
{
}

a2dExtStroke::a2dExtStroke(const a2dStroke& stroke) 
	: m_Default(), m_Style(), m_Color(), m_Width(), m_Cap(), m_Join(), m_bStyle(false), m_bInitialised(false), 
	  m_bEditable(false), m_bColor(false), m_bWidth(false), m_bCap(false), m_bJoin(false)
{
	Set(stroke);
}

// Initialises all attributes with <stroke>.
void a2dExtStroke::Set(const a2dStroke& stroke)
{
	m_Style = stroke.GetStyle();
	m_bStyle = true;

	if(HasIndividualAttributes(stroke.GetStyle()))
		SetAttributes(stroke);
}

void a2dExtStroke::SetAttributes(const a2dStroke& stroke)
{
	m_Color = stroke.GetColour();
	m_Width = stroke.GetWidth();
	m_Cap   = stroke.GetCap();
	m_Join  = stroke.GetJoin();

	m_bInitialised = true;
	m_bEditable = true;
	m_bColor = true;
	m_bWidth = true;
	m_bCap = true;
	m_bJoin = true;
}

// Compares with <stroke> and sets all unequal attributes to invalid.
// Attributes that are still invalid will not compared again.
void a2dExtStroke::Mix(const a2dStroke& stroke)
{
	if( m_bStyle && (stroke.GetStyle() != m_Style) ) m_bStyle = false;

	if(!HasIndividualAttributes(stroke.GetStyle()))
		return;

	if(!m_bInitialised)
	{
		SetAttributes(stroke);
		return;
	}

	if( m_bColor && (stroke.GetColour() != m_Color) ) m_bColor = false;
	if( m_bWidth && (stroke.GetWidth() != m_Width) ) m_bWidth = false;
	if( m_bCap && (stroke.GetCap() != m_Cap) ) m_bCap = false;
	if( m_bJoin && (stroke.GetJoin() != m_Join) ) m_bJoin = false;
}

// Returns the result of setting all valid attributes to <stroke>.
a2dStroke a2dExtStroke::Get(const a2dStroke& stroke) const
{
	if(m_bStyle)
	{
	switch(m_Style)
	{
		case a2dSTROKE_LAYER:
		case a2dSTROKE_NULLSTROKE:
					return *a2dNullSTROKE;

		case a2dSTROKE_TRANSPARENT:
					return *a2dTRANSPARENT_STROKE;
	}
	}

	a2dStroke mixed = stroke;
	if(!HasIndividualAttributes(stroke.GetStyle()))
	{
		if(!m_bStyle)
			return stroke;

		mixed = m_Default;
	}

	if( m_bColor && (mixed.GetColour() != m_Color) )
		mixed.SetColour(m_Color);
	if( m_bWidth && (mixed.GetWidth() != m_Width) )
		mixed.SetWidth(m_Width);
	if( m_bCap && (mixed.GetCap() != m_Cap) )
		mixed.SetCap(m_Cap);
	if( m_bJoin && (mixed.GetJoin() != m_Join) )
		mixed.SetJoin(m_Join);
	if( m_bStyle && (mixed.GetStyle() != m_Style) )
		mixed.SetStyle(m_Style);
	return mixed;
}

void a2dExtStroke::SetStyle(a2dStrokeStyle style)
{
	m_Style = style;
	m_bStyle = true;
	m_bEditable = HasIndividualAttributes(m_Style);
}

void a2dExtStroke::SetColor(const wxColour& color)
{
	m_Color = color;
	m_bColor = true;
}

void a2dExtStroke::SetWidth(const float& width)
{
	m_Width = width;
	m_bWidth = true;
}

void a2dExtStroke::SetCap(wxPenCap cap)
{
	m_Cap = cap;
	m_bCap = true;
}

void a2dExtStroke::SetJoin(wxPenJoin join)
{
	m_Join = join;
	m_bJoin = true;
}


//===========================================================================
// a2dExtFill
//===========================================================================

a2dExtFill::a2dExtFill()
	: m_Fill(), m_bFill(false)
{
}

a2dExtFill::a2dExtFill(const a2dFill& fill)
	: m_Fill(), m_bFill(false)
{
	Set(fill);
}

// Initialises with <fill>.
void a2dExtFill::Set(const a2dFill& fill)
{
	if( fill.GetStyle() == a2dFILL_SOLID || fill.GetStyle() == a2dFILL_TRANSPARENT || 
		fill.GetStyle() == a2dFILL_LAYER || fill.GetStyle() == a2dFILL_NULLFILL )
	{
		m_Fill = fill;
		m_bFill = true;
	}
	else m_bFill = false;
}

// Compares with <fill> and sets to invalid if unequal.
void a2dExtFill::Mix(const a2dFill& fill)
{
	if(!m_bFill) return; // already invalid, don't compare
	switch(m_Fill.GetStyle())
	{
		case a2dFILL_TRANSPARENT:
				if(fill.GetStyle() != a2dFILL_TRANSPARENT) m_bFill = false;
				break;

		case a2dFILL_SOLID:
				if(fill.GetStyle() != a2dFILL_SOLID) m_bFill = false;
				else if(fill.GetColour() != m_Fill.GetColour()) m_bFill = false;
				break;

		default:
				m_bFill = false;
	}
}

// Returns the a2dFill member object if valid, or <fill> instead.
a2dFill a2dExtFill::Get(const a2dFill& fill) const
{
	if(m_bFill) return m_Fill;
	return fill;
/*
	// Returns the result of setting all valid attributes to <fill>.
	if(!m_bFill) return fill;

	a2dFill mixed = fill;
	switch(m_Fill.GetStyle())
	{
		case a2dFILL_TRANSPARENT:
				mixed.SetStyle(a2dFILL_TRANSPARENT);
				break;

		case a2dFILL_SOLID:
				mixed.SetStyle(a2dFILL_SOLID);
				mixed.SetColour(m_Fill.GetColour());
				break;
	}
	return mixed;
*/
}


