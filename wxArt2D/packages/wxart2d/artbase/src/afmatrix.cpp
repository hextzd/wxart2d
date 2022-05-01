/*! \file artbase/src/afmatrix.cpp
    \brief a2dAffineMatrix class
    \author Klaas Holwerda Chris Breeze, Julian Smart

    Copyright: 2001-2004 (C) Klaas Holwerda Chris Breeze, Julian Smart

    Licence: wxWidgets licence

    RCS-ID: $Id: afmatrix.cpp,v 1.42 2009/09/10 17:04:08 titato Exp $
*/

#include "a2dprec.h"
#include "wx/general/genmod.h"

#if wxART2D_USE_CVGIO
#include "wx/xmlparse/genxmlpars.h"
#endif //wxART2D_USE_CVGIO

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/afmatrix.h"
#include <math.h>
#include <ctype.h>

#if defined(__VISUALC__)

// MSVC warning 4660 is quite stupid. It says that the template is already instantiated
// by using it, but it is not fully instantiated as required for a library
#ifdef _MSC_VER
#pragma warning(disable: 4660)
#endif

template a2dPropertyIdTyped<a2dPoint2D, a2dPoint2DProperty>;
template a2dPropertyIdTyped<a2dAffineMatrix, a2dMatrixProperty>;

#ifdef _MSC_VER
#pragma warning(default: 4660)
#endif

#endif

IMPLEMENT_DYNAMIC_CLASS( a2dMatrixProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dPoint2DProperty, a2dNamedProperty )

a2dAffineMatrix a2dIDENTITY_MATRIX;

a2dAffineMatrix::a2dAffineMatrix( double xt, double yt, double scalex, double scaley, double degrees )
{
    if ( degrees )
    {
        double angle = wxDegToRad( -degrees );
        double c = cos( angle );
        double s = sin( angle );

        m_matrix[0][0] = c * scalex;
        m_matrix[1][0] = -s * scaley;
        m_matrix[2][0] = xt;
        m_matrix[0][1] = s * scalex;
        m_matrix[1][1] = c * scaley;
        m_matrix[2][1] = yt;

    }
    else
    {
        m_matrix[0][0] = scalex;
        m_matrix[1][0] = 0;
        m_matrix[2][0] = xt;
        m_matrix[0][1] = 0;
        m_matrix[1][1] = scaley;
        m_matrix[2][1] = yt;
    }

    m_isIdentity = IsIdentity1();
}

a2dAffineMatrix::a2dAffineMatrix( double sx, double shx, double shy, double sy, double tx, double ty )
{
    m_matrix[0][0] = sx;
    m_matrix[1][0] = shy;
    m_matrix[2][0] = tx;
    m_matrix[0][1] = shx;
    m_matrix[1][1] = sy;
    m_matrix[2][1] = ty;
    m_isIdentity = IsIdentity1();
}

a2dAffineMatrix::a2dAffineMatrix( void )
{
    Identity();
}

a2dAffineMatrix::a2dAffineMatrix( const a2dAffineMatrix& mat )
{
    memcpy( m_matrix, mat.m_matrix, 6 * sizeof( double ) );
    m_isIdentity = mat.m_isIdentity;

    //(*this) = mat;
}

double a2dAffineMatrix::GetValue( int col, int row ) const
{
    if ( row < 0 || row > 1 || col < 0 || col > 2 )
        return 0.0;

    return m_matrix[col][row];
}

void a2dAffineMatrix::SetValue( int col, int row, double value )
{
    if ( row < 0 || row > 1 || col < 0 || col > 2 )
        return;

    m_matrix[col][row] = value;
    m_isIdentity = IsIdentity1();
}

void a2dAffineMatrix::operator = ( const a2dAffineMatrix& mat )
{
    /*
        int i, j;
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 2; j++)
            {
                m_matrix[i][j] = mat.m_matrix[i][j];
            }
        }
    */
    memcpy( m_matrix, mat.m_matrix, 6 * sizeof( double ) );

    m_isIdentity = mat.m_isIdentity;
}

A2DARTBASEDLLEXP bool operator == ( const a2dAffineMatrix& a, const a2dAffineMatrix& b )
{
    if ( a.m_isIdentity && b.m_isIdentity )
        return true;

    if ( memcmp( a.m_matrix, b.m_matrix, sizeof( a.m_matrix ) ) )
        return false;

    return true;
}

A2DARTBASEDLLEXP bool operator != ( const a2dAffineMatrix& a, const a2dAffineMatrix& b )
{
    return ( ! ( a == b ) );
}


bool a2dAffineMatrix::operator== ( const a2dAffineMatrix& a )
{
    if ( m_isIdentity && a.m_isIdentity )
        return true;

    if ( memcmp( m_matrix, a.m_matrix, sizeof( m_matrix ) ) )
        return false;

    return true;
}

bool a2dAffineMatrix::operator!= ( const a2dAffineMatrix& a )
{
    return ( ! ( *this == a ) );
}


double& a2dAffineMatrix::operator()( int col, int row )
{
    if ( row < 0 || row > 1 || col < 0 || col > 2 )
        return m_matrix[0][0];

    return m_matrix[col][row];
}

double a2dAffineMatrix::operator()( int col, int row ) const
{
    if ( row < 0 || row > 1 || col < 0 || col > 2 )
        return 0.0;

    return m_matrix[col][row];
}

//! Calculates the determinant of a 2 x 2 matrix
inline double wxCalculateDet( double a11, double a21, double a12, double a22 )
{
    return a11 * a22 - a12 * a21;
}

// Invert matrix
bool a2dAffineMatrix::Invert( void )
{
    if ( m_isIdentity )
        return true;

    double inverseMatrix[3][2];
    double det = m_matrix[0][0] * m_matrix[1][1] - m_matrix[0][1] * m_matrix[1][0];

    if( det )
    {
        // calculate the adjoint
        inverseMatrix[0][0] =  m_matrix[1][1];
        inverseMatrix[0][1] = -m_matrix[0][1];


        inverseMatrix[1][0] = -m_matrix[1][0];
        inverseMatrix[1][1] =  m_matrix[0][0];

        inverseMatrix[2][0] =  m_matrix[1][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[1][1];
        inverseMatrix[2][1] = -( m_matrix[0][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[0][1] );

        inverseMatrix[0][0] /= det; inverseMatrix[1][0] /= det; inverseMatrix[2][0] /= det;
        inverseMatrix[0][1] /= det; inverseMatrix[1][1] /= det; inverseMatrix[2][1] /= det;

        int i, j;
        for ( i = 0; i < 3; i++ )
        {
            for ( j = 0; j < 2; j++ )
            {
                m_matrix[i][j] = inverseMatrix[i][j];
            }
        }

        m_isIdentity = IsIdentity1();
        return true;
    }
    else if( m_matrix[0][0] && !m_matrix[1][0] && !m_matrix[0][1] && !m_matrix[1][1] )
    {
        // We cannot realy invert this, but it is a 1D degenerated case, that can be 1D inverted
        m_matrix[0][0] = 1 / m_matrix[0][0];
        m_matrix[2][0] = -m_matrix[0][0] * m_matrix[2][0];
        m_matrix[2][1] = -m_matrix[2][1];
        return true;
    }
    else if( !m_matrix[0][0] && !m_matrix[1][0] && !m_matrix[0][1] && m_matrix[1][1] )
    {
        // We cannot realy invert this, but it is a 1D degenerated case, that can be 1D inverted
        m_matrix[1][1] = 1 / m_matrix[1][1];
        m_matrix[2][1] = -m_matrix[1][1] * m_matrix[2][1];
        m_matrix[2][0] = -m_matrix[2][0];
        return true;
    }
    else
    {
        return false;
    }
}

// Make into identity matrix
bool a2dAffineMatrix::Identity( void )
{
    m_matrix[0][0] = m_matrix[1][1] = 1.0;
    m_matrix[1][0] = m_matrix[2][0] = m_matrix[0][1] = m_matrix[2][1] = 0.0;
    m_isIdentity = true;

    return true;
}

// Scale by scale (isotropic scaling i.e. the same in x and y):
//           | scale  0      0      |
// matrix' = |  0     scale  0      | x matrix
//           |  0     0      scale  |
//
bool a2dAffineMatrix::Scale( double scale )
{
    int i, j;
    for ( i = 0; i < 3; i++ )
    {
        for ( j = 0; j < 2; j++ )
        {
            m_matrix[i][j] *= scale;
        }
    }
    m_isIdentity = IsIdentity1();

    return true;
}

// Remove Scale:
//           | 1/sqrt(det) 0           0      |
// matrix' = | 0           1/sqrt(det) 0      | x matrix
//           | 0           0           1      |
//
bool a2dAffineMatrix::RemoveScale()
{
    if ( m_isIdentity )
        return true;
    double scale = sqrt( fabs( m_matrix[0][0] * m_matrix[1][1] - m_matrix[0][1] * m_matrix[1][0] ) );
    if( scale )
    {
        int i, j;
        for ( i = 0; i < 2; i++ )
        {
            for ( j = 0; j < 2; j++ )
            {
                m_matrix[i][j] /= scale;
            }
        }
        for ( j = 0; j < 2; j++ )
        {
            double scaleJ = sqrt( m_matrix[0][j] * m_matrix[0][j] + m_matrix[1][j] * m_matrix[1][j] );
            wxASSERT_MSG( scaleJ != 0, wxT( "matrix scaleJ == 0 !" ) );
            m_matrix[0][j] /= scaleJ;
            m_matrix[1][j] /= scaleJ;
        }
        m_isIdentity = IsIdentity1();

        return true;
    }
    wxASSERT_MSG( scale != 0, wxT( "matrix determinant == 0 !" ) );
    return false;
}


// scale a matrix in 2D
//
//     xs    0      xc(1-xs)
//     0    ys      yc(1-ys)
//     0     0      1
//
a2dAffineMatrix&  a2dAffineMatrix::Scale( const double& xs, const double& ys, const double& xc, const double& yc )
{
    double r00, r10, r20, r01, r11, r21;

    if ( m_isIdentity )
    {
        double tx  = xc * ( 1 - xs );
        double ty  = yc * ( 1 - ys );
        r00 = xs;
        r10 = 0;
        r20 = tx;
        r01 = 0;
        r11 = ys;
        r21 = ty;
    }
    else if ( xc != 0 || yc != 0 )
    {
        double tx  = xc * ( 1 - xs );
        double ty  = yc * ( 1 - ys );
        r00 = xs * m_matrix[0][0];
        r10 = xs * m_matrix[1][0];
        r20 = xs * m_matrix[2][0] + tx;
        r01 = ys * m_matrix[0][1];
        r11 = ys * m_matrix[1][1];
        r21 = ys * m_matrix[2][1] + ty;
    }
    else
    {
        r00 = xs * m_matrix[0][0];
        r10 = xs * m_matrix[1][0];
        r20 = xs * m_matrix[2][0];
        r01 = ys * m_matrix[0][1];
        r11 = ys * m_matrix[1][1];
        r21 = ys * m_matrix[2][1];
    }

    m_matrix[0][0] = r00;
    m_matrix[1][0] = r10;
    m_matrix[2][0] = r20;
    m_matrix[0][1] = r01;
    m_matrix[1][1] = r11;
    m_matrix[2][1] = r21;

    /* or like this
        // first translate to origin O
        (*this).Translate(-x_cen, -y_cen);

        // now do the scaling
        a2dAffineMatrix scale;
        scale.m_matrix[0][0] = x_fac;
        scale.m_matrix[1][1] = y_fac;
       scale.m_isIdentity = IsIdentity1();

        *this = scale * (*this);

        // translate back from origin to x_cen, y_cen
        (*this).Translate(x_cen, y_cen);
    */

    m_isIdentity = IsIdentity1();

    return *this;
}


// mirror a matrix in x, y
//
//     -1      0      0     X-mirror
//      0     -1      0     Y-mirror
a2dAffineMatrix&  a2dAffineMatrix::Mirror( bool y, bool x )
{
    a2dAffineMatrix temp;
    if ( x )
    {
        temp.m_matrix[0][0] = -1;
        temp.m_isIdentity = false;
    }
    if ( y )
    {
        temp.m_matrix[1][1] = -1;
        temp.m_isIdentity = false;
    }

    *this = temp * ( *this );
    m_isIdentity = IsIdentity1();
    return *this;
}

// Translate by dx, dy:
//           | 1  0 dx |
// matrix' = | 0  1 dy | x matrix
//           | 0  0  1 |
//
bool a2dAffineMatrix::Translate( double dx, double dy )
{
    m_matrix[2][0] += dx;
    m_matrix[2][1] += dy;
    m_isIdentity = IsIdentity1();
    return true;
}

// Rotate clockwise by the given number of degrees:
//           |  cos sin 0 |
// matrix' = | -sin cos 0 | x matrix
//           |   0   0  1 |
bool a2dAffineMatrix::Rotate( double degrees )
{
    if ( degrees == 0 )
        return true;

    Rotate( -degrees, 0, 0 );
    return true;
}

bool a2dAffineMatrix::SkewX( double degrees )
{
    if ( degrees == 0 )
        return true;

    double angle = degrees * wxPI / 180.0;
    double t = tan( angle );

    a2dAffineMatrix skew;

    skew.m_matrix[1][0] = t;
    skew.m_isIdentity = false;

    *this = skew * ( *this );

    m_isIdentity = IsIdentity1();
    return true;
}

bool a2dAffineMatrix::SkewY( double degrees )
{
    if ( degrees == 0 )
        return true;

    double angle = degrees * wxPI / 180.0;
    double t = tan( angle );

    a2dAffineMatrix skew;

    skew.m_matrix[0][1] = t;
    skew.m_isIdentity = false;

    *this = skew * ( *this );

    m_isIdentity = IsIdentity1();
    return true;
}

// counter clockwise rotate around a point
//
//  cos(r) -sin(r)    x(1-cos(r))+y(sin(r)
//  sin(r)  cos(r)    y(1-cos(r))-x(sin(r)
//    0      0        1
a2dAffineMatrix&  a2dAffineMatrix::Rotate( const double& degrees, const double& x, const double& y )
{
    if ( degrees == 0 )
        return * this;

    double angle = wxDegToRad( degrees );
    double c = cos( angle );
    double s = sin( angle );
    double r00, r10, r20, r01, r11, r21;

    if ( m_isIdentity )
    {
        double tx  = x * ( 1 - c ) + y * s;
        double ty  = y * ( 1 - c ) - x * s;
        r00 = c ;
        r10 = -s;
        r20 = tx;
        r01 = s;
        r11 = c;
        r21 = ty;
    }
    else if ( x != 0 || y != 0 )
    {
        double tx  = x * ( 1 - c ) + y * s;
        double ty  = y * ( 1 - c ) - x * s;
        r00 = c * m_matrix[0][0] - s * m_matrix[0][1];
        r10 = c * m_matrix[1][0] - s * m_matrix[1][1];
        r20 = c * m_matrix[2][0] - s * m_matrix[2][1] + tx;
        r01 = c * m_matrix[0][1] + s * m_matrix[0][0];
        r11 = c * m_matrix[1][1] + s * m_matrix[1][0];
        r21 = c * m_matrix[2][1] + s * m_matrix[2][0] + ty;
    }
    else
    {
        r00 = c * m_matrix[0][0] - s * m_matrix[0][1];
        r10 = c * m_matrix[1][0] - s * m_matrix[1][1];
        r20 = c * m_matrix[2][0] - s * m_matrix[2][1];
        r01 = c * m_matrix[0][1] + s * m_matrix[0][0];
        r11 = c * m_matrix[1][1] + s * m_matrix[1][0];
        r21 = c * m_matrix[2][1] + s * m_matrix[2][0];
    }

    m_matrix[0][0] = r00;
    m_matrix[1][0] = r10;
    m_matrix[2][0] = r20;
    m_matrix[0][1] = r01;
    m_matrix[1][1] = r11;
    m_matrix[2][1] = r21;

    /* or like this
        double angle = degrees * pi / 180.0;
        double c = cos(angle);
        double s = sin(angle);
        double tx  = x*(1-c)+y*s;
        double ty  = y*(1-c)-x*s;

        a2dAffineMatrix rotate;
        rotate.m_matrix[2][0] = tx;
        rotate.m_matrix[2][1] = ty;

        rotate.m_matrix[0][0] = c;
        rotate.m_matrix[0][1] = s;

        rotate.m_matrix[1][0] = -s;
        rotate.m_matrix[1][1] = c;

       rotate.m_isIdentity=false;
       *this = rotate * (*this);
    */
    m_isIdentity = IsIdentity1();

    return *this;
}

// Transform a point from logical to device coordinates
void a2dAffineMatrix::TransformPoint( double x, double y, double& tx, double& ty ) const
{
    if ( m_isIdentity )
    {
        tx = x; ty = y; return;
    }

    tx = x * m_matrix[0][0] + y * m_matrix[1][0] + m_matrix[2][0];
    ty = x * m_matrix[0][1] + y * m_matrix[1][1] + m_matrix[2][1];
}

// Transform a point from logical to device coordinates
void a2dAffineMatrix::TransformPoint( double& tx, double& ty ) const
{
    if ( m_isIdentity )
        return;

    tx = tx * m_matrix[0][0] + ty * m_matrix[1][0] + m_matrix[2][0];
    ty = tx * m_matrix[0][1] + ty * m_matrix[1][1] + m_matrix[2][1];
}

void a2dAffineMatrix::TransformPoint( a2dPoint2D* point ) const
{
    if ( m_isIdentity )
        return;

    double x = point->m_x * m_matrix[0][0] + point->m_y * m_matrix[1][0] + m_matrix[2][0];
    point->m_y = point->m_x * m_matrix[0][1] + point->m_y * m_matrix[1][1] + m_matrix[2][1];
    point->m_x = x;
}

void a2dAffineMatrix::TransformPoint( const a2dPoint2D& src, a2dPoint2D* dest ) const
{
    if ( m_isIdentity )
    {
        *dest = src;
        return;
    }

    dest->m_x = src.m_x * m_matrix[0][0] + src.m_y * m_matrix[1][0] + m_matrix[2][0];
    dest->m_y = src.m_x * m_matrix[0][1] + src.m_y * m_matrix[1][1] + m_matrix[2][1];
}

void a2dAffineMatrix::TransformVector( double dx, double dy, double& tdx, double& tdy ) const
{
    if ( m_isIdentity )
    {
        tdx = dx; tdy = dy; return;
    }
    // consider two points (px,py) and (px+dx,py+dy), transform both and subtract
    // to get the new vector.  In 2D homogeneous coords, also equiv to
    // mtx * [dx,dy,0], instead of mtx * [x,y,1] for a point.

    tdx = dx * m_matrix[0][0] + dy * m_matrix[1][0];
    tdy = dx * m_matrix[0][1] + dy * m_matrix[1][1];
}

double a2dAffineMatrix::TransformDistance( double distance ) const
{
    if ( m_isIdentity )
        return distance;
    /*
        fill in (0,0) for one point, and (0,distance) or (distance,0) for the second

        tx = x * m_matrix[0][0] + y * m_matrix[1][0] + m_matrix[2][0];
        ty = x * m_matrix[0][1] + y * m_matrix[1][1] + m_matrix[2][1];

        and you get

        xt = dx * m_matrix[0][0] + dy * m_matrix[1][0];
        yt = dx * m_matrix[0][1] + dy * m_matrix[1][1];
    */

    return wxMin( sqrt( m_matrix[0][0] * m_matrix[0][0] + m_matrix[0][1] * m_matrix[0][1] ) * distance,
                  sqrt( m_matrix[1][0] * m_matrix[1][0] + m_matrix[1][1] * m_matrix[1][1] ) * distance );
}

void a2dAffineMatrix::TransformDistance( double& dx, double& dy ) const
{
    /*
        fill in (0,0) for one point, and (dx,dy) for the second

        tx = x * m_matrix[0][0] + y * m_matrix[1][0] + m_matrix[2][0];
        ty = x * m_matrix[0][1] + y * m_matrix[1][1] + m_matrix[2][1];

        and you get

        xt = dx * m_matrix[0][0] + dy * m_matrix[1][0];
        yt = dy * m_matrix[0][1] + dy * m_matrix[1][1];
    */
    double xt = dx * m_matrix[0][0] + dy * m_matrix[1][0];
    double yt = dx * m_matrix[0][1] + dy * m_matrix[1][1];
    dx = xt;
    dy = yt;
}

a2dAffineMatrix& a2dAffineMatrix::operator*=( const a2dAffineMatrix& mat )
{
    if ( mat.m_isIdentity )
        return * this;
    if ( m_isIdentity )
    {
        *this = mat;
        return *this;
    }
    else
    {
        a2dAffineMatrix  result;
        for ( int i = 0; i < 2; i++ )
        {
            double sum;

            sum = m_matrix[0][i] * mat.m_matrix[0][0];
            sum += m_matrix[1][i] * mat.m_matrix[0][1];
            result.m_matrix[0][i] = sum;

            sum = m_matrix[0][i] * mat.m_matrix[1][0];
            sum += m_matrix[1][i] * mat.m_matrix[1][1];
            result.m_matrix[1][i] = sum;

            sum = m_matrix[0][i] * mat.m_matrix[2][0];
            sum += m_matrix[1][i] * mat.m_matrix[2][1];
            sum += m_matrix[2][i];
            result.m_matrix[2][i] = sum;
        }
        *this = result;
    }

    m_isIdentity = IsIdentity1();
    return *this;

}

a2dAffineMatrix  a2dAffineMatrix::operator*( const a2dAffineMatrix& mat ) const
{
    if ( mat.m_isIdentity )
        return * this;
    if ( m_isIdentity )
    {
        return mat;
    }
    else
    {
        a2dAffineMatrix  result;
        for ( int i = 0; i < 2; i++ )
        {
            double sum;

            sum = m_matrix[0][i] * mat.m_matrix[0][0];
            sum += m_matrix[1][i] * mat.m_matrix[0][1];
            result.m_matrix[0][i] = sum;

            sum = m_matrix[0][i] * mat.m_matrix[1][0];
            sum += m_matrix[1][i] * mat.m_matrix[1][1];
            result.m_matrix[1][i] = sum;

            sum = m_matrix[0][i] * mat.m_matrix[2][0];
            sum += m_matrix[1][i] * mat.m_matrix[2][1];
            sum += m_matrix[2][i];
            result.m_matrix[2][i] = sum;
        }
        result.m_isIdentity = result.IsIdentity1();
        return result;
    }
}

//!  matrixes subtract
a2dAffineMatrix a2dAffineMatrix::operator-( const a2dAffineMatrix& b )
{
    a2dAffineMatrix
    ret;
    ret.m_matrix[0][0] = m_matrix[0][0] - b.m_matrix[0][0];
    ret.m_matrix[0][1] = m_matrix[0][1] - b.m_matrix[0][1];
    ret.m_matrix[1][0] = m_matrix[1][0] - b.m_matrix[1][0];
    ret.m_matrix[1][1] = m_matrix[1][1] - b.m_matrix[1][1];
    ret.m_matrix[2][0] = m_matrix[2][0] - b.m_matrix[2][0];
    ret.m_matrix[2][1] = m_matrix[2][1] - b.m_matrix[2][1];

    return ret;
}

//!  matrixes subtract
A2DARTBASEDLLEXP a2dAffineMatrix operator-( const a2dAffineMatrix& a, const a2dAffineMatrix& b )
{
    a2dAffineMatrix
    ret;
    ret.m_matrix[0][0] = a.m_matrix[0][0] - b.m_matrix[0][0];
    ret.m_matrix[0][1] = a.m_matrix[0][1] - b.m_matrix[0][1];
    ret.m_matrix[1][0] = a.m_matrix[1][0] - b.m_matrix[1][0];
    ret.m_matrix[1][1] = a.m_matrix[1][1] - b.m_matrix[1][1];
    ret.m_matrix[2][0] = a.m_matrix[2][0] - b.m_matrix[2][0];
    ret.m_matrix[2][1] = a.m_matrix[2][1] - b.m_matrix[2][1];

    return ret;
}

double a2dAffineMatrix::Get_scaleX() const
{
    if ( IsIdentity() )
        return 1;

    double rot_angle = atan2( m_matrix[0][1], m_matrix[0][0] ) * 180 / wxPI;
    a2dAffineMatrix inverse;
    inverse.Rotate( -rot_angle, m_matrix[2][0], m_matrix[2][1] );

    a2dAffineMatrix h = *this;
    h *= inverse;


    double scale_factor;
    scale_factor = h.GetValue( 0, 0 );

    if ( scale_factor < 0 )
        scale_factor = -scale_factor;

    return scale_factor;
}

double a2dAffineMatrix::Get_scaleY() const
{
    if ( IsIdentity() )
        return 1;

    double rot_angle = atan2( m_matrix[0][1], m_matrix[0][0] ) * 180 / wxPI;
    a2dAffineMatrix inverse;
    inverse.Rotate( -rot_angle, m_matrix[2][0], m_matrix[2][1] );

    a2dAffineMatrix h = *this;
    h *= inverse;


    double scale_factor;
    scale_factor = h.GetValue( 1, 1 );

    if ( scale_factor < 0 )
        scale_factor = -scale_factor;

    return scale_factor;
}

double a2dAffineMatrix::GetRotation() const
{
    if ( IsIdentity() )
        return 0.0;

    double temp1 = m_matrix[0][0];   // for angle calculation
    double temp2 = m_matrix[0][1];   //

    // Rotation
    double rot_angle = atan2( temp2, temp1 ) * 180 / wxPI;
    if ( rot_angle == -180.0 )
        rot_angle = 180.0;

    return rot_angle;
}

void a2dAffineMatrix::SetRotation( double rotation )
{
    double x = m_matrix[2][0];
    double y = m_matrix[2][1];
    if ( !IsIdentity() )
        Rotate( -GetRotation(), x, y );

    Rotate( rotation, x, y );
}

double a2dAffineMatrix::GetDeterminant() const
{
    return m_matrix[0][0] * m_matrix[1][1] - m_matrix[1][0] * m_matrix[0][1];
}

double a2dAffineMatrix::GetDeterminant1() const
{
    return m_matrix[2][0] * m_matrix[1][1] - m_matrix[1][0] * m_matrix[2][1];
}

double a2dAffineMatrix::GetDeterminant2() const
{
    return m_matrix[0][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[0][1];
}

double a2dAffineMatrix::GetMaximum() const
{
    return
        wxMax(
            wxMax( fabs( m_matrix[0][0] ), fabs( m_matrix[1][1] ) ),
            wxMax( fabs( m_matrix[1][0] ), fabs( m_matrix[0][1] ) )
        );
}

double a2dAffineMatrix::GetMaximum1() const
{
    return
        wxMax(
            wxMax( fabs( m_matrix[2][0] ), fabs( m_matrix[1][1] ) ),
            wxMax( fabs( m_matrix[1][0] ), fabs( m_matrix[2][1] ) )
        );
}

double a2dAffineMatrix::GetMaximum2() const
{
    return
        wxMax(
            wxMax( fabs( m_matrix[0][0] ), fabs( m_matrix[2][1] ) ),
            wxMax( fabs( m_matrix[2][0] ), fabs( m_matrix[0][1] ) )
        );
}

void a2dAffineMatrix::DebugDump( const wxChar* text, double w, double h ) const
{
    wxLogDebug( text );
    wxLogDebug( wxT( "a00=%12.6lf, a01=%12.6lf" ), m_matrix[0][0], m_matrix[0][1] );
    wxLogDebug( wxT( "a10=%12.6lf, a11=%12.6lf" ), m_matrix[1][0], m_matrix[1][1] );
    wxLogDebug( wxT( "a20=%12.6lf. a21=%12.6lf" ), m_matrix[2][0], m_matrix[2][1] );
    wxLogDebug( wxT( "w  =%12.6lf, h  =%12.6lf" ), w, h );
    wxLogDebug( wxT( "cx =%12.6lf, cy =%12.6lf" ),
                0.5 * ( m_matrix[0][0] * w + m_matrix[1][0] * h ) + m_matrix[2][0],
                0.5 * ( m_matrix[0][1] * w + m_matrix[1][1] * h ) + m_matrix[2][1]
              );
    wxLogDebug( wxT( "lx =%12.6lf, ly =%12.6lf" ),
                w * sqrt( m_matrix[0][0] * m_matrix[0][0] + m_matrix[0][1] * m_matrix[0][1] ),
                h * sqrt( m_matrix[1][0] * m_matrix[1][0] + m_matrix[1][1] * m_matrix[1][1] )
              );
    wxLogDebug( wxT( "\n" ) );
}

// The default constructor makes it an identity matrix
const a2dAffineMatrix a2dAffineMatrix::sm_Identity;


//***************************** functies used global ********************************
A2DARTBASEDLLEXP bool ParseCvgTransForm( a2dAffineMatrix& matrix, const wxString& str, wxString& error )
{
    if ( str == wxT( "" ) )
        return false;

    wxString numstr;
    double matrixcoef[12];
    int nr_matrixcoef;

    wxString keywstr;
    size_t i;
    for ( i = 0; i < str.Len(); i++ )
    {
        //skip space
        while ( wxIsspace( str[i] ) ) i++;

        nr_matrixcoef = 0;
        keywstr.Clear();
        //transform keyword
        while ( i < str.Len() &&  isalpha ( str[i] ) )
        {
            keywstr += str.GetChar( i );
            i++;
        }

        //skip space
        while ( wxIsspace( str[i] ) ) i++;

        if ( str[i] != wxT( '(' ) )
        {
            error = _( "CVG parsing error: missing" );
            return false;
        }
        i++;

        while ( wxIsspace( str[i] ) ) i++;

        while ( i < str.Len() &&  str[i] != wxT( ')' ) )
        {
            numstr.Clear();
            while ( i < str.Len() && ( isdigit( str[i] ) || str[i] == wxT( '+' ) || str[i] == wxT( '-' ) || str[i] == wxT( '.' )  || str[i] == wxT( 'e' )  || str[i] == wxT( 'E' ) )  )
            {
                numstr += str.GetChar( i );
                i++;
            }
            numstr.ToCDouble( &matrixcoef[nr_matrixcoef] );
            nr_matrixcoef++;
            while ( i < str.Len() && ( wxIsspace( str[i] ) || str[i] == wxT( ',' ) ) ) i++;
        }

        if ( str[i] != wxT( ')' ) )
        {
            error = _( "CVG parsing error: missing" );
            return false;
        }
        i++;

        if ( keywstr == wxT( "matrix" ) )
        {
            if ( nr_matrixcoef != 6 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            matrix.SetValue( 0, 0, matrixcoef[0] );
            matrix.SetValue( 0, 1, matrixcoef[1] );
            matrix.SetValue( 1, 0, matrixcoef[2] );
            matrix.SetValue( 1, 1, matrixcoef[3] );
            matrix.SetValue( 2, 0, matrixcoef[4] );
            matrix.SetValue( 2, 1, matrixcoef[5] );
        }
        else if ( keywstr == wxT( "translate" ) )
        {
            if ( nr_matrixcoef == 1 )
                matrix.Translate( matrixcoef[0] , 0 );
            else if ( nr_matrixcoef != 2 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            else
                matrix.Translate( matrixcoef[0] , matrixcoef[1] );
        }
        else if ( keywstr == wxT( "scale" ) )
        {
            if ( nr_matrixcoef == 1 )
                matrix.Scale( matrixcoef[0], 1.0, matrix.GetValue( 2, 0 ), matrix.GetValue( 2, 1 ) );
            else if ( nr_matrixcoef != 2 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            else
                matrix.Scale( matrixcoef[0], matrixcoef[1], matrix.GetValue( 2, 0 ), matrix.GetValue( 2, 1 ) );
        }
        else if ( keywstr == wxT( "rotate" ) )
        {
            if ( nr_matrixcoef != 1 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            matrix.Rotate( matrixcoef[0] );
        }
        else if ( keywstr == wxT( "skewX" ) )
        {
            if ( nr_matrixcoef != 1 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            matrix.SkewX( matrixcoef[0] );
        }
        else if ( keywstr == wxT( "skewY" ) )
        {
            if ( nr_matrixcoef != 1 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            matrix.SkewY( matrixcoef[0] );
        }
        else if ( keywstr == wxT( "flipX" ) )
        {
            if ( nr_matrixcoef != 0 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            matrix.Mirror( true, false );
        }
        else if ( keywstr == wxT( "flipY" ) )
        {
            if ( nr_matrixcoef != 0 )
            {
                error.Printf( _( "CVG : wrong number of arguments %s " ), keywstr.c_str() );
                return false;
            }
            matrix.Mirror( false, true );
        }
        else
        {
            error.Printf( _( "CVG : invalid transform %s " ), keywstr.c_str() );
            return false;
        }
    }
    return true;
}

//----------------------------------------------------------------------------
// a2dMatrixProperty
//----------------------------------------------------------------------------

a2dMatrixProperty::a2dMatrixProperty(): a2dNamedProperty()
{}

a2dMatrixProperty::a2dMatrixProperty( const a2dPropertyIdMatrix* id, const a2dAffineMatrix& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dMatrixProperty::a2dMatrixProperty( const a2dMatrixProperty* ori )
    : a2dNamedProperty( *ori )
{
    m_value = ori->m_value;
}

a2dMatrixProperty::~a2dMatrixProperty()
{}

a2dMatrixProperty::a2dMatrixProperty( const a2dMatrixProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dMatrixProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dMatrixProperty( *this );
};

void a2dMatrixProperty::Assign( const a2dNamedProperty& other )
{
    a2dMatrixProperty* propcast = wxStaticCast( &other, a2dMatrixProperty );
    m_value = propcast->m_value;
}

#if wxART2D_USE_CVGIO
void a2dMatrixProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString s;
        s.Printf ( _T( "matrix( %s %s %s %s %s %s )" ),
                   wxString::FromCDouble( m_value.GetValue( 0, 0 ) ), wxString::FromCDouble( m_value.GetValue( 0, 1 ) ),
                   wxString::FromCDouble( m_value.GetValue( 1, 0 ) ), wxString::FromCDouble( m_value.GetValue( 1, 1 ) ),
                   wxString::FromCDouble( m_value.GetValue( 2, 0 ) ), wxString::FromCDouble( m_value.GetValue( 2, 1 ) )
                 );
        out.WriteAttribute( wxT( "transform" ), s );
    }
    else
    {}
}

void a2dMatrixProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString str = parser.GetAttributeValue( wxT( "transform" ) );
        wxString error;
        if ( !str.IsEmpty() && !::ParseCvgTransForm( m_value, str, error ) )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid transform %s at line %d" ), str.c_str(), parser.GetCurrentLineNumber() );
        }
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

void a2dMatrixProperty::SetValue( const a2dAffineMatrix& value )
{
    m_value = value;
}

wxString a2dMatrixProperty::StringRepresentation() const
{
    wxString s;
    s.Printf ( wxT( "%s = matrix( %s %s %s %s %s %s )" ), m_id->GetName().c_str(),
               wxString::FromCDouble( m_value.GetValue( 0, 0 ) ), wxString::FromCDouble( m_value.GetValue( 0, 1 ) ),
               wxString::FromCDouble( m_value.GetValue( 1, 0 ) ), wxString::FromCDouble( m_value.GetValue( 1, 1 ) ),
               wxString::FromCDouble( m_value.GetValue( 2, 0 ) ), wxString::FromCDouble( m_value.GetValue( 2, 1 ) )
             );

    return s;
}

wxString a2dMatrixProperty::StringValueRepresentation() const
{
    wxString s;
    s.Printf ( wxT( "matrix( %s %s %s %s %s %s )" ),
               wxString::FromCDouble( m_value.GetValue( 0, 0 ) ), wxString::FromCDouble( m_value.GetValue( 0, 1 ) ),
               wxString::FromCDouble( m_value.GetValue( 1, 0 ) ), wxString::FromCDouble( m_value.GetValue( 1, 1 ) ),
               wxString::FromCDouble( m_value.GetValue( 2, 0 ) ), wxString::FromCDouble( m_value.GetValue( 2, 1 ) )
             );

    return s;
}

a2dMatrixProperty* a2dMatrixProperty::CreatePropertyFromString( const a2dPropertyIdMatrix* id, const wxString& value )
{
    wxString error;
    a2dAffineMatrix result;
    if ( ::ParseCvgTransForm( result, value , error ) )
        return new a2dMatrixProperty( id, result );
    else
        return 0;
}

//----------------------------------------------------------------------------
// a2dPoint2DProperty
//----------------------------------------------------------------------------

a2dPoint2DProperty::a2dPoint2DProperty(): a2dNamedProperty()
{}

a2dPoint2DProperty::a2dPoint2DProperty( const a2dPropertyIdPoint2D* id, const a2dPoint2D& value, int index, bool afterinversion )
    : a2dNamedProperty( id )
{
    m_value = value;
    m_index = index;
    m_afterinversion = afterinversion;
}

a2dPoint2DProperty::a2dPoint2DProperty( const a2dPoint2DProperty* ori )
    : a2dNamedProperty( *ori )
{
    m_value = ori->m_value;
    m_index = ori->m_index;
    m_afterinversion = ori->m_afterinversion;
}

a2dPoint2DProperty::~a2dPoint2DProperty()
{}

a2dPoint2DProperty::a2dPoint2DProperty( const a2dPoint2DProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
    m_index = other.m_index;
    m_afterinversion = other.m_afterinversion;
}

a2dNamedProperty* a2dPoint2DProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPoint2DProperty( *this );
};

void a2dPoint2DProperty::Assign( const a2dNamedProperty& other )
{
    a2dPoint2DProperty* propcast = wxStaticCast( &other, a2dPoint2DProperty );
    m_value = propcast->m_value;
    m_index = propcast->m_index;
    m_afterinversion = propcast->m_afterinversion;
}

a2dPoint2DProperty* a2dPoint2DProperty::CreatePropertyFromString( const a2dPropertyIdPoint2D* WXUNUSED( id ), const wxString& WXUNUSED( value ) )
{
    return 0;
}

#if wxART2D_USE_CVGIO
void a2dPoint2DProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "point2d" ), StringValueRepresentation() );
    }
    else
    {}
}

void a2dPoint2DProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString str = parser.GetAttributeValue( wxT( "point2d" ) );
        wxString error;
        if ( !str.IsEmpty() ) //!todo && !::ParsePoint2D( m_value, str, error ) )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid point2d %s at line %d" ), str.c_str(), parser.GetCurrentLineNumber() );
        }
    }
    else
    {}
}
#endif //wxART2D_USE_CVGIO

void a2dPoint2DProperty::SetValue( double x, double y, int index, bool afterinversion )
{
    m_value.m_x = x;
    m_value.m_y = y;
    m_index = index;
    m_afterinversion = afterinversion;
}

void a2dPoint2DProperty::SetValue( const a2dPoint2D& P, int index, bool afterinversion )
{
    m_value = P;
    m_index = index;
    m_afterinversion = afterinversion;
}

wxString a2dPoint2DProperty::StringRepresentation() const
{
    wxString s;
    s.Printf ( wxT( "%s = point2d( %s, %s, %d )" ), m_id->GetName().c_str(), wxString::FromCDouble( m_value.m_x ), wxString::FromCDouble( m_value.m_y ), m_index );

    if ( m_afterinversion == true )
        s << wxT( "true" );
    else
        s << wxT( "false" );

    return s;
}

wxString a2dPoint2DProperty::StringValueRepresentation() const
{
    wxString s;
    s.Printf ( wxT( "point2d( %s, %s, %d )" ), wxString::FromCDouble( m_value.m_x ), wxString::FromCDouble( m_value.m_y ), m_index );

    if ( m_afterinversion == true )
        s << wxT( "true" );
    else
        s << wxT( "false" );

    return s;
}
