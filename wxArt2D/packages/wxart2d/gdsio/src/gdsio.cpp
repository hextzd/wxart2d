/*! \file gdsio/src/gdsio.cpp
    \brief GdsII parser
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: gdsio.cpp,v 1.135 2009/06/07 13:28:08 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <cstring>

#ifdef new
#undef new
#endif

#include "a2dprivate.h"

#if wxART2D_USE_GDSIO

#include "wx/docview/doccom.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/editor/candoc.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/recur.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/cameleon.h"
#include "wx/gdsio/gdsio.h"
#include "wx/gdsio/gdserr.h"

#include <wx/datetime.h>
#include <limits.h>

const a2dError a2dError_GDSIIparse( wxT( "a2dError_GDSIIparse" ), wxT( "gdsii parsing error" ) );

/*
GDS-II:
<stream format> ::= HEADER BGNLIB LIBNAME [REFLIBS]
                                [FONTS] [ATTRTABLE] [GENERATION]
                                [<FormatType>] UNI TS {<Structure>}*
                                ENDLIB
<FormatType>        ::= FORMAT | FORMAT {MASK}+ ENDMASK
<Structure>         ::= BGNSTR STRNAME [STRCLASS]
                                {<element>}* ENDSTR
<element>           ::= {<boundary> | <path> | <SREF> |
                                <AREF> | <text> | <node> | <bbox>}
                                {<property>}* ENDEL
<boundary>          ::= BOUNDARY [ELFLAGS] [PLEX] LAYER
                                DATATYPE XY
<path>              ::= PATH [ELFLAGS] [PLEX] LAYER
                                DATATYPE [PATHTYPE] [WIDTH]
                                [BGNEXTN] [ENDEXTN] XY
<SREF>              ::= SREF [ELFLAGS] [PLEX] SNAME
                                [<strans>] XY
<AREF>              ::= AREF [ELFLAGS] [PLEX] SNAME
                                [<strans>] COLROW XY
<text>              ::= TEXT [ELFLAGS] [PLEX] LAYER
                                <textbody>
<node>              ::= NODE [ELFLAGS] [PLEX] LAYER
                                NODETYPE XY
<box>                   ::= BOX [ELFLAGS] [PLEX] LAYER
                                BOXTYPE XY
<textbody>          ::= TEXTTYPE [PRESENTATION] [PATHTYPE]
                                [WIDTH] [<strans>] XY STRING
<property>          ::= PROPATTR PROPVALUE
*/


/*
Purpose Used by GDS driver for conversions and read en write
        GDSII elements
*/

// ------------- input functions -----------------

#define MAX_G_BYTES         8       // maximum bytes a value can use

//! array of bytes, which  holds a string in GDS-II
struct _ARRAY
{
    unsigned char array[MAX_G_BYTES];
};
struct _ARRAYB
{
    char array[MAX_G_BYTES];
};

//! for reading bytes from a binary GDS-II file, and convert them to internal types.
struct a_union_value
{
    //! constructor reset it to 0's
    a_union_value() { memset( array.array, 0, MAX_G_BYTES ); }

    union
    {
        TWO_G_BYTE_SIGNED_INTEGER  two_byte_signed_integer;
        FOUR_G_BYTE_SIGNED_INTEGER four_byte_signed_integer;
        FOUR_G_BYTE_REAL           four_byte_real;
        EIGHT_G_BYTE_REAL          eight_byte_real;
        _ARRAY                     array;
        _ARRAYB                    arrayc;
    };
};

// 2 bytes UNsigned integer
unsigned int a2dIOHandlerGDSIn::ReadUnsignedInt()
{
    unsigned char swap;
    a_union_value a_value;
    a_value.eight_byte_real = 0;

    a2dIOHandlerStrIn::Read( a_value.arrayc.array, 2 );

#if wxBYTE_ORDER == wxBIG_ENDIAN
#else
    swap = a_value.array.array[1];
    a_value.array.array[1] = a_value.array.array[0];
    a_value.array.array[0] = swap;
#endif
    return a_value.two_byte_signed_integer;
}


// 2 bytes signed integer
int a2dIOHandlerGDSIn::ReadInt()
{
    unsigned char swap;
    a_union_value a_value;
    a_value.eight_byte_real = 0;

    a2dIOHandlerStrIn::Read( a_value.arrayc.array, 2 );

#if wxBYTE_ORDER == wxBIG_ENDIAN
#else
    swap = a_value.array.array[1];
    a_value.array.array[1] = a_value.array.array[0];
    a_value.array.array[0] = swap;
#endif
    return a_value.two_byte_signed_integer;
}

// 4 byte signed integer
long a2dIOHandlerGDSIn::ReadLong()
{
    a_union_value a_value;
    char array[4];

    // reads a GdsII variable and convers to a dos-variable
    // the variable consist of 4 bits, numbered 0 1 2 3 , m_streami
    // will be converted to the following sequence: 3 2 1 0

    a2dIOHandlerStrIn::Read( array, 4 );

#if wxBYTE_ORDER == wxBIG_ENDIAN
    a_value.array.array = array;
#else
    {
        a_value.array.array[3] = array[0];
        a_value.array.array[2] = array[1];
        a_value.array.array[1] = array[2];
        a_value.array.array[0] = array[3];
    }
#endif

    return a_value.four_byte_signed_integer;
}

// 4 byte byte real
float a2dIOHandlerGDSIn::ReadFloat()
{
    a_union_value a_value;

    // reads a GdsII variable and convers to a dos-variable
    // the variable consist of 4 bits, numbered 0 1 2 3 , m_streami
    // will be converted to the following sequence: 3 2 1 0
#if wxBYTE_ORDER == wxBIG_ENDIAN
    for( short int i = 0; i <= 3; i++ )
#else
    for( short int i = 3; i >= 0; i-- )
#endif
        a_value.array.array[i] = GetC();

    // NOTE: in the original written code in C, a four byte real isn't convert
    //       like a eight byte real. Cause a four byte real isn't use in the
    //       gdsii, there's no problem. maybe in future versions, a four byte
    //       real must also converted. (the function gds2double must be changed
    //       for using 4 bytes instead of 8).

    return a_value.four_byte_real;
}


// 8 byte real
double a2dIOHandlerGDSIn::ReadDouble()
{
    a_union_value a_value;
    short int   i;

    // reads a GdsII variable and convers to a dos-variable
    // Reverse ordering will happen in the function gds2double

    for( i = 0; i < 8; i++ )
        a_value.array.array[i] = GetC();

    // convert gdsii-real into a machine double
    gds2double( a_value.array.array );

    return a_value.eight_byte_real;
}

/*! PC double union
To convert GDSII doubles to machine doubles :
void gds_double( fb)

To convert machine doubles to GDSII doubles :
void double_gds( fb)


GDSII floating point representation (8-byte REAL)

SEEEEEEE MMMMMMMM MMMMMMMM MMMMMMMM
MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

Sign         = S=1 negatif S=0 positif
Expo         = (E-64)^16
Mantisse     = (1/16 <= M < 1)
Double value = (-1)^S * Expo * Mantisse
Zero         = all bits of M zero

unix (Apollo) or wxBIG_ENDIAN (8-byte REAL)

SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM
MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

Sign         = S=1 negatif S=0 positif
Expo         = (E-1023)^2
Mantisse     = (1.M)binary
Double value = (-1)^S * Expo * Mantisse
Zero         = E=0 and M=0
+-Infinit    = E=2047 and M=0

notice:
    a wxUint32 in turbo C =  in memory for instance like this
    m=40 m+1=99 m+2=94 m+3=93

    then    (long) & 0xFF000000 gives the result
    m=40 m+1=99 m+2=94 m+3=00

    in most other machine this is
    m=00 m+1=99 m+2=94 m+3=93

    all of this is due to that the bytes of types are in reverse order
    in memory.
*/
struct bldun
{
    bldun() { memset( by, 0, sizeof( wxFloat64 ) ); }
    union
    {
        wxByte by[ sizeof( wxFloat64 ) / sizeof( wxByte ) ];
        wxUint32 fl[ sizeof( wxFloat64 ) / sizeof( wxUint32 ) ];
        wxFloat64 machdb;
    };
};


//! convert a gds double to a machine double
//! input
//!     arguments  : unsigned char fb[]  8byte array with gds double
//! output
//!     arguments  : unsigned char fb[]  8byte array with machine double
void a2dIOHandlerGDSIn::gds2double( unsigned char fb[] )
{
    bldun bld;
    int j;
    wxUint32 carry, e, mantissa[2], s;
#   if defined(ultrix) || wxBYTE_ORDER == wxLITTLE_ENDIAN
    wxUint32 swap;
#   endif

#if wxBYTE_ORDER == wxLITTLE_ENDIAN
    // for turbo c real,integer,doubles are saved in memory
    // in reverse order, so first reverse bytes
    for ( j = 0; j < 8; j++ )
        bld.by[j] = fb[7 - j];
#else
    for ( j = 0; j < 8; j++ )
        bld.by[j] = fb[j];
#endif


#ifdef vax
    if( bld.fl[0] == 0 && bld.fl[1] == 0 ) return;

    // mask highest bit (bit 32) and shift to bit1
    s = ( bld.fl[1] & 0x80000000L ) >> 31;
    // same for exponent bits
    e = ( bld.fl[1] & 0x7F000000L ) >> 24;

    // mask mantisse fo GDS double
    mantissa[0] = bld.fl[1] & 0x00FFFFFFL;
    mantissa[1] = bld.fl[0];

    // 2^(4*(E-64))=GDStrue exp  +128 for VAX
    e = 4 * ( e - 64 ) + 128;
    if ( mantissa[0] == 0 && mantissa[1] == 0 && e != 0 )
    {
        // the number = zero
        bld.fl[1] = 0;
        // result back in byte array
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[j];
        return;
    }

    // change mant. to 0.1mant
    while( mantissa[0] < 0x00800000L )
    {
        // times 2
        carry = mantissa[1] >> 31;
        // carry to next four bytes
        mantissa[0] = mantissa[0] << 1 | carry;
        // bit not available become zero
        mantissa[1] = mantissa[1] << 1;

        // without loss in accurancy
        // so lower exponent by one
        e--;
    }

    // mantisse is now 0.1mant
    //  - mask highest redundant bit of mantisse
    //  - swap word (2 bytes) positions
    //  - put sign and exponent on right place
    bld.fl[0] = ( mantissa[0] & 0x007F0000L ) >> 16 |
                ( mantissa[0] & 0x0000FFFFL ) << 16 |
                s << 15 | e << 7;

    // swap word positions
    bld.fl[1] = mantissa[1] << 16 | mantissa[1] >> 16;
    // result back in byte array
    for ( j = 0; j < 8; j++ )
        fb[j] = bld.by[j];

#else
    // convert GDSII double to _G_UNIX (Apollo) double
    //if all is zero, it is true zero value
    if ( bld.fl[0] == 0 && bld.fl[1] == 0 )
        return;

#if wxBYTE_ORDER == wxLITTLE_ENDIAN  || defined(ultrix)
    // swap the longs
    swap = bld.fl[0] ; bld.fl[0] = bld.fl[1] ; bld.fl[1] = swap ;
#endif
    // mask sign bit and shift to bit 1
    s = ( bld.fl[0] & 0x80000000L ) >> 31;
    // same for exponent bits
    e = ( bld.fl[0] & 0x7F000000L ) >> 24;

    // mask mantisse bits
    mantissa[0] = bld.fl[0] & 0x00FFFFFFL;
    mantissa[1] = bld.fl[1];


    /* THE NUMBER IS WRONG BUT IT DOES NOT HARM ME, SO I OUTCOMENTED IT
       WE just shift what is in the mantisse some more until
       we reach one that is set.
        // if < 1/16 is not a right number
        if (mantissa[0] < 0x00100000L)
        {
            bld.fl[0] = 0x80000000L;
            bld.fl[1] = 0x00000000L;

#if wxBYTE_ORDER == wxBIG_ENDIAN
#       if defined(ultrix)
            swap = bld.fl[0];
            bld.fl[0] = bld.fl[1];
            bld.fl[1] = swap;
#       endif
            // result back in byte array
            for (j = 0; j < 8; j++)
                fb[j]=bld.by[j];
            return;
        }
    */

    // 2^(4*(e-64))=GDS  +1023=for unix
    e = 4 * ( e - 64 ) + 1023;

    if ( mantissa[0] == 0 && mantissa[1] == 0 && e != 0 )
    {
        //this is a bad number, the mantisse is total zero
        //so  i decide to return true zero as result
        bld.fl[0] = 0x00000000L;
        bld.fl[1] = 0x00000000L;

        // result back in byte array
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[j];
        return;
    }

    // change mant to 1.mant
    while ( mantissa[0] < 0x01000000L )
    {
        // times two
        carry = mantissa[1] >> 31;
        mantissa[0] = ( mantissa[0] << 1 ) | carry;
        mantissa[1] = mantissa[1] << 1;

        // so minus one
        e--;
    }

    // out of range
    if( e < 1 || e > 2047 )
    {
        bld.fl[0] = 0x80000000L;
        bld.fl[1] = 0x00000000L;

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
        swap = bld.fl[0];
        bld.fl[0] = bld.fl[1];
        bld.fl[1] = swap;
#       endif

        // result back in byte array
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[j];
        return;
    }

    // now skip highest redundant bit of mantisseand shift mantisse
    // to the right place in the longs
    bld.fl[0] = ( ( mantissa[0] & 0x00FFFFFFL ) >> 4 ) | ( s << 31 ) | ( e << 20 );
    bld.fl[1] = ( mantissa[0] << 28 ) | ( mantissa[1] >> 4 );

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
    swap = bld.fl[0];
    bld.fl[0] = bld.fl[1];
    bld.fl[1] = swap;
#   endif
    // result back in byte array
    for ( j = 0; j < 8; j++ )
        fb[j] = bld.by[j];
#endif // vax
}


//! convert a machine double to gds double
//! usage: void   double2gds( fb )
//! input
//!   arguments  : unsigned char fb[]  8byte array with machine double
//! output
//!   arguments  : unsigned char fb[]  8byte array with gds double
/*! Important note:
        The number of mantissa bit in VAX representation is
        one less than the number of mantissa bits in GDSII
        representation. This routine is not able to generate the
        last 2 bits of the GDSII representation exactly.
        There does not exist a general solution for this problem!!
        In order to overcome the single problem that we are aware of,
        the UNITs record that is sometimes compared to a UNITs record,
        the solution is hard coded in this routine.
        The last 8 bits of a number representing 1E-9 result in 53,
        but should produce 51 (See GDSII documentation of UNIT record
        and the given example of it).

        !! The same story holds for APOLLO and wxBIG_ENDIAN,
        but even worse. 1E-3 results in
        F0 for the last 8 bits instead of EF. Also corrected.
*/
void a2dIOHandlerGDSOut::double2gds( unsigned char fb[] )

{
    bldun bld;

    union
    {
        wxUint32    f[2];
        wxFloat64   check;
    } hard_check;
    int j;

#ifdef vax
    wxUint32 carry, e, f[2], s;

    // make copy into union
    for ( j = 0; j < 8; j++ )
        bld.by[j] = fb[j];

    hard_check.f[0] = bld.fl[0];
    hard_check.f[1] = bld.fl[1];

    // take care of the words places
    s = ( bld.fl[0] & 0x00008000L ) >> 15;
    // mask sign and exponent and shift to bit1
    e = ( bld.fl[0] & 0x00007F80L ) >> 7;

    if( s == 0 & e == 0 )
    {
        bld.fl[0] = bld.fl[1] = 0;
        // result back in byte array
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[j];
        return;
    }

    // make mant 0.1mant
    f[0] = ( bld.fl[0] & 0x0000007FL ) << 16 | bld.fl[0] >> 16 | 0x00800000L;
    // swap words
    f[1] = bld.fl[1] << 16 | bld.fl[1] >> 16;

    // true exponent
    e = e - 128;

    // new exponent 16^exp
    while ( ( e % 4 ) != 0 )
    {
        // divide by two
        carry = f[0] & 0x00000001L;
        f[0] = f[0] >> 1;
        f[1] = f[1] >> 1 | carry << 31;
        // so add 1
        e++;
    }

    // now change 2^expo to 16^exp for GDS
    e = e / 4 + 64;

    // put bits on right place
    bld.fl[1] = f[0] | e << 24 | s << 31;
    bld.fl[0] = f[1];

    // Hard coded exception begin!
    if ( hard_check.check == 1E-9 )
        bld.fl[0] = bld.fl[0] & 0XFFFFFF51L;
    // End of hard coded exception
    for ( j = 0; j < 8; j++ )
        bld.by[j] = fb[j];

#else
    //convert unix (Apollo,wxLITTLE_ENDIAN) double to GDSII double
    wxUint32 carry, f[2], s;
#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
    wxUint32 swap;
#   endif

    long e;

    for ( j = 0; j < 8; j++ )
        bld.by[j] = fb[j];

    // backup of original machine number
    hard_check.f[0] = bld.fl[0];
    hard_check.f[1] = bld.fl[1];

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
    swap = bld.fl[0];
    bld.fl[0] = bld.fl[1];
    bld.fl[1] = swap;
#   endif

    // mask sign bit
    s = ( bld.fl[0] & 0x80000000L ) >> 31;
    // mask exponent
    e = ( bld.fl[0] & 0x7FF00000L ) >> 20;

    // number to small
    if( ( s == 0 ) & ( e == 0 ) )
    {
        // assume it is zero
        bld.fl[0] = bld.fl[1] = 0;

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
        // for turbo c real, integer,d oubles are saved in memory
        // in reverse order, so first reverse bytes
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[7 - j];
#   else
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[j];
#   endif

        return;
    }

    // 52 bits mantisse mask
    f[0] = ( bld.fl[0] & 0x000FFFFFL );
    f[1] = bld.fl[1];

    // add 2 ^ 0 bit that is not yet represented because it was redundant
    f[0] |= 0x00100000L;

    //
    //   -- Now bitposition for 2 ^ -1 is bit 12. It should be 8.
    //   -- Instead of shifting 4 bits to the left and than normalize the
    //   -- exponent by shifting 1 to 4 times to the right, there is only
    //   -- one shift of 0 to 3 time to the left. This is reflected in the
    //   -- exponent that will be 1 greater in the end
    //

    // true exponent
    e = e - 1023;

    // exponent should be 2^(4*newexp)
    while( ( e % 4 ) != 0 )
    {
        // divide by two
        carry = f[1] >> 31;
        f[0] <<= 1;
        f[1] <<= 1;
        f[0] |= carry;
        // so minus one
        e--;
    }

    // because of trick add one more
    e = e / 4 + 65;

    // number to big are small
    if( e < 0 || e > 127 )
    {
        bld.fl[0] = 0x80000000L;
        bld.fl[1] = 0x00000000L;

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
        swap = bld.fl[0];
        bld.fl[0] = bld.fl[1];
        bld.fl[1] = swap;
#       endif

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
        // for turbo c real, integer, doubles are saved in memory
        // in reverse order, so first reverse bytes
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[7 - j];
#       else
        for ( j = 0; j < 8; j++ )
            fb[j] = bld.by[j];
#       endif

        return;
    }

    // put bits into right place for GDS
    bld.fl[0] = f[0] | ( e << 24 ) | ( s << 31 );
    bld.fl[1] = f[1];

    // Hard coded exception begin!
    if( hard_check.check == 1E-3 )
        bld.fl[1] = ( bld.fl[1] & 0XFFFFFF00L ) | 0X000000EFL;
    if( hard_check.check == 1E-9 )
        bld.fl[1] = ( bld.fl[1] & 0XFFFFFF00L ) | 0X00000051L;
    // End of hard coded exception

#if wxBYTE_ORDER == wxLITTLE_ENDIAN || defined(ultrix)
    swap = bld.fl[0];
    bld.fl[0] = bld.fl[1];
    bld.fl[1] = swap;
#   endif
#if wxBYTE_ORDER == wxLITTLE_ENDIAN
    // for turbo c real,integer,doubles are saved in memory
    // in reverse order, so first reverse bytes
    for ( j = 0; j < 8; j++ )
        fb[j] = bld.by[7 - j];
#   else
    for ( j = 0; j < 8; j++ )
        fb[j] = bld.by[j];
#   endif

#endif
}

//! ckecking of it is near enough to a integer number
/*!
    used for deciding if rotation is there in a matrix.
*/
double Check_int( double getal )

{
    // check if the number is very close to an integer
    if ( ( ceil( getal ) - getal ) < 0.0001 )
        return ceil( getal );

    else if ( ( getal - floor( getal ) ) < 0.0001 )

        return floor( getal );

    return getal;

}

//----------------------------------------------------------------------------------/

// Strans

//----------------------------------------------------------------------------------/


Strans::Strans( double a_angle, double a_scale,

                bool a_abs_angle, bool a_abs_scale, bool a_reflection )
{
    m_stransflags.total = 0;

    SetAbsAngle( a_abs_angle );
    SetAbsScale( a_abs_scale );
    SetReflection( a_reflection );
    SetAngle( a_angle );
    SetScale( a_scale );
}

Strans& Strans::operator=( Strans& a_strans )
{
    m_stransflags.total = a_strans.m_stransflags.total;

    SetAbsAngle( bool( a_strans.m_stransflags.bits.abs_angle ) );
    SetAbsScale( bool( a_strans.m_stransflags.bits.abs_scale ) );
    SetReflection( bool( a_strans.m_stransflags.bits.reflection ) );
    SetAngle( a_strans.m_angle );
    SetScale( a_strans.m_scale );

    return *this;
}


bool Strans::GetStrans()
{
    return m_strans_need;
}

void Strans::SetAngle( double a_angle )
{
    m_angle = a_angle;
    if ( m_scale != 0 )
        m_strans_need = true;
}


void Strans::SetScale( double a_scale )
{
    m_scale = a_scale;
    if ( m_scale != 1 )
        m_strans_need = true;
}


void Strans::SetAbsAngle( bool b )
{
    m_stransflags.bits.abs_angle = b;
}


void Strans::SetAbsScale( bool b )
{
    m_stransflags.bits.abs_scale = b;
}


void Strans::SetReflection( bool b )
{
    m_stransflags.bits.reflection = b;
}



double Strans::GetAngle()
{
    return m_angle;
}


double Strans::GetScale()
{
    return m_scale;
}


bool Strans::GetAbsAngle()
{
    return ( bool ) m_stransflags.bits.abs_angle;
}


bool Strans::GetAbsScale()
{
    return ( bool )m_stransflags.bits.abs_scale;
}


bool Strans::GetReflection()
{
    return ( bool )m_stransflags.bits.reflection;
}


void Strans::MakeStrans( const a2dAffineMatrix& matrix )
{
    double scale_factor = matrix.GetValue( 0, 0 );
    double rot_angle;

    double temp1 = matrix.GetValue( 0, 0 ); // voor het berekenen van de hoek

    double temp2 = matrix.GetValue( 0, 1 ); //
    bool check_1 = ( matrix.GetValue( 0, 0 ) < 0 ); // om te kijken of er horizontaal gespiegeld moet worden

    bool check_2 = ( matrix.GetValue( 1, 1 ) < 0 );     //                    verticaal

    m_strans_need = false;

    if ( check_1 )
    {
        // Reflection horizontal (in y)
        SetReflection( true );
        rot_angle = 180;
        SetAngle( rot_angle );
        m_strans_need = true;
    }

    if ( check_2 )
    {
        // Reflection vertical (in x)
        SetReflection( !GetReflection() );
        m_strans_need = true;
    }


    if ( matrix.GetValue( 0, 1 ) != 0 )
    {
        // Rotation
        rot_angle = Check_int( atan2( temp2, temp1 ) * 180 / wxPI );                                                // kijk of dit een integer is (met bepaalde toegestane afwijking)

        SetAngle( rot_angle );
        if ( rot_angle != 90 && rot_angle != -90 )
            scale_factor = matrix.GetValue( 0, 0 ) / cos( ( rot_angle / 180 ) * wxPI );
        else
            scale_factor = matrix.GetValue( 1, 0 ) / sin( ( rot_angle / 180 ) * wxPI ); // er kan nl. niet door 0 gedeeld worden !
        m_strans_need = true;
    }



    // scale
    scale_factor = Check_int( scale_factor );

    if ( scale_factor < 0 ) // schaling altijd positief
        scale_factor = -scale_factor;

    if ( scale_factor != 1 ) // niet schalen bij 1
    {
        SetScale( scale_factor );
        m_strans_need = true;
    }

}

//----------------------------------------------------------------------------------/
//    a2dIOHandlerGDSIn
//----------------------------------------------------------------------------------/

a2dIOHandlerGDSIn::a2dIOHandlerGDSIn()
{
    m_fontType = a2dFONT_STROKED;
    m_docClassInfo = &a2dCanvasDocument::ms_classInfo;
    m_refMaxx = 10; 
    m_dx = m_dy = 0;
    m_asCameleons = false;
    m_asMultiroot = false;
    m_layers = NULL;
    m_gridx = 0; 
    m_gridy = 0;
}

a2dIOHandlerGDSIn::~a2dIOHandlerGDSIn()
{
}

void a2dIOHandlerGDSIn::InitializeLoad(  )
{
    m_gridx = 0; 
    m_gridy = 0;
    a2dIOHandlerStrIn::InitializeLoad();
    if ( m_doc )
        a2dCanvasGlobals->GetHabitat()->SetAberPolyToArc( double( a2dCanvasGlobals->GetHabitat()->GetAberPolyToArc() ) / m_doc->GetUnitsScale() );
}

void a2dIOHandlerGDSIn::ResetLoad(  )
{
    a2dIOHandlerStrIn::ResetLoad();
}

bool a2dIOHandlerGDSIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{
    if ( obj && !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;

    if ( docClassInfo && m_docClassInfo && !docClassInfo->IsKindOf( m_docClassInfo ) )
        return false;

    m_streami = &stream;
    m_doc = 0;

    SeekI( 0 );

    m_recordsize      = 0;
    m_recordtype      = 0;
    m_datatype        = 0;
    m_back            = false;

    if ( !ReadHeader( GDS_HEADER ) )
    {
        SeekI( 0 );
        return false;
    }
    SeekI( 0 );
    return true;
}

bool a2dIOHandlerGDSIn::LinkReferences()
{
    bool res = a2dIOHandler::LinkReferences();
    /*
    {
        //search in the root object for the childs that have the bin flag set,
        //which means they are referenced, and can be deleted.
        //All othere are top structures.
        a2dCanvasObject* root = m_doc->GetDrawing()->GetRootObject();
        a2dCanvasObjectList::iterator rootchild = root->GetChildObjectList()->begin();
        while ( rootchild != root->GetChildObjectList()->end() )
        {
            a2dCanvasObjectList::iterator delnode = rootchild;
            rootchild++;
            if ( ( *delnode )->GetCheck() )
            {
                root->GetChildObjectList()->erase( delnode );
            }
        }
    }
    */
    return res;
}

bool a2dIOHandlerGDSIn::Load( a2dDocumentInputStream& stream, wxObject* doc )
{
    m_streami = &stream;
    m_doc = wxStaticCast( doc, a2dCanvasDocument );
    InitializeLoad();

    wxString path;
    path = m_doc->GetFilename().GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR  ) + m_doc->GetFilename().GetName() + wxT( ".cvg" );
    if ( wxFileExists( path ) )
        m_doc->LoadLayers( path );

    m_layers = m_doc->GetLayerSetup();
    m_doc->GetDrawing()->SetLayerSetup( m_layers );

    m_recordsize      = 0;
    m_recordtype      = 0;
    m_datatype        = 0;
    m_back            = false;


    for ( a2dLayerMapNr::iterator it= m_layers->GetLayerIndex().begin(); it != m_layers->GetLayerIndex().end(); ++it)
    {
        a2dLayerInfo* layerobj = it->second;
        wxUint16 i = it->first;
        m_mapping[ layerobj->GetInMapping() ] = layerobj;
    }
    try
    {
        ReadGdsII();
    }
    catch ( GDS_Error& _error )
    {
        ResetLoad();
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "error in GDSII file \n %s \n" ), _error.GetErrorMessage().c_str() );
        return false;
    }

    ResetLoad();
    return true;
}

// GdsII --------------------------------------------------------------------
void a2dIOHandlerGDSIn::ReadGdsII()
{
    //The a2dObject::m_check is used for removing structure from the rootobject after resolving references.
    //when a new document is filled this would not be needed, since all m_check flags are default set false.
    //But maybe for the future import it is important to do.

    a2dWalker_SetCheck setp( false );
    setp.Start( m_doc->GetDrawing()->GetRootObject() );

    a2dCameleon::SetCameleonRoot( m_doc->GetDrawing()->GetRootObject() );

    // Grammar: HEADER BGNLIB LIBNAME [REFLIBS] [FONTS]
    //              [ATTRTABLE] [GENERATION] [<FormatType>]
    //              UNITS {<Structure>}* ENDLIB
    //
    if ( !ReadHeader( GDS_HEADER ) )
        throw GDS_Error( _( "GDS_HEADER record missing (in GDSII)" ), _( "Error" ) );

    wxString s;
    s.Printf("%d", ReadInt());
    m_doc->SetVersion( s );

    if ( !ReadBgnlib() )
        throw GDS_Error( _( "BGNLIB record missing (in GDSII)" ) );

    if ( !ReadLibrarySettings() )
        throw GDS_Error( _( "LIBNAME missing (in GDSII)" ) );

    if ( m_asCameleons )
    {
        while ( ReadCameleonStructure( m_doc->GetDrawing()->GetRootObject() ) )
        {
        }
    }
    else
    {
        while ( ReadStructure( m_doc->GetDrawing()->GetRootObject() ) )
        {
        }
    }

    if ( !ReadHeader( GDS_ENDLIB ) )
        throw GDS_Error( _( "GDS_ENDLIB record missing (in GDSII)" ), _( "Error" ) );

    //references to other structure are known
    //by name now. Set the pointers in those references
    //to the right structures
    LinkReferences(); //GDS links on Name

    if ( m_asCameleons )
    {
        m_dx = m_dy = 0;

        m_doc->GetDrawing()->SetDrawingId( a2dDrawingId_cameleonrefs() );
        if ( m_doc->GetDrawing()->GetRootObject()->GetChildObjectsCount() == 0 )
                throw GDS_Error( wxT( "no structures found" ) );

        m_doc->GetDrawing()->SetRootRecursive();

        a2dCameleon* topcam = NULL;
        a2dCanvasObjectList::iterator rootchild = m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->begin();
        while ( rootchild != m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->end() )
        {
            a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
            if ( ref )
            {
                a2dLayout* layout = ref->GetAppearance<a2dDiagram>();
                layout->GetDrawing()->SetRootRecursive();
                if ( ! layout->GetCheck() )
                {
                    topcam = ref;
                    m_doc->SetTopCameleon( topcam );
                    m_doc->SetStartObject( layout->GetDrawing()->GetRootObject() );
                    m_doc->SetShowObject( layout->GetDrawing()->GetRootObject() );
                }
            }
            rootchild++;
        }

        /*
            a2dCameleon* cam;
            if ( topcam ) 
            {
                cam = topcam;
                a2dLayout* layout = cam->GetAppearance<a2dDiagram>();
                a2dSmrtPtr<class a2dDrawing > Structure = layout->GetDrawing();
                m_doc->SetRootObject( Structure );
            }
            else
                throw GDS_Error( wxT( "no non referenced structure found" ) );
        */
    }
    else
    {
        if ( m_asMultiroot )
        {

            //search in the root object for the childs that have the bin flag set,
            //which means they are referenced, and can be deleted.
            //All othere are top structures.
            a2dCanvasObject* root = m_doc->GetDrawing()->GetRootObject();
            a2dCanvasObject* top = NULL;
            a2dCanvasObjectList::iterator rootchild = root->GetChildObjectList()->begin();
            while ( rootchild != root->GetChildObjectList()->end() )
            {
                a2dCanvasObjectList::iterator node = rootchild;
                rootchild++;
                a2dNameReference* ref = wxDynamicCast( node->Get(), a2dNameReference );
                if ( ref && !ref->GetCanvasObject()->GetCheck() )
                {
                    top = *node;
                    break;
                }
            }

            m_doc->SetMultiRoot();
            if ( top )
            {
                a2dNameReference* ref = wxDynamicCast( top, a2dNameReference );
                m_doc->SetStartObject( ref->GetCanvasObject() );
            }
            else
                m_doc->SetStartObject( m_doc->GetDrawing()->GetRootObject() );
        }
        else
        {
            if ( m_doc->GetDrawing()->GetRootObject()->GetChildObjectsCount() != 1 )
            {
                m_doc->SetMultiRoot();
                m_doc->SetStartObject( m_doc->GetDrawing()->GetRootObject() );
            }
            else
            {
                m_doc->SetMultiRoot( false );
                m_doc->GetDrawing()->SetRootRecursive();
                m_doc->SetStartObject( m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->front() );
            }
        }
        m_doc->GetDrawing()->SetUnits( m_doc->GetUnits() );
        m_doc->GetDrawing()->SetUnitsAccuracy( m_doc->GetUnitsAccuracy() );
        m_doc->GetDrawing()->SetUnitsScale( m_doc->GetUnitsScale() );

    }
}

// Bgnlib -------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadBgnlib()
{
    if ( !ReadHeader( GDS_BGNLIB ) )
        return( false );

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    year = ReadInt();
    month = ReadInt() - 1;
    day = ReadInt();
    hour = ReadInt();
    minute = ReadInt();
    second = ReadInt();

    m_doc->GetModificationTime().Set( day, ( wxDateTime::Month )month, year, hour, minute, second );

    year = ReadInt();
    month = ReadInt() - 1;
    day = ReadInt();
    hour = ReadInt();
    minute = ReadInt();
    second = ReadInt();

    m_doc->GetAccessTime().Set( day, ( wxDateTime::Month )month, year, hour, minute, second );

    return( true );
}

// Library ------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadLibrarySettings()
{
    if ( !ReadHeader( GDS_LIBNAME ) )
        throw GDS_Error( _( "GDS_LIBNAME record missing (in GDSII)" ), _( "Error" ) );

    wxString libname;
    ReadString( libname );

    m_doc->SetLibraryName( libname );

    if ( ReadHeader( GDS_REFLIBS ) )
        SkipData();

    if ( ReadHeader( GDS_FONTS ) )
        SkipData();

    if ( ReadHeader( GDS_ATTRTABLE ) )
        SkipData();

    if ( ReadHeader( GDS_GENERATIONS ) )
        SkipData();

    if ( ReadHeader( GDS_FORMAT ) )
        SkipData();

    if ( !ReadHeader( GDS_UNITS ) )
        throw GDS_Error( _( "UNITS missing (in GDSII)" ) );

    m_userunits_out = ReadDouble();
    m_doc->SetUnitsAccuracy( m_userunits_out );
    double value1;
    value1 = ReadDouble();
    double metersScale = value1 / m_userunits_out;
    m_doc->SetUnitsScale( metersScale );

    a2dDoMu unit = a2dDoMu( 1, 1 );
    m_doc->SetUnits( unit.GetMultiplierString() );
    unit = a2dDoMu( 1, metersScale );
    m_doc->SetUnits( unit.GetMultiplierString() );

    return true;
}


bool a2dIOHandlerGDSIn::ReadHeader( int type )
{
    if ( m_back )
    {
        if ( m_recordtype == type )
        {
            m_back = false;
            return true;
        }
    }
    else
    {
        if ( Eof() )
            throw GDS_Error( _( "Unexpected end of GDSII-file!" ), _( "Driver Error" ) );

        m_recordsize = ReadUnsignedInt();
        m_recordtype = GetC();
        m_datatype = GetC();

        m_recordsize -= ( TWO_G_BYTE_UNSIGNED_INTEGER ) GDS_HEADERSIZE;

        if ( m_recordtype == type )
            return true;
        if ( m_recordtype > 56 )
        {
//          throw GDS_Error("Unknown Record Type! Record is not part of the GDS-II Stream Format v5.2","Driver Error", 9, 0);
            SkipData();
        }
        m_back = true;
    }
    return false;
}

void a2dIOHandlerGDSIn::SkipData()
{
    for ( int i = 0; i < m_recordsize; i++ )
    {
        GetC();
    }

    m_back = false;
    m_recordsize      = 0;
    m_recordtype      = 0;
    m_datatype        = 0;
}

void a2dIOHandlerGDSIn::ReadString( wxString& a_string )
{
    wxString header;
    char buf[512];
    int last_read = a2dIOHandlerStrIn::Read( buf, m_recordsize );

    if ( buf[m_recordsize - 1] == '\0' )
        a_string = wxString( buf, wxConvUTF8, m_recordsize - 1 );
    else
        a_string = wxString( buf, wxConvUTF8, m_recordsize );
}

// Read Element -------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadElement( a2dCanvasObject* parent )
{
    if  ( ReadBoundary( parent ) ||
            ReadPath( parent ) ||
            ReadStructureReference( parent ) ||
            ReadArrayReference( parent ) ||
            ReadText( parent ) ||
            ReadNode( parent ) ||
            ReadBox( parent ) )
    {
    }
    else
        return false;

    while ( ReadProperty() )
        ;

    if ( !ReadHeader( GDS_ENDEL ) )
        throw GDS_Error( _( "ENDEL missing (in Element)" ) );

    return true;
}


// Property -----------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadProperty()
{
    int i;
    if ( ReadHeader( GDS_PROPATTR ) )
    {
        // Read Property Attribute
        int propattr;
        propattr = ReadInt();

        if ( Eof() )
            throw GDS_Error( _( "Unexpected end of GDSII-file!" ), _( "GDSII Input parsing Error" ) );

        m_back = false;

        // Check (for) Property Value
        if ( !ReadHeader( GDS_PROPVALUE ) )
            throw GDS_Error( _( "PROPVALUE missing (in Property)" ) );

        // Read Property Value
        G_BYTE propval[126];
        for ( i = 0; i < m_recordsize; i++ )
        {
            if ( Eof() )
                throw GDS_Error( _( "Unexpected end of GDSII-file!" ), _( "GDSII Input parsing Error" ) );

            propval[i] = GetC();
        }
        m_back = false;
        //  End string
        if ( i <= 126 )
            propval[i] = 0;

        return true;    //  Found one!
    }
    return false;   // Didn't find one.
}


// Boundary -----------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadBoundary( a2dCanvasObject* parent )
{
    // Grammar: BOUNDARY [ELFLAGS] [PLEX] LAYER DATATYPE XY
    if ( !ReadHeader( GDS_BOUNDARY ) )
        return false;

    // Create a new Boundary
    //a2dSmrtPtr<a2dPolygon> _Boundary = new a2dPolygon();
    a2dSmrtPtr<a2dPolygonL> _Boundary = new a2dPolygonL();

    ReadElflags( _Boundary );

    if ( ReadHeader( GDS_PLEX ) )
        SkipData();

    if ( !ReadLayer( _Boundary ) )
        throw GDS_Error( _( "Boundary: LAYER missing (in GDS-file)" ) );

    if ( !ReadHeader( GDS_DATATYPE ) )
        throw GDS_Error( _( "Boundary: DATATYPE missing (in GDS-file)" ) );

    SetDataTypeProperty( _Boundary, ReadInt() );

    if ( m_mapping[ _Boundary->GetLayer() ]->GetRead() )
    {
        if ( !Read( _Boundary ) )
            throw GDS_Error( _( "Header: XY missing (in Boundary)" ) );

        parent->Append( _Boundary );
    }
    else
    {
        ReadHeader( GDS_XY );
        SkipData();
    }

    return true;
}

// Path ---------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadPath( a2dCanvasObject* parent )
{
    // Grammar: PATH [ELFLAGS] [PLEX] LAYER DATATYPE
    //              [PATHTYPE] [WIDTH] [BGNEXTN] [ENDEXTN] XY

    if ( !ReadHeader( GDS_PATH ) )
        return false;

    // Create a new Polyline
    a2dSmrtPtr<a2dPolylineL> path = new a2dPolylineL();

    ReadElflags( path );

    if ( ReadHeader( GDS_PLEX ) )
        SkipData();

    if ( !ReadLayer( path ) )
        throw GDS_Error( _( "record LAYER missing (in Path)" ) );

    if ( !ReadHeader( GDS_DATATYPE ) )
        throw GDS_Error( _( "record DATATYPE missing (in Path)" ) );

    SetDataTypeProperty( path, ReadInt() );

    if ( ReadHeader( GDS_PATHTYPE ) )
        path->SetPathType( ( a2dPATH_END_TYPE ) ReadInt() );

    if ( ReadHeader( GDS_WIDTH ) )
        path->SetContourWidth( ReadLong() * m_userunits_out );

    if ( ReadHeader( GDS_BGNEXTN ) )
        SkipData();

    if ( ReadHeader( GDS_ENDEXTN ) )
        SkipData();

    if ( m_mapping[ path->GetLayer() ]->GetRead() )
    {
        if ( !Read( path->GetSegments() ) )
            throw GDS_Error( _( "Header: XY missing (in Path)" ) );
        parent->Append( path );
        // with arrow heads draw with polyline, this is not so good.
        //path->RemoveRedundantPoints();
    }
    else
    {
        ReadHeader( GDS_XY );
        SkipData();
    }

    return true;
}

// Text ---------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadText( a2dCanvasObject* parent )
{
    // Grammar: TEXT [ELFLAGS] [PLEX] LAYER
    //              TEXTTYPE [PRESENTATION] [PATHTYPE] [WIDTH]
    //              [<strans>] XY STRING
    //
    //              <strans> = STRANS [MAG] [ANGLE]

    if ( !ReadHeader( GDS_TEXT ) )
        return false;


    // Elflags
    // instead of ReadElflags(Text) for speed
    int elflags = 0;
    if ( ReadHeader( GDS_ELFLAGS ) )
        elflags = ReadInt();

    if ( ReadHeader( GDS_PLEX ) )
        SkipData();

    int layer;
    if ( !ReadLayer( layer ) )
        throw GDS_Error( _( "Text: LAYER is missing (in GDS-file)" ) );

    //  <textbody>          ::= TEXTTYPE [PRESENTATION] [PATHTYPE]
    //                                  [WIDTH] [<strans>] XY STRING
    // Must be there
    if ( !ReadHeader( GDS_TEXTTYPE ) )
        throw GDS_Error( _( "Text: TEXTBODY is missing (in GDS-file)" ) );

    int texttype = ReadInt();
    int presentation = 0;
    int pathtype = 0;
    long textHeight = 0;
    // Optional
    if ( ReadHeader( GDS_PRESENTATION ) )
        presentation = ReadInt();

    if ( ReadHeader( GDS_PATHTYPE ) )
        pathtype = ReadInt();

    if ( ReadHeader( GDS_WIDTH ) )
    {
        // in MDS this is generated along with the MAG record holding the same,
        // so igore it here.
        // Text->SetTextHeight( ReadLong() * m_userunits_out * m_scale_out );
        textHeight = ReadLong();
    }

    Strans Strans;
    ReadStrans( Strans );

    if ( !ReadHeader( GDS_XY ) )
        throw GDS_Error( _( "Text: XY is missing (in GDS-file)" ) );

    a2dPoint2D point;
    Read( point );

    a2dAffineMatrix relative_matrix;

    // Ok, now we have a strans, but we want to work with a matrix,
    // so let's convert it to one:

    // Mirror in X if necessary
    if ( Strans.GetReflection() )
        relative_matrix.Mirror();

    //  Rotate if necessary
    if ( Strans.GetAbsAngle() == 0 )
        relative_matrix.Rotate( Strans.GetAngle(), 0, 0 );

    // Scale if necessary
    if ( Strans.GetAbsScale() == 0 )
    {
        //WE DIRECTLY SET IT TO TEXT HEIGHT
        //double scale = Strans.GetScale();
        //relative_matrix.Scale(scale, scale, 0, 0);
    }

    // Translate2D over XY from the structrereference
    relative_matrix.Translate( point.m_x, point.m_y );

    if ( !ReadHeader( GDS_STRING ) )
        throw GDS_Error( _( "Text: STRING is missing (in GDS-file)" ) );

    wxString a_string;
    ReadString( a_string );
    a_string.Replace( wxT( "\t" ), wxT( "        " ) );

    // Create a new Text
    a2dSmrtPtr<a2dTextGDS> Text = new a2dTextGDS( m_fontType, a_string, Strans.GetScale() * a2dTextGDS::GetFontScaleFactor(), presentation );
    //Text->SetLineSpacing( Text->GetLineHeight() / 119.25 );

    Text->SetLayer( m_mapping[ layer ]->GetLayer() );
    Text->SetTextType( texttype );
    Text->SetPathtype( ( TEXT_PATHTYPE )pathtype );
    Text->SetTransformMatrix( relative_matrix );

#ifdef _G_UNIX
    Text->SetTemplate( ( bool ) ( elflags && 2 ) );
    Text->SetExternal( ( bool ) ( elflags && 1 ) );
#else
    Text->SetTemplate( ( bool ) ( elflags && 1 ) );
    Text->SetExternal( ( bool ) ( elflags && 2 ) );
#endif


    if ( m_mapping[ layer ]->GetRead() )
        parent->Append( Text );

    return true;
}


// StructureReference -------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadStructureReference( a2dCanvasObject* parent )
{
//  <SREF>              ::= SREF [ELFLAGS] [PLEX] SNAME
//                                  [<strans>] XY
//  <strans>                ::= STRANS [MAG] [ANGLE]

    if ( m_asCameleons )
        return ReadStructureCameleonReference( parent );

    if ( !ReadHeader( GDS_SREF ) )
        return false;

    a2dPoint2D point;
    a2dAffineMatrix relative_matrix;
    a2dSmrtPtr<class a2dCanvasObjectReference> sref = new a2dCanvasObjectReference();

    try
    {

        ReadElflags( sref );

        if ( ReadHeader( GDS_PLEX ) )
            SkipData();

        if ( !ReadHeader( GDS_SNAME ) )
            throw GDS_Error( _( "Header: SNAME missing (in Structure Reference)" ) );

        wxString strname;
        ReadString( strname );
        sref->SetName( strname );

        Strans strans;
        ReadStrans( strans );

        if ( !ReadHeader( GDS_XY ) )
            throw GDS_Error( _( "Header: XY missing (in Structure Reference)" ) );

        Read( point );

        // Ok, now we have a strans, but we want to work with a matrix,
        // so let's convert it to one:

        // Mirror in X if necessary
        if ( strans.GetReflection() )
            relative_matrix.Mirror();

        //  Rotate if necessary
        if ( strans.GetAbsAngle() == 0 )
            relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

        // Scale if necessary
        if ( strans.GetAbsScale() == 0 )
        {
            double scale = strans.GetScale();
            relative_matrix.Scale( scale, scale, 0, 0 );
        }

        // Translate2D over XY from the structure reference
        relative_matrix.Translate( point.m_x, point.m_y );

        sref->SetTransformMatrix( relative_matrix );

        parent->Append( sref );
        ResolveOrAddLink( sref.Get(), sref->GetName() );
    }
    catch ( GDS_Error& _error )
    {
        throw _error;
    }

    return true;
}

// StructureReference -------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadStructureCameleonReference( a2dCanvasObject* parent )
{
//  <SREF>              ::= SREF [ELFLAGS] [PLEX] SNAME
//                                  [<strans>] XY
//  <strans>                ::= STRANS [MAG] [ANGLE]

    if ( !ReadHeader( GDS_SREF ) )
        return false;

    a2dPoint2D point;
    a2dAffineMatrix relative_matrix;
    a2dSmrtPtr<class a2dCameleonInst> sref = new a2dCameleonInst( 0, 0, (a2dDiagram*) 0 );

    try
    {

        ReadElflags( sref );

        if ( ReadHeader( GDS_PLEX ) )
            SkipData();

        if ( !ReadHeader( GDS_SNAME ) )
            throw GDS_Error( _( "Header: SNAME missing (in Structure Reference)" ) );

        wxString strname;
        ReadString( strname );
        sref->SetAppearanceName( strname + wxT(":layout") );
        sref->SetName( strname );

        Strans strans;
        ReadStrans( strans );

        if ( !ReadHeader( GDS_XY ) )
            throw GDS_Error( _( "Header: XY missing (in Structure Reference)" ) );

        Read( point );

        // Ok, now we have a strans, but we want to work with a matrix,
        // so let's convert it to one:

        // Mirror in X if necessary
        if ( strans.GetReflection() )
            relative_matrix.Mirror();

        //  Rotate if necessary
        if ( strans.GetAbsAngle() == 0 )
            relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

        // Scale if necessary
        if ( strans.GetAbsScale() == 0 )
        {
            double scale = strans.GetScale();
            relative_matrix.Scale( scale, scale, 0, 0 );
        }

        // Translate2D over XY from the structure reference
        relative_matrix.Translate( point.m_x, point.m_y );

        sref->SetTransformMatrix( relative_matrix );

        parent->Append( sref );
        ResolveOrAddLink( sref.Get(), sref->GetName() );
    }
    catch ( GDS_Error& _error )
    {
        throw _error;
    }

    return true;
}

// Arrayreference -----------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadArrayReference( a2dCanvasObject* parent )
{
    // Grammar: AREF [ELFLAGS] [PLEX] SNAME [<strans>]
    //              COLROW XY

    if ( !ReadHeader( GDS_AREF ) )
        return false;

    a2dPoint2D Point;
    a2dAffineMatrix relative_matrix;
    a2dPoint2D HorPoint;
    a2dPoint2D VerPoint;

    // Create a new Aref
    a2dSmrtPtr<class a2dCanvasObjectArrayReference> arrayref = new a2dCanvasObjectArrayReference();
    try
    {
        ReadElflags( arrayref );

        if ( ReadHeader( GDS_PLEX ) )
            SkipData();

        if ( !ReadHeader( GDS_SNAME ) )
            throw GDS_Error( _( "Header: SNAME missing (in Structure Reference)" ) );

        wxString strname;
        ReadString( strname );
        arrayref->SetName( strname );

        Strans strans;
        ReadStrans( strans );

        if ( !ReadHeader( GDS_COLROW ) )
            throw GDS_Error( _( "Header: COLROW missing (in Array Reference)" ) );

        int columns = ReadInt();
        arrayref->SetColumns( columns );

        int rows = ReadInt();
        arrayref->SetRows( rows );


        if ( !ReadHeader( GDS_XY ) )
            throw GDS_Error( _( "Header: XY missing (in Array Reference)" ) );

        Read( Point );
        Read( HorPoint );
        Read( VerPoint );

        arrayref->SetHorzSpace( sqrt( pow( HorPoint.m_x - Point.m_x, 2 ) + pow( HorPoint.m_y - Point.m_y, 2 ) ) / columns );
        arrayref->SetVertSpace( sqrt( pow( VerPoint.m_x - Point.m_x, 2 ) + pow( VerPoint.m_y - Point.m_y, 2 ) ) / rows );

        // Ok, now we have a strans, but we want to work with a matrix,
        // so let's convert it to one:

        // Mirror in X if necessary
        if ( strans.GetReflection() )
            relative_matrix.Mirror();

        //  Rotate if necessary
        if ( strans.GetAbsAngle() == 0 )
            relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

        // Scale if necessary
        if ( strans.GetAbsScale() == 0 )
        {
            EIGHT_G_BYTE_REAL scale = strans.GetScale();
            relative_matrix.Scale( scale, scale, 0, 0 );
        }

        // Translate2D over XY from the structurereference
        relative_matrix.Translate( Point.m_x, Point.m_y );
        arrayref->SetTransformMatrix( relative_matrix );

        parent->Append( arrayref );
        ResolveOrAddLink( arrayref.Get(), arrayref->GetName() );
    }
    catch ( GDS_Error& _error )
    {
        throw _error;
    }

    return true;
}


bool a2dIOHandlerGDSIn::ReadNode( a2dCanvasObject* WXUNUSED( parent ) )
{
    // not implemented
    return false;
}


// Box ----------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadBox( a2dCanvasObject* parent )
{
    // Grammar: BOX [ELFLAGS] [PLEX] LAYER BOXTYPE XY
    if ( !ReadHeader( GDS_BOX ) )
        return false;

    // Create a new Boundary
    a2dRect* Box = new a2dRect();

    ReadElflags( Box );

    if ( ReadHeader( GDS_PLEX ) )
        SkipData();

    if ( !ReadLayer( Box ) )
        throw GDS_Error( _( "Box: LAYER missing (in GDS-file)" ) );

    if ( !ReadHeader( GDS_BOXTYPE ) )
        throw GDS_Error( _( "Box: BOXTYPE missing (in GDS-file)" ) );

    Box->SetBoxType( ReadInt() );

    if ( !ReadHeader( GDS_XY ) )
        return false;

    int points = m_recordsize / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) );

    if ( points != 5 )
        throw GDS_Error( _( "Wrong number of points in BOX XY." ), _( "Fatal GDSII error" ) );

    a2dPoint2D seg;
    Read( seg );
    a2dPoint2D seg2;
    Read( seg2 );
    a2dPoint2D seg3;
    Read( seg3 );
    a2dPoint2D seg4;
    Read( seg4 );
    a2dPoint2D seg5;
    Read( seg5 ); // Last point matches first one read

    double minx, miny, maxx, maxy;

    minx = maxx = seg.m_x;
    miny = maxy = seg.m_y;

    minx = wxMin( minx, seg2.m_x );
    maxx = wxMax( maxx, seg2.m_x );
    miny = wxMin( miny, seg2.m_y );
    maxy = wxMax( maxy, seg2.m_y );

    minx = wxMin( minx, seg3.m_x );
    maxx = wxMax( maxx, seg3.m_x );
    miny = wxMin( miny, seg3.m_y );
    maxy = wxMax( maxy, seg3.m_y );

    minx = wxMin( minx, seg4.m_x );
    maxx = wxMax( maxx, seg4.m_x );
    miny = wxMin( miny, seg4.m_y );
    maxy = wxMax( maxy, seg4.m_y );

    Box->SetWidth( fabs( maxx - minx ) );
    Box->SetHeight( fabs( maxy - miny ) );
    Box->SetPosXY( minx, miny );

    if ( m_mapping[ Box->GetLayer() ]->GetRead() )
        parent->Append( Box );

    return true;
}

// Elflags ------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadElflags( a2dCanvasObject* object )
{
    if ( !ReadHeader( GDS_ELFLAGS ) )
        return false;

    int value;
    value = ReadInt();

#ifdef _G_UNIX
    object->SetTemplate( ( bool ) ( value && 2 ) );
    object->SetExternal( ( bool ) ( value && 1 ) );
#else
    object->SetTemplate( ( bool ) ( value && 1 ) );
    object->SetExternal( ( bool ) ( value && 2 ) );
#endif

    return true;
}

// Layer --------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadLayer( a2dCanvasObject* object )
{
    if ( !ReadHeader( GDS_LAYER ) )
        return false;

    int layer;
    layer = ReadInt();
    if ( layer < 0 || layer >= wxMAXLAYER )
    {
        layer = 0;
        wxString mes;
        mes.Printf( _( "Object %s, layer >= wxMAXLAYER put on layer 0 " ), object->GetClassInfo()->GetClassName() );
        a2dDocviewGlobals->ReportWarningF( a2dError_NotSpecified, _( "warning GDSII file \n %s \n" ),
                                           mes.c_str() );
    }

    a2dLayerInfo* info = m_mapping[ layer ];
    if ( !info )
    {
        wxString buf;
        buf.Printf( wxT( "layer %d" ), layer );
        info = new a2dLayerInfo( layer, buf );
        m_layers->Append( info );
        m_layers->SetPending( true );
        m_mapping[ layer ] = info;
    }
    object->SetLayer( info->GetLayer() );
    return true;
}

// Layer --------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadLayer( int& layer )
{
    if ( !ReadHeader( GDS_LAYER ) )
        return false;

    layer = ReadInt();
    if ( layer < 0 || layer >= wxMAXLAYER )
    {
        layer = 0;
        wxString mes;
        mes.Printf( wxT( "%s" ), _( "Object layer >= wxMAXLAYER put on layer 0 " ) );
        a2dDocviewGlobals->ReportWarningF( a2dError_NotSpecified, _( "warning GDSII file \n %s \n" ),
                                           mes.c_str() );
    }
    return true;
}

// Point --------------------------------------------------------------------
bool a2dIOHandlerGDSIn::Read( a2dPoint2D& Point )
{
    FOUR_G_BYTE_SIGNED_INTEGER value;
    value = ReadLong();
    Point.m_x = value * m_doc->GetUnitsAccuracy();
    value = ReadLong();
    Point.m_y = value * m_doc->GetUnitsAccuracy();
    return true;
}

// Polygon ------------------------------------------------------------------
bool a2dIOHandlerGDSIn::Read( a2dPolygonL* polygon )
{
    if ( !ReadHeader( GDS_XY ) )
        return false;

    int points = m_recordsize / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) ) - 1;

    if ( points < 2 )
    {
        if ( points == 1 )
        {
            a2dPoint2D seg;
            Read( seg );
            a2dDocviewGlobals->ReportErrorF( a2dError_GDSIIparse,
                                             _( "GDSII: Structure %s Only one point in Polygon: %f, %f " ), m_strucname.c_str(), seg.m_x, seg.m_y );
            polygon->AddPoint( seg.m_x, seg.m_y );
            a2dPoint2D segend;
            Read( segend ); // Last point matches first one read: skip
        }
        else
            throw GDS_Error( _( "Too few points in Polygon." ), _( "Fatal GDSII error" ) );
    }
    else
    {
        for ( int i = 0; i < points; i++ )
        {
            a2dPoint2D seg;
            Read( seg );
            polygon->AddPoint( seg.m_x, seg.m_y );
        }
        a2dPoint2D segend;
        Read( segend ); // Last point matches first one read: skip
    }

    return true;
}

// Polyline -----------------------------------------------------------------
bool a2dIOHandlerGDSIn::Read( a2dVertexList* segments )
{
    if ( !ReadHeader( GDS_XY ) )
        return false;

    int points = m_recordsize / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) );
    if ( points < 2 )
    {
        if ( points == 1 )
        {
            a2dLineSegment* seg = new a2dLineSegment( 0, 0 );

            FOUR_G_BYTE_SIGNED_INTEGER value;
            value = ReadLong();
            seg->m_x = value * m_doc->GetUnitsAccuracy();
            value = ReadLong();
            seg->m_y = value * m_doc->GetUnitsAccuracy();

            a2dDocviewGlobals->ReportErrorF( a2dError_GDSIIparse,
                                             _( "GDSII: Structure %s Only one point in Polyline: %f, %f " ), m_strucname.c_str(), seg->m_x, seg->m_y );
            segments->push_back( seg );
        }
        else
            throw GDS_Error( _( "Too few points in Polyline." ), _( "Fatal GDSII error" ) );
    }
    else
    {
        a2dLineSegment* seg;
        for ( int i = 0; i < points; i++ )
        {
            seg = new a2dLineSegment( 0, 0 );

            FOUR_G_BYTE_SIGNED_INTEGER value;
            value = ReadLong();
            seg->m_x = value * m_doc->GetUnitsAccuracy();
            value = ReadLong();
            seg->m_y = value * m_doc->GetUnitsAccuracy();
            segments->push_back( seg );
        }
    }

    return true;
}



// Strans -------------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadStrans( Strans& Strans )
{
    if ( !ReadHeader( GDS_STRANS ) )
        return( false );

    EIGHT_G_BYTE_REAL real;

    Strans.m_stransflags.total = ReadInt();

    if ( ReadHeader( GDS_MAG ) )
    {
        real = ReadDouble();
        Strans.SetScale( real );
    }

    if ( ReadHeader( GDS_ANGLE ) )
    {
        real = ReadDouble();
        Strans.SetAngle( real );
    }

    return true;
}


// Structure ----------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadStructure( a2dCanvasObject* parent )
{
    // Grammar: BGNSTR STRNAME [STRCLASS] {<element>}*
    //              ENDSTR

    if ( !ReadHeader( GDS_BGNSTR ) )
        return false;

    a2dSmrtPtr<class a2dCanvasObject> Structure = new a2dCanvasObject();
    Structure->SetIgnoreLayer( true );

    //TODO store as property on current object????
    wxDateTime m_modificationtime = wxDateTime::Now();
    wxDateTime m_accesstime = wxDateTime::Now();

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    year = ReadInt();
    month = ReadInt() - 1;
    day = ReadInt();
    hour = ReadInt();
    minute = ReadInt();
    second = ReadInt();

    m_modificationtime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_ModificationDateTime->SetPropertyToObject( Structure, m_modificationtime );

    year = ReadInt();
    month = ReadInt() - 1;
    day = ReadInt();
    hour = ReadInt();
    minute = ReadInt();
    second = ReadInt();

    m_accesstime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_AccessDateTime->SetPropertyToObject( Structure, m_accesstime );

    if ( !ReadHeader( GDS_STRNAME ) )
        throw GDS_Error( _( "STRNAME record missing (in Structure)" ) );

    ReadString( m_strucname );
    Structure->SetName( m_strucname );
    //wxLogDebug(wxT("start structure= %s"), strucname.c_str() );

    if( ReadHeader( GDS_STRCLASS ) )
        SkipData();

    while ( ReadElement( Structure ) )
    {
    }


    if( !ReadHeader( GDS_ENDSTR ) )
        throw GDS_Error( _( "ENDSTR record missing (in GDSII)" ) );

    // Add this Element to the end of the list
    
    if ( m_asMultiroot )
    {      
        a2dNameReference* ref = new a2dNameReference( m_gridx, m_gridy,  Structure, m_strucname );
        ref->SetDrawFrame();
        parent->Append( ref );

        if ( m_gridx < 5 * (10 * a2dCanvasGlobals->GetHabitat()->GetObjectGridSize()) )
            m_gridx = a2dCanvasGlobals->GetHabitat()->GetObjectGridSize()*10 + m_gridx; 
        else
        {    
            m_gridx = 0;
            m_gridy = a2dCanvasGlobals->GetHabitat()->GetObjectGridSize() + m_gridy;
        }
    }
    else
        parent->Append( Structure );

    // this one needs to be resolved later
    GetObjectHashMap()[m_strucname] = Structure;
    //wxLogDebug(wxT("structure= %s"), strucname.c_str() );

    return true;
}

// Structure ----------------------------------------------------------------
bool a2dIOHandlerGDSIn::ReadCameleonStructure( a2dCanvasObject* parent )
{
    // Grammar: BGNSTR STRNAME [STRCLASS] {<element>}*
    //              ENDSTR

    if ( !ReadHeader( GDS_BGNSTR ) )
        return false;

    a2dSmrtPtr<a2dCameleon> cam = new a2dCameleon();
    a2dLayout* layout = new a2dLayout( cam, 0,0 );
    cam->AddAppearance( layout ); 
    a2dSmrtPtr<a2dDrawing> Structure = layout->GetDrawing();
    Structure->SetLayerSetup( m_layers );

    //TODO store as property on current object????
    wxDateTime m_modificationtime = wxDateTime::Now();
    wxDateTime m_accesstime = wxDateTime::Now();

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    year = ReadInt();
    month = ReadInt() - 1;
    day = ReadInt();
    hour = ReadInt();
    minute = ReadInt();
    second = ReadInt();

    m_modificationtime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_ModificationDateTime->SetPropertyToObject( layout, m_modificationtime );

    year = ReadInt();
    month = ReadInt() - 1;
    day = ReadInt();
    hour = ReadInt();
    minute = ReadInt();
    second = ReadInt();

    m_accesstime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_AccessDateTime->SetPropertyToObject( layout, m_accesstime );

    if ( !ReadHeader( GDS_STRNAME ) )
        throw GDS_Error( _( "STRNAME record missing (in Structure)" ) );

    ReadString( m_strucname );
    cam->SetName( m_strucname );
    layout->SetName( m_strucname + wxT(":layout") );
    //wxLogDebug(wxT("start structure= %s"), strucname.c_str() );

    if( ReadHeader( GDS_STRCLASS ) )
        SkipData();

    while ( ReadElement( Structure->GetRootObject() ) )
    {
    }


    if( !ReadHeader( GDS_ENDSTR ) )
        throw GDS_Error( _( "ENDSTR record missing (in GDSII)" ) );

    // Add this Element to the end of the list
    cam->AddToRoot();
    int px = m_dx * 30 * 1.5;
    int py = -m_dy * 10 * 1.5;
    cam->SetPosXY( px, py );
    m_dx++;
    if ( m_dx > m_refMaxx ) 
        { m_dy++; m_dx =0; }

    // this one needs to be resolved later
    GetObjectHashMap()[ m_strucname ] = cam;
    //wxLogDebug(wxT("structure= %s"), strucname.c_str() );

    return true;
}

void a2dIOHandlerGDSIn::SetDataTypeProperty( a2dCanvasObject* toobject, wxUint16 type )
{
    if ( type != 0 )
        a2dCanvasObject::PROPID_Datatype->SetPropertyToObject( toobject, type );
}

//----------------------------------------------------------------------------------/
//    a2dIOHandlerGDSOut
//----------------------------------------------------------------------------------/

a2dIOHandlerGDSOut::a2dIOHandlerGDSOut()
{
    m_scale_out = 1;
    m_textAsPath = false;
    m_fromViewAsTop = false;
    m_asCameleons = false;
    m_AberArcToPoly = 0;
}

a2dIOHandlerGDSOut::~a2dIOHandlerGDSOut()
{
}

bool a2dIOHandlerGDSOut::CanSave( const wxObject* obj )
{
    if ( !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;
    return true;
}

void a2dIOHandlerGDSOut::InitializeSave()
{
    a2dIOHandlerStrOut::InitializeSave();
}

void a2dIOHandlerGDSOut::ResetSave(  )
{
    a2dIOHandlerStrOut::ResetSave();
}

bool a2dIOHandlerGDSOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    m_doc = ( a2dCanvasDocument* ) doc;
    InitializeSave();

    a2dCanvasObject* showobject = m_doc->GetStartObject();
    if ( !showobject )
        showobject = m_doc->GetDrawing()->GetRootObject();

    m_layers = m_doc->GetDrawing()->GetLayerSetup();

    if  ( !m_AberArcToPoly )
        m_AberArcToPoly = double( m_doc->GetDrawing()->GetHabitat()->GetAberPolyToArc() ) / m_doc->GetUnitsScale();

    if ( m_fromViewAsTop )
    {
        a2dCanvasView* drawer = wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), a2dCanvasView );
        if ( drawer )
            showobject = drawer->GetDrawingPart()->GetShowObject();
    }

    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( showobject, false );

    for ( a2dLayerMapNr::iterator it= m_layers->GetLayerIndex().begin(); it != m_layers->GetLayerIndex().end(); ++it)
    {
        a2dLayerInfo* layerobj = it->second;
        wxUint16 i = it->first;
        m_mapping[ layerobj->GetOutMapping() ] = layerobj;
    }

    m_streamo = &stream;


    //first check if the dat is not to big to fit in four byte integers
    //this is the maximum for vertexes in GDSII data

    a2dBoundingBox drawing = showobject->GetBbox();

    m_userunits_out = m_doc->GetUnitsAccuracy();
    if ( m_userunits_out == 0 )
    {
        wxLogWarning( wxT( "GDS userunits: 0 was set to 0.01" ) );
        m_userunits_out = 0.01;
    }
    drawing.SetMin( drawing.GetMinX() / m_userunits_out, drawing.GetMinY() / m_userunits_out );
    drawing.SetMax( drawing.GetMaxX() / m_userunits_out, drawing.GetMaxY() / m_userunits_out );
    a2dBoundingBox maxint( LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX );

    int l = 0;
    while ( maxint.Intersect( drawing, 0 ) != _IN )
    {
        drawing.SetMin( drawing.GetMinX() / 10.0, drawing.GetMinY() / 10.0 );
        drawing.SetMax( drawing.GetMaxX() / 10.0, drawing.GetMaxY() / 10.0 );
        l++;
    }
    m_scale_out = pow( 10., l );

    WriteHeader( 2, GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER, GDS_HEADER );

	long version;
	bool oke = m_doc->GetVersion().ToLong( &version );
	if ( oke )
		WriteInt( version );
	else
		WriteInt( 1 );

    WriteHeader( 12 * sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_BGNLIB );

    WriteInt( m_doc->GetModificationTime().GetYear() );
    WriteInt( m_doc->GetModificationTime().GetMonth() + 1 );
    WriteInt( m_doc->GetModificationTime().GetDay() );
    WriteInt( m_doc->GetModificationTime().GetHour() );
    WriteInt( m_doc->GetModificationTime().GetMinute() );
    WriteInt( m_doc->GetModificationTime().GetSecond() );

    WriteInt( m_doc->GetAccessTime().GetYear() );
    WriteInt( m_doc->GetAccessTime().GetMonth() + 1 );
    WriteInt( m_doc->GetAccessTime().GetDay() );
    WriteInt( m_doc->GetAccessTime().GetHour() );
    WriteInt( m_doc->GetAccessTime().GetMinute() );
    WriteInt( m_doc->GetAccessTime().GetSecond() );

    int value = wxStrlen( m_doc->GetLibraryName() );
    if ( value % 2 != 0 )
        value++;
    WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_LIBNAME );

    WriteString( m_doc->GetLibraryName() );
    WriteHeader( 2 * sizeof( EIGHT_G_BYTE_REAL ),
                 GDSTYPE_EIGHT_G_BYTE_REAL,
                 GDS_UNITS );

    WriteDouble( m_userunits_out );
    WriteDouble( m_doc->GetUnitsScale()*m_userunits_out * m_scale_out );
    a2dCanvasObjectList towrite;
    if ( m_doc->GetDrawing()->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        if ( m_fromViewAsTop )
            towrite.push_back( showobject );
        else
        {
            a2dCanvasObjectList::iterator rootchild = m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->begin();
            while ( rootchild != m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->end() )
            {
                a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
                if ( ref )
                {
                    //a2dLayout* layout = ref->GetCameleon()->GetAppearance<a2dDiagram>();
                    //towrite.push_back( layout->GetDrawing() );
                    //layout->SetBin( false );
                    towrite.push_back( ref );
                    ref->SetBin( false );
                }
                rootchild++;
            }
        }
    }
    else
    {
        if ( m_fromViewAsTop || !m_doc->GetMultiRoot() )
            towrite.push_back( showobject );
        else
        {
            m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->CollectObjects( &towrite );
        }
    }

    a2dCanvasObjectList::iterator iter = towrite.begin();
    while ( towrite.size() )
    {
        a2dCanvasObject* obj = *iter;
        if ( m_doc->GetMultiRoot() &&  !m_fromViewAsTop )
        {
            a2dNameReference* ref = wxDynamicCast( obj, a2dNameReference );
            if ( ref )
                SaveStructure( ref->GetCanvasObject(), &towrite );
        }
        else
            SaveStructure( obj, &towrite );

        obj->SetBin( true );
        towrite.erase( iter );
        iter = towrite.begin();
    }

    WriteHeader( 0, 0, GDS_ENDLIB );

    ResetSave();

    return true;
}

void a2dIOHandlerGDSOut::SaveStructure( a2dCanvasObject* object, a2dCanvasObjectList* towrite )
{
    SetFlags( object );
    if (  !object->GetRelease() )
    {
        if ( !object->GetBin() )
        {
            //write a struture BEGIN

            // BGNSTR
            WriteHeader( 12 * sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                         GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                         GDS_BGNSTR );

            wxDateTime modTime = a2dCanvasObject::PROPID_ModificationDateTime->GetPropertyValue( object );

            WriteInt( modTime.GetYear() );
            WriteInt( modTime.GetMonth() + 1 );
            WriteInt( modTime.GetDay() );
            WriteInt( modTime.GetHour() );
            WriteInt( modTime.GetMinute() );
            WriteInt( modTime.GetSecond() );

            wxDateTime accesTime = a2dCanvasObject::PROPID_AccessDateTime->GetPropertyValue( object );

            WriteInt( accesTime.GetYear() );
            WriteInt( accesTime.GetMonth() + 1 );
            WriteInt( accesTime.GetDay() );
            WriteInt( accesTime.GetHour() );
            WriteInt( accesTime.GetMinute() );
            WriteInt( accesTime.GetSecond() );

            // STRNAME
            TWO_G_BYTE_UNSIGNED_INTEGER value = ( TWO_G_BYTE_UNSIGNED_INTEGER ) wxStrlen( object->GetName() );
            WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_STRNAME );
            WriteString( object->GetName() );

            if ( 0 != wxDynamicCast( object, a2dCameleon ) )
            {
                a2dCameleon* cam = wxDynamicCast( object, a2dCameleon );
                //a2dDiagram* dia = wxDynamicCast( cam->GetAppearanceByName( cam->GetName() ), a2dDiagram );
                a2dDiagram* dia = wxDynamicCast( cam->GetAppearance<a2dDiagram>(), a2dDiagram );
                if ( dia )
                { 
                    a2dCanvasObjectList* childobjects = dia->GetDrawing()->GetRootObject()->GetChildObjectList();
                    if ( childobjects != wxNullCanvasObjectList )
                    {
                        forEachIn( a2dCanvasObjectList, childobjects )
                        {
                            a2dCanvasObject* obj = *iter;
                            if (  !obj->GetRelease() &&  m_layers->GetVisible( obj->GetLayer() ) || obj->GetIgnoreLayer() )
                            {
                                Save( obj, towrite );
                            }
                        }
                    }
                }
            }
            else
            {
                a2dCanvasObjectList* childobjects = object->GetChildObjectList();
                if ( childobjects != wxNullCanvasObjectList )
                {
                    forEachIn( a2dCanvasObjectList, childobjects )
                    {
                        a2dCanvasObject* obj = *iter;
                        if (  !obj->GetRelease() &&  m_layers->GetVisible( obj->GetLayer() ) || obj->GetIgnoreLayer() )
                        {
                            Save( obj, towrite );
                        }
                    }
                }
            }
            WriteHeader( 0, 0, GDS_ENDSTR );
        }
    }
}

void a2dIOHandlerGDSOut::Save( a2dCanvasObject* object, a2dCanvasObjectList* towrite )
{
    if( object->IsTemporary_DontSave() )
        return;

    //in case an object can not be written as is, and need to be converted to vector paths, and written that as childs.
    a2dCanvasObjectList* vectorpaths = wxNullCanvasObjectList;

    SetFlags( object );
    //write the object itself as part of the structure (which is the parent )
    if ( 0 != wxDynamicCast( object, a2dPolylineL ) )
    {
        a2dPolylineL* obj = wxDynamicCast( object, a2dPolylineL );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dSurface ) )
    {
        // treat surfaces without conversion to vector paths
        a2dSmrtPtr<a2dSurface> poly = wxDynamicCast( object, a2dSurface );
        poly = wxDynamicCast( poly->Clone( a2dObject::clone_deep ), a2dSurface );
        if ( poly && poly->GetSegments()->HasArcs() )
        {
            poly->GetSegments()->ConvertToLines( m_AberArcToPoly );
        }
        WriteBoundary( poly->GetTransformMatrix(), poly->GetSegments(), poly->GetLayer(), a2dCanvasObject::PROPID_Datatype->GetPropertyValue( poly ), poly->GetSpline() );

        a2dListOfa2dVertexList& holes = poly->GetHoles();
        if ( holes.size() )
            wxLogWarning( _( "a2dSurface holes are saved as polygons in contour, better convert to polygons first" ) );

        for( a2dListOfa2dVertexList::iterator iterp = holes.begin(); iterp != holes.end(); iterp++ )
        {
            a2dVertexListPtr vlist = ( *iterp );
            if ( vlist->HasArcs() )
                vlist->ConvertToLines( m_AberArcToPoly );

            WriteBoundary( poly->GetTransformMatrix(), vlist, poly->GetLayer(), a2dCanvasObject::PROPID_Datatype->GetPropertyValue( poly ), poly->GetSpline() );
        }
    }
    else if ( 0 != wxDynamicCast( object, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxDynamicCast( object, a2dPolygonL );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dVectorPath ) )
    {
        a2dVectorPath* obj = wxDynamicCast( object, a2dVectorPath );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCanvasObjectArrayReference ) )
    {
        a2dCanvasObjectArrayReference* obj = wxDynamicCast( object, a2dCanvasObjectArrayReference );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCanvasObjectReference ) )
    {
        a2dCanvasObjectReference* obj = wxDynamicCast( object, a2dCanvasObjectReference );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCameleonInst ) )
    {
        a2dCameleonInst* obj = wxDynamicCast( object, a2dCameleonInst );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCameleon ) )
    {
        a2dCameleon* obj = wxDynamicCast( object, a2dCameleon );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dTextGDS ) )
    {
        a2dTextGDS* obj = wxDynamicCast( object, a2dTextGDS );

        if ( !m_textAsPath )
            DoSave( obj, towrite );
        else
            vectorpaths = obj->GetAsCanvasVpaths( false );
    }
    else if ( 0 != wxDynamicCast( object, a2dText ) )
    {
        a2dText* obj = wxDynamicCast( object, a2dText );

        if ( !m_textAsPath )
            DoSave( obj, towrite );
        else
            vectorpaths = obj->GetAsCanvasVpaths( false );
    }
    else if (   ( 0 != wxDynamicCast( object, a2dArrow ) ) ||
                ( 0 != wxDynamicCast( object, a2dRectC ) ) ||
                ( 0 != wxDynamicCast( object, a2dRect  ) ) ||
                ( 0 != wxDynamicCast( object, a2dCircle ) ) ||
                ( 0 != wxDynamicCast( object, a2dEllipse ) ) ||
                ( 0 != wxDynamicCast( object, a2dEllipticArc ) ) ||
                ( 0 != wxDynamicCast( object, a2dArc ) ) ||
                ( 0 != wxDynamicCast( object, a2dSLine ) )
            )
    {
        vectorpaths = object->GetAsCanvasVpaths( false );
    }
    else if ( wxString( object->GetClassInfo()->GetClassName() ) == wxT( "a2dCanvasObject" ) )
    {
        WriteHeader( 0, 0, GDS_SREF );
        WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_BIT_ARRAY,
                     GDS_ELFLAGS );

        WriteInt( m_objectFlags );

        int value = wxStrlen( object->GetName() );
        if ( value % 2 != 0 )
            value++;
        WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_SNAME );
        WriteString( object->GetName() );

        //write a structure reference in the parent object
        a2dAffineMatrix lworld = object->GetTransformMatrix();
        if ( !lworld.IsIdentity() )
        {
            Strans* strans = new Strans();
            strans->MakeStrans( lworld );
            if ( strans->GetStrans() )
                Write( strans );
        }

        WriteHeader( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                     GDS_XY );

        WritePoint( object->GetPosX(), object->GetPosY() );

        WriteHeader( 0, 0, GDS_ENDEL );
        towrite->push_back( object );
    }
    else
    {
        wxLogWarning( wxT( "GDSIO : object %s is not implemented for output" ), object->GetClassInfo()->GetClassName() );
    }

    //now write extra vector paths ( e.g. as a result of a converted object )
    if ( vectorpaths != wxNullCanvasObjectList )
    {
        a2dAffineMatrix lworld = object->GetTransformMatrix();

        for( a2dCanvasObjectList::iterator iter = vectorpaths->begin(); iter != vectorpaths->end(); ++iter )
        {
            a2dVectorPath* canpath = ( a2dVectorPath* ) ( *iter ).Get();
            WriteVpath( lworld, canpath->GetSegments(), canpath->GetLayer(), canpath->GetDataType(), canpath->GetPathType(), canpath->GetContourWidth() );
        }
        delete vectorpaths;
    }

    if ( object->GetChildObjectList() != wxNullCanvasObjectList &&  object->GetChildObjectsCount() )
    {
        WriteHeader( 0, 0, GDS_SREF );
        WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_BIT_ARRAY,
                     GDS_ELFLAGS );

        WriteInt( m_objectFlags );

        int value = wxStrlen( object->GetName() );
        if ( value % 2 != 0 )
            value++;
        WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_SNAME );
        WriteString( object->GetName() );

        //write a structure reference in the parent object
        a2dAffineMatrix lworld = object->GetTransformMatrix();
        if ( !lworld.IsIdentity() )
        {
            Strans* strans = new Strans();
            strans->MakeStrans( lworld );
            if ( strans->GetStrans() )
                Write( strans );
        }

        WriteHeader( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                     GDS_XY );

        WritePoint( object->GetPosX(), object->GetPosY() );

        WriteHeader( 0, 0, GDS_ENDEL );

        //a new structure is needed
        towrite->push_back( object );
    }

}

void a2dIOHandlerGDSOut::DoSave( a2dPolylineL* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    a2dVertexList* origpoints = obj->GetSegments();

    a2dVertexList* rlist;
    if (  obj->GetSpline()  || origpoints->HasArcs() )
    {

        a2dVertexList* lpoints = new a2dVertexList();
        *lpoints = *origpoints;
        lpoints->ConvertSplinedPolyline( 10 );
        rlist = lpoints;
    }
    else
        rlist = origpoints;

    WritePath( obj->GetTransformMatrix(), rlist, obj->GetLayer(),
               a2dCanvasObject::PROPID_Datatype->GetPropertyValue( obj ), obj->GetPathType(), obj->GetContourWidth() );

    /*
        double xscale = obj->GetEndScaleX();
        double yscale = obj->GetEndScaleY();

        if ( obj->GetBegin() )
        {
            a2dVertexList::compatibility_iterator node = rlist->GetFirst();
            a2dPoint2D* point1 = (a2dPoint2D*)node->GetData();
            node = node->GetNext();
            a2dPoint2D* point2 = (a2dPoint2D*)node->GetData();

            double dx, dy;
            dx=point2->m_x-point1->m_x;
            dy=point2->m_y-point1->m_y;

            double ang1;
            if (!dx && !dy)
                ang1=0;
            else
                ang1 = wxRadToDeg(atan2(dy,dx));

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale(xscale,yscale,0,0);
            lworld.Rotate(-ang1);
            lworld.Translate(point1->m_x,point1->m_y);

            WriteStartElementAttributes( "g" );
            s.Printf ( "matrix( %f %f %f %f %f %f )",
                       lworld.GetValue(0,0), lworld.GetValue(0,1),
                       lworld.GetValue(1,0), lworld.GetValue(1,1),
                       lworld.GetValue(2,0), lworld.GetValue(2,1)
                     );
            WriteAttribute( "transform", s );
            WriteEndAttributes();

            Save( obj->GetBegin(), layer );

            WriteEndElement();
        }

        if ( obj->GetEnd() )
        {
            a2dVertexList::compatibility_iterator node = rlist->GetLast();
            a2dPoint2D* point1 = (a2dPoint2D*)node->GetData();
            node = node->GetPrevious();
            a2dPoint2D* point2 = (a2dPoint2D*)node->GetData();

            double dx, dy;
            dx=point2->m_x-point1->m_x;
            dy=point2->m_y-point1->m_y;
            double ang2;
            if (!dx && !dy)
                ang2=0;
            else
                ang2 = wxRadToDeg(atan2(dy,dx));

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale(xscale,yscale,0,0);
            lworld.Rotate(-ang2);
            lworld.Translate(point1->m_x,point1->m_y);

            WriteStartElementAttributes( "g" );
            s.Printf ( "matrix( %f %f %f %f %f %f )",
                       lworld.GetValue(0,0), lworld.GetValue(0,1),
                       lworld.GetValue(1,0), lworld.GetValue(1,1),
                       lworld.GetValue(2,0), lworld.GetValue(2,1)
                     );
            WriteAttribute( "transform", s );
            WriteEndAttributes();

            Save( obj->GetEnd(), layer );

            WriteEndElement();
        }
    */
    if ( obj->GetSpline() || origpoints->HasArcs() )
        delete rlist;
}

void a2dIOHandlerGDSOut::DoSave( a2dPolygonL* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    WriteBoundary( obj->GetTransformMatrix(), obj->GetSegments(), obj->GetLayer(), a2dCanvasObject::PROPID_Datatype->GetPropertyValue( obj ), obj->GetSpline() );
}

void a2dIOHandlerGDSOut::DoSave( a2dVectorPath* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    WriteVpath( obj->GetTransformMatrix(), obj->GetSegments(), obj->GetLayer(), a2dCanvasObject::PROPID_Datatype->GetPropertyValue( obj ), obj->GetPathType(), obj->GetContourWidth() );
}

void a2dIOHandlerGDSOut::DoSave( a2dText* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    WriteHeader( 0, 0, GDS_TEXT );
    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_LAYER );
    WriteInt( m_mapping[obj->GetLayer()]->GetLayer() );

    // Texttype
    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_TEXTTYPE );

    WriteInt( TEXT_PATH_END_SQAURE );

    /* for the moment this is in the strans together with matrix
        if ( obj->GetTextHeight() != 0 )
        {
            WriteHeader(sizeof(FOUR_G_BYTE_SIGNED_INTEGER),
                            GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                            GDS_WIDTH);
            WriteLong( (int) (obj->GetTextHeight() / m_userunits_out/m_scale_out) );
        }
    */

    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );
        if( obj->GetTextHeight() != 0 )
            strans.SetScale( strans.GetScale() * obj->GetTextHeight() );

        if ( strans.GetStrans() )
            Write( &strans );
    }

    // write the coordinate to the outputstream
    WriteHeader( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                 GDS_XY );

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    // writes the text
    int nrChar = wxStrlen( obj->GetText() );
    if ( nrChar % 2 != 0 )
        nrChar++;
    WriteHeader( nrChar, GDSTYPE_ASCII_STRING, GDS_STRING );

    WriteString( obj->GetText() );

    WriteHeader( 0, 0, GDS_ENDEL );
}

void a2dIOHandlerGDSOut::DoSave( a2dTextGDS* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    WriteHeader( 0, 0, GDS_TEXT );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_LAYER );
    
    WriteInt( m_mapping[obj->GetLayer()]->GetLayer() );

    // Texttype
    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_TEXTTYPE );

    WriteInt( obj->GetTextType() );

    if ( obj->GetPresentationFlags() )
    {
        // write only when a_presentation isn't default
        if ( ( obj->GetFontGDS() != DEFAULT_PRESENTATION_FONT ) ||
                ( obj->GetVertical() != DEFAULT_PRESENTATION_VERTICAL ) ||
                ( obj->GetHorizontal() != DEFAULT_PRESENTATION_HORIZONTAL ) )
        {
            WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                         GDSTYPE_BIT_ARRAY,
                         GDS_PRESENTATION );
            WriteInt( obj->GetPresentationFlags() );
        }
    }

    if ( obj->GetPathtype() )
    {
        WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                     GDS_PATHTYPE );

        WriteInt( obj->GetPathtype() );
    }

    /* for the moment this is in the strans together with matrix
        if ( obj->GetTextHeight() != 0 )
        {
            WriteHeader(sizeof(FOUR_G_BYTE_SIGNED_INTEGER),
                            GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                            GDS_WIDTH);
            WriteLong( (int) (obj->GetTextHeight() / m_userunits_out/m_scale_out) );
        }
    */

    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );
        if( obj->GetTextHeight() != 0 )
            strans.SetScale( strans.GetScale() * obj->GetTextHeight() / a2dTextGDS::GetFontScaleFactor() );

        if ( strans.GetStrans() )
            Write( &strans );
    }

    // write the coordinate to the outputstream
    WriteHeader( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                 GDS_XY );

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    // writes the text
    int nrChar = wxStrlen( obj->GetText() );
    if ( nrChar % 2 != 0 )
        nrChar++;
    WriteHeader( nrChar, GDSTYPE_ASCII_STRING, GDS_STRING );

    WriteString( obj->GetText() );

    WriteHeader( 0, 0, GDS_ENDEL );
}

void a2dIOHandlerGDSOut::DoSave( a2dCanvasObjectReference* obj, a2dCanvasObjectList* towrite )
{
    WriteHeader( 0, 0, GDS_SREF );
    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    int value = wxStrlen( obj->GetName() );
    if ( value % 2 != 0 )
        value++;
    WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_SNAME );
    WriteString( obj->GetName() );

    //write a structure reference in the parent object
    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans* strans = new Strans();
        strans->MakeStrans( lworld );
        if ( strans->GetStrans() )
            Write( strans );
    }

    WriteHeader( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                 GDS_XY );

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    WriteHeader( 0, 0, GDS_ENDEL );

    if ( obj->GetCanvasObject() )
        towrite->push_back( obj->GetCanvasObject() );
}

void a2dIOHandlerGDSOut::DoSave( a2dCanvasObjectArrayReference* obj, a2dCanvasObjectList* towrite )
{
    WriteHeader( 0, 0, GDS_AREF );
    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    int value = wxStrlen( obj->GetName() );
    if ( value % 2 != 0 )
        value++;
    WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_SNAME );
    WriteString( obj->GetName() );

    //write a structure reference in the parent object
    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans* strans = new Strans();
        strans->MakeStrans( lworld );
        if ( strans->GetStrans() )
            Write( strans );
    }

    WriteHeader( 2 * sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_COLROW );

    WriteInt( obj->GetColumns() );
    WriteInt( obj->GetRows() );

    WriteHeader( 6 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                 GDS_XY );

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    WritePoint( obj->GetPosX() + obj->GetHorzSpace() * obj->GetColumns(), obj->GetPosY() );
    WritePoint( obj->GetPosX(), obj->GetPosY() + obj->GetVertSpace() * obj->GetRows() );

    WriteHeader( 0, 0, GDS_ENDEL );

    if ( obj->GetCanvasObject() )
        towrite->push_back( obj->GetCanvasObject() );
}

void a2dIOHandlerGDSOut::DoSave( a2dCameleonInst* obj, a2dCanvasObjectList* towrite )
{
    a2dDiagram* dia = wxDynamicCast( obj->GetAppearance(), a2dDiagram );
    if ( ! dia )
        return;

    WriteHeader( 0, 0, GDS_SREF );
    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    int value = wxStrlen( dia->GetCameleon()->GetName() );
    if ( value % 2 != 0 )
        value++;
    WriteHeader( value, GDSTYPE_ASCII_STRING, GDS_SNAME );
    WriteString( dia->GetCameleon()->GetName() );

    //write a structure reference in the parent object
    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans* strans = new Strans();
        strans->MakeStrans( lworld );
        if ( strans->GetStrans() )
            Write( strans );
    }

    WriteHeader( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                 GDS_XY );

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    WriteHeader( 0, 0, GDS_ENDEL );

    if ( m_asCameleons && dia->GetCameleon() )
        towrite->push_back( dia->GetCameleon() );
}

void a2dIOHandlerGDSOut::DoSave( a2dCameleon* obj, a2dCanvasObjectList* towrite )
{
    //SaveStructure( obj, towrite );
}

void a2dIOHandlerGDSOut::Write( Strans* Strans )
{
    // writes only when strans isn't default
    if ( ( Strans->m_stransflags.bits.abs_angle != DEFAULT_STRANS_ABS_ANGLE ) ||
            ( Strans->m_stransflags.bits.abs_scale != DEFAULT_STRANS_ABS_SCALE ) ||
            ( Strans->m_stransflags.bits.reflection != DEFAULT_STRANS_REFLECTION ) ||
            ( Strans->GetScale() != DEFAULT_STRANS_SCALE ) ||
            ( Strans->GetAngle() != DEFAULT_STRANS_ANGLE ) )
    {
        WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_BIT_ARRAY,
                     GDS_STRANS );
        WriteInt( Strans->m_stransflags.total );

        // writes only the scale when different from default
        if ( Strans->GetScale() != DEFAULT_STRANS_SCALE )
        {
            WriteHeader( sizeof( EIGHT_G_BYTE_REAL ),
                         GDSTYPE_EIGHT_G_BYTE_REAL,
                         GDS_MAG );
            WriteDouble( Strans->GetScale() );

        }

        // writes only the angle when different from default
        if ( Strans->GetAngle() != DEFAULT_STRANS_ANGLE )
        {
            WriteHeader( sizeof( EIGHT_G_BYTE_REAL ),
                         GDSTYPE_EIGHT_G_BYTE_REAL,
                         GDS_ANGLE );
            WriteDouble( Strans->GetAngle() );
        }
    }
}

void a2dIOHandlerGDSOut::WriteHeader( TWO_G_BYTE_UNSIGNED_INTEGER nw_recordsize, G_BYTE nw_datatype, G_BYTE nw_recordtype )
{
    // a string must be odd length
    if ( nw_datatype == GDSTYPE_ASCII_STRING &&
            ( nw_recordsize % 2 ) != 0 )
        nw_recordsize++;

    WriteInt( nw_recordsize + GDS_HEADERSIZE );
    WriteByte( nw_recordtype );
    WriteByte( nw_datatype );

    m_recordsize = ( TWO_G_BYTE_UNSIGNED_INTEGER ) nw_recordsize;
}



void a2dIOHandlerGDSOut::WriteString( const wxString& a_string )
{
    int value = a_string.Len();
    for ( int i = 0 ; i < value; i++ )
        WriteByte( a_string[i] );

    if ( value % 2 != 0 )
        WriteByte( wxT( '\0' ) );
}


// Elflags ------------------------------------------------------------------
void a2dIOHandlerGDSOut::SetFlags( a2dCanvasObject* Element )
{
    if ( ( Element->GetTemplate() != 0 ) ||
            ( Element->GetExternal() != 0 ) )
    {
        m_objectFlags = 0;

        if ( Element->GetTemplate() )
            m_objectFlags += 1 << 1;

        if ( Element->GetExternal() )
            m_objectFlags += 1 << 2;
    }
}

// Point --------------------------------------------------------------------
void a2dIOHandlerGDSOut::WritePoint( double xi, double yi )
{
    long x;
    long y;
    if ( xi > 0 )
        x = ( long ) ( xi /  m_userunits_out / m_scale_out + 0.5 );
    else
        x = ( long ) ( xi /  m_userunits_out / m_scale_out - 0.5 );
    if ( yi > 0 )
        y = ( long ) ( yi /  m_userunits_out / m_scale_out + 0.5 );
    else
        y = ( long ) ( yi /  m_userunits_out / m_scale_out - 0.5 );

    WriteLong( x );
    WriteLong( y );
}

void a2dIOHandlerGDSOut::WriteVpath( const a2dAffineMatrix& lworld, const a2dVpath* path, int layer, int datatype, int pathtype, double width )
{
    double tstep = 1 / ( double ) SPLINE_STEP;
    unsigned int i;

    a2dVertexArray* cpoints = new a2dVertexArray;

    double x, y;
    bool move = false;
    int count = 0;
    bool nostrokeparts = false;

    //first draw as much as possible ( nostroke parts may stop this first round )
    for ( i = 0; i < path->size(); i++ )
    {
        a2dVpathSegment* seg = path->Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( count == 0 )
                {
                    lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                }
                else
                {
                    i--;
                    move = true;
                }
                break;

            case a2dPATHSEG_LINETO:
                lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                cpoints->push_back( new a2dLineSegment( x, y ) );
                count++;
                break;
            case a2dPATHSEG_LINETO_NOSTROKE:
                lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                cpoints->push_back( new a2dLineSegment( x, y ) );
                count++;
                nostrokeparts = true;
                break;

            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg;

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    lworld.TransformPoint( xw, yw, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg;

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                    lworld.TransformPoint( xw, yw, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( path->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    a2dGlobals->Aberration( phit, radius , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    for ( step = 0; step < segments + 1; step++ )
                    {
                        lworld.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        theta = theta + dphi;
                    }
                }
                else
                {
                    lworld.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                }
            }
            break;
        }

        if ( move )
        {
            WritePath( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width );
            move = false;
            cpoints->clear();
            count = 0;
        }
        else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
        {
            if ( nostrokeparts || seg->GetClose() == a2dPATHSEG_END_CLOSED_NOSTROKE )
            {
                WriteBoundary( a2dIDENTITY_MATRIX, cpoints, layer, datatype );
                nostrokeparts = true;
            }
            else
            {
                WriteBoundary( a2dIDENTITY_MATRIX, cpoints, layer, datatype );
            }

            move = false;
            cpoints->clear();
            count = 0;
        }
        else if ( i == path->size() - 1 ) //last segment?
        {
            WritePath( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width );
            cpoints->clear();
        }
    }

    if ( nostrokeparts )
    {
        move = false;
        count = 0;
        cpoints->clear();

        nostrokeparts = false;

        double lastmovex = 0;
        double lastmovey = 0;

        for ( i = 0; i < path->size(); i++ )
        {
            a2dVpathSegment* seg = path->Item( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    if ( count == 0 )
                    {
                        lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        lastmovex = x;
                        lastmovey = y;
                        count++;
                    }
                    else
                    {
                        i--;
                        move = true;
                    }
                    break;

                case a2dPATHSEG_LINETO:
                    lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    break;

                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                    if ( count == 0 )
                    {
                        lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                    }
                    else
                    {
                        i--;
                        nostrokeparts = true;
                    }
                    break;

                case a2dPATHSEG_CBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                        lworld.TransformPoint( xw, yw, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_QBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                        lworld.TransformPoint( xw, yw, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_ARCTO:
                {
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;

                    double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                    if ( cseg->CalcR( path->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    {
                        double dphi;
                        unsigned int segments = 20;
                        a2dGlobals->Aberration( phit, radius , dphi, segments );

                        double theta = beginrad;
                        unsigned int step;

                        double x, y;
                        for ( step = 0; step < segments + 1; step++ )
                        {
                            lworld.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                            cpoints->push_back( new a2dLineSegment( x, y ) );
                            count++;
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        double x, y;
                        lworld.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                    }
                }
                break;
            }

            if ( move || nostrokeparts )
            {
                WritePath( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width );
                move = false;
                nostrokeparts = false;
                cpoints->clear();
                count = 0;
            }
            else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
            {
                if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
                {
                    cpoints->push_back( new a2dLineSegment( lastmovex, lastmovey ) );
                    count++;
                }
                WritePath( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width );
                nostrokeparts = false;
                move = false;
                cpoints->clear();
                count = 0;
            }
            else if ( i == path->size() )
            {
                WritePath( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width );
                cpoints->clear();
            }
        }
    }

    delete cpoints;

}

// Path ---------------------------------------------------------------------
void a2dIOHandlerGDSOut::WritePath( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype, int pathtype, double width )
{
    WriteHeader( 0, 0, GDS_PATH );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_LAYER );

    WriteInt( m_mapping[layer]->GetLayer() );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_DATATYPE );

    WriteInt( datatype );

    if ( pathtype )
    {
        WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                     GDS_PATHTYPE );

        WriteInt( pathtype );
    }

    if ( width )
    {
        WriteHeader( sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                     GDS_WIDTH );
        WriteLong( Round( width /  m_userunits_out / m_scale_out ) );
    }

    long help;
    points->ConvertToLines(m_AberArcToPoly);
    unsigned int n;
    n = points->size();

    help = 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) * n;

    if ( help > 65530L )
    {
        wxLogWarning(  wxT( "%s" ), _( "to many points in Polyline" ) );
        help = 65530L;
        n = help / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) );
    }
    WriteHeader( help, GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER, GDS_XY );

    unsigned int i;
    for ( i = 0; i < n;  i++ )
    {
        double x, y;
        lworld.TransformPoint( points->Item( i )->m_x, points->Item( i )->m_y, x, y );
        WritePoint( x, y );
    }

    WriteHeader( 0, 0, GDS_ENDEL );
}

// Path ---------------------------------------------------------------------
void a2dIOHandlerGDSOut::WritePath( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, int pathtype, double width )
{
    WriteHeader( 0, 0, GDS_PATH );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_LAYER );

    WriteInt( m_mapping[layer]->GetLayer() );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_DATATYPE );

    WriteInt( datatype );

    if ( pathtype )
    {
        WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                     GDS_PATHTYPE );

        WriteInt( pathtype );
    }

    if ( width )
    {
        WriteHeader( sizeof( FOUR_G_BYTE_SIGNED_INTEGER ),
                     GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER,
                     GDS_WIDTH );
        WriteLong( Round( width /  m_userunits_out / m_scale_out ) );
    }

    points->ConvertToLines(m_AberArcToPoly);
    unsigned int n = points->size();

    long help = 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) * n;
    if ( help > 65530L )
    {
        wxLogWarning( wxT( "%s" ), _( "to many points in Polyline" ) );
        help = 65530L;
        n = help / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) );
    }
    WriteHeader( help, GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER, GDS_XY );

    a2dVertexList::iterator iter = points->begin();
    unsigned int i = 0;
    while ( i < n )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
        double x, y;
        lworld.TransformPoint( point.m_x, point.m_y, x, y );
        WritePoint( x, y );
        iter++;
        i++;
    }

    WriteHeader( 0, 0, GDS_ENDEL );
}

// Boundary------------------------------------------------------------------
void a2dIOHandlerGDSOut::WriteBoundary( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype )
{
    WriteHeader( 0, 0, GDS_BOUNDARY );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_LAYER );

    WriteInt( m_mapping[layer]->GetLayer() );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_DATATYPE );

    WriteInt( datatype );

    points->ConvertToLines(m_AberArcToPoly);
    long help;
    unsigned int n = points->size();

    help = 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) * ( n + 1 );

    if ( help > 65530L )
    {
        wxLogWarning( wxT( "%s" ), _( "to many points in Polyline" ) );
        //clip the number of point to write to the max possible.
        //TODO use boolean algorithms to divide big polygons.
        help = 65530L;
        n = help / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) ) - 1;
    }
    WriteHeader( help, GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER, GDS_XY );

    double x, y;
    unsigned int i;
    for ( i = 0; i < n;  i++ )
    {
        lworld.TransformPoint( points->Item( i )->m_x, points->Item( i )->m_y, x, y );
        WritePoint( x, y );
    }
    lworld.TransformPoint( points->Item( 0 )->m_x, points->Item( 0 )->m_y, x, y );
    WritePoint( x, y );

    WriteHeader( 0, 0, GDS_ENDEL );
}

// Boundary------------------------------------------------------------------
void a2dIOHandlerGDSOut::WriteBoundary( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, bool spline )
{
    a2dVertexList* rlist;

    if ( spline || points->HasArcs() )
    {
        a2dVertexList* lpoints = new a2dVertexList();
        *lpoints = *points;
        lpoints->ConvertSplinedPolygon( 10 );
        rlist = lpoints;
    }
    else
        rlist = points;

    WriteHeader( 0, 0, GDS_BOUNDARY );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_BIT_ARRAY,
                 GDS_ELFLAGS );

    WriteInt( m_objectFlags );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_LAYER );

    WriteInt( m_mapping[layer]->GetLayer() );

    WriteHeader( sizeof( TWO_G_BYTE_SIGNED_INTEGER ),
                 GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER,
                 GDS_DATATYPE );

    WriteInt( datatype );

    points->ConvertToLines(m_AberArcToPoly);
    unsigned int n = points->size();

    long help = 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) * ( n + 1 );
    if ( help > 65530L )
    {
        wxLogWarning( wxT( "%s" ), _( "to many points in Polyline" ) );
        help = 65530L;
        n = help / ( 2 * sizeof( FOUR_G_BYTE_SIGNED_INTEGER ) ) - 1;
    }
    WriteHeader( help, GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER, GDS_XY );

    a2dVertexList::iterator iter = points->begin();
    double x, y;
    unsigned int i = 0;
    while ( i < n )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
        lworld.TransformPoint( point.m_x, point.m_y, x, y );
        WritePoint( x, y );
        iter++;
        i++;
    }
    lworld.TransformPoint( points->front()->m_x, points->front()->m_y, x, y );
    WritePoint( x, y );

    WriteHeader( 0, 0, GDS_ENDEL );

    if ( spline || points->HasArcs() )
        delete rlist;
}

// -------------- output -----------------

// a byte
void a2dIOHandlerGDSOut::WriteByte( G_BYTE ch )
{
#if wxUSE_STD_IOSTREAM
    m_streamo->put( ch );
#else
    m_streamo->PutC( ch );
#endif
}

// 2 bytes UNsigned integer
void a2dIOHandlerGDSOut::WriteUnsignedInt( TWO_G_BYTE_UNSIGNED_INTEGER i )
{
    a_union_value a_value;
    a_value.two_byte_signed_integer = i;
#if wxBYTE_ORDER == wxBIG_ENDIAN
    WriteByte( a_value.array.array[0] );
    WriteByte( a_value.array.array[1] );
#else
    WriteByte( a_value.array.array[1] );
    WriteByte( a_value.array.array[0] );
#endif
}

// 2 bytes signed integer
void a2dIOHandlerGDSOut::WriteInt( TWO_G_BYTE_SIGNED_INTEGER i )
{
    a_union_value a_value;
    a_value.two_byte_signed_integer = i;
#if wxBYTE_ORDER == wxBIG_ENDIAN
    WriteByte( a_value.array.array[0] );
    WriteByte( a_value.array.array[1] );
#else
    WriteByte( a_value.array.array[1] );
    WriteByte( a_value.array.array[0] );
#endif
}


// 4 byte signed integer
void a2dIOHandlerGDSOut::WriteLong( FOUR_G_BYTE_SIGNED_INTEGER i )
{
    a_union_value a_value;
    a_value.four_byte_signed_integer = i;

    // writes a GdsII variable and convers to a dos-variable
    // the variable consist of 4 bits, numbered 0 1 2 3 , this
    // will be converted to the following sequence: 3 2 1 0

#if wxBYTE_ORDER == wxBIG_ENDIAN
    int t;
    for( t = 0; t <= 3; t++ )
#else
    int t;
    for( t = 3; t >= 0; t-- )
#endif
        WriteByte( a_value.array.array[t] );
}


// 4 byte byte real
void a2dIOHandlerGDSOut::WriteFloat( FOUR_G_BYTE_REAL r )
{
    a_union_value a_value;
    a_value.four_byte_real = r;

    // writes a GdsII variable and convers to a dos-variable
    // the variable consist of 4 bits, numbered 0 1 2 3 , this
    // will be converted to the following sequence: 3 2 1 0
#if wxBYTE_ORDER == wxBIG_ENDIAN
    for( short int i = 0; i <= 3; i++ )
#else
    for( short int i = 3; i >= 0; i-- )
#endif
        WriteByte( a_value.array.array[i] );
}


// 8 byte real
void a2dIOHandlerGDSOut::WriteDouble( EIGHT_G_BYTE_REAL r )
{
    a_union_value a_value;
    a_value.eight_byte_real = r;

    short int   i;

    // convers the machine double into GdsII-real
    double2gds( a_value.array.array );

    // writes a GdsII variable and convers to a dos-variable
    // Reverse ordering will happen in the function gds2double
    for( i = 0; i < 8; i++ )
        WriteByte( a_value.array.array[i] );

}


#endif //wxART2D_USE_GDSIO
