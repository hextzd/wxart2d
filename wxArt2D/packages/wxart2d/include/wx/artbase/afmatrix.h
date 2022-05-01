/*! \file wx/artbase/afmatrix.h
    \brief affine matrix class

    Affine matrixes are used for placing objects in a drawing at a position and angle etc.

    \author Klaas Holwerda Chris Breeze, Julian Smart
    \date Created 01/02/97

    Copyright: (c) Julian Smart and Markus Holzem

    Licence: wxWidgets licence

    Affine Matrix calculation.

    RCS-ID: $Id: afmatrix.h,v 1.14 2009/08/20 18:59:13 titato Exp $
*/

#ifndef _WX_MATRIXH__
#define _WX_MATRIXH__

#include "wx/geometry.h"
#include "wx/artbase/artglob.h"

//! A 2x3 affine matrix class for 2D transformations
/*!
  A 2x3 matrix to do 2D transformations.

  It follows a 'column vector' convention, more or less.  A point P=(x,y) is
  treated as the transpose of [x, y, 1], and transformation is accomplished
  by pre-multiplication by the transformation matrix M:
  \verbatim
                  [ sx  shx  tx ]   [x]
       [x',y'] =  [ shy  sy  ty ] * [y] = [sx*x+shx*y+tx, shy*x+sy*y+ty]
                                    [1]
  \endverbatim
  It is only "more-or-less" so because accessors of this class use a convention
  of (col,row), which is backwards from normal mathematical notation.  The
  actual memory layout is double[3][2], which means it's stored column-major:
      {sx, shy, shx, sy, tx, ty}.

  It can be used to map data to window coordinates.
  But also for manipulating your own 2D data.
  Because this class is specialy designed to do 2D transformations a lot of things
  could be optimized for this particlular purpose.
  For example drawing a picture (composed of several primitives)
  at a certain coordinate and angle within another parent picture.
  At all times m_isIdentity is set if the matrix itself is an Identity matrix.
  m_isIdentity is used where possible to optimize calculations.

    \ingroup drawer
    \ingroup general
*/
class A2DARTBASEDLLEXP a2dAffineMatrix
{
public:

    //! constructor
    /*!
        Initialize matrix.

        First a translation is proceed after this a scalling at that position followed by a rotation at that position.

        \param xt translation in x
        \param yt translation in y
        \param scalex scaling in x
        \param scaley scaling in y
        \param degrees rotation clockwise
    */
    a2dAffineMatrix( double xt, double yt, double scalex = 1, double scaley = 1, double degrees = 0 );

    a2dAffineMatrix( double sx, double shx, double shy, double sy, double tx, double ty );

    //! constructor (makes an identity matrix)
    a2dAffineMatrix( void );

    //! constructor using other matrix
    a2dAffineMatrix( const a2dAffineMatrix& mat );

    //!get the value in the matrix at col,row
    /*!
        rows are horizontal (second index of m_matrix member)
        columns are vertical (first index of m_matrix member)
    */
    double GetValue( int col, int row ) const;

    //!set the value in the matrix at col,row
    /*!
        rows are horizontal (second index of m_matrix member)
        columns are vertical (first index of m_matrix member)
    */
    void SetValue( int col, int row, double value );

    //! make the same
    void operator= ( const a2dAffineMatrix& mat );

    //! are they eqaul
    friend A2DARTBASEDLLEXP bool operator == ( const a2dAffineMatrix& a, const a2dAffineMatrix& b );

    //! are they NOT eqaul
    friend A2DARTBASEDLLEXP bool operator != ( const a2dAffineMatrix& a, const a2dAffineMatrix& b );

    //! are they eqaul
    bool operator== ( const a2dAffineMatrix& a );

    //! are they NOT eqaul
    bool operator!= ( const a2dAffineMatrix& a );

    //!multiply matrix m with this
    a2dAffineMatrix& operator*=( const a2dAffineMatrix& m );

    //!multiply this by matrix m and return result
    a2dAffineMatrix operator*( const a2dAffineMatrix& m ) const;

    //!subtract two matrices
    a2dAffineMatrix operator-( const a2dAffineMatrix& a );

    //!subtract two matrices
    friend A2DARTBASEDLLEXP a2dAffineMatrix operator-( const a2dAffineMatrix& a, const a2dAffineMatrix& b );

    static inline a2dAffineMatrix Mul( const a2dAffineMatrix& m1, const a2dAffineMatrix& m2 ) { return m1 * m2; }

    /*!
        rows are horizontal (second index of m_matrix member)
        columns are vertical (first index of m_matrix member)
    */
    double& operator()( int col, int row );

    /*!
        rows are horizontal (second index of m_matrix member)
        columns are vertical (first index of m_matrix member)
    */
    double operator()( int col, int row ) const;

    //! Invert matrix
    bool Invert( void );

    //! Make into identity matrix
    bool Identity( void );

    //! Return a static identity matrix as reference
    static const a2dAffineMatrix& GetIdentityMatrix() {return sm_Identity; }

    //! Is the matrix the identity matrix?
    /*! Only returns a flag, which is set whenever an operation
    is done.
    */
    inline bool IsIdentity( void ) const { return m_isIdentity; }

    //! This does an actual check.
    inline bool IsIdentity1( void ) const ;

    //! Is the matrix only a translation?
    /*! checks the matrix to see if it only does a traslation.
    */
    inline bool IsTranslate( void ) const;

    inline bool IsTranslateScaleIso( void ) const;

    //!Scale by scale (isotropic scaling i.e. the same in x and y):
    /*!ex:
    \verbatim
               | scale  0      0      |
     matrix' = |  0     scale  0      | x matrix
               |  0     0      scale  |
    \endverbatim
    */
    bool Scale( double scale );

    //! Remove Scale:
    /*! ex:
       \verbatim
                 | 1/sqrt(det) 0           0      |
       matrix' = | 0           1/sqrt(det) 0      | x matrix
                 | 0           0           1      |
       \endverbatim
    */
    bool RemoveScale();

    //!Scale with center point and x/y scale
    /*!ex:
    \verbatim
               |  xs    0      xc(1-xs) |
     matrix' = |  0    ys      yc(1-ys) | x matrix
               |  0     0      1        |
    \endverbatim
    */
    a2dAffineMatrix&  Scale( const double& xs, const double& ys, const double& xc, const double& yc );

    //! mirror a matrix in x, y
    /*!ex:
    \verbatim
               | -1     0      0 |
     matrix' = |  0    -1      0 | x matrix
               |  0     0      1 |
    \endverbatim
    */
    a2dAffineMatrix&  Mirror( bool y = true, bool x = false );

    //! Translate by dx, dy:
    /*!ex:
    \verbatim
               | 1  0 dx |
     matrix' = | 0  1 dy | x matrix
               | 0  0  1 |
    \endverbatim
    */
    bool Translate( double x, double y );

    //! Rotate clockwise by the given number of degrees:
    /*!
    Rotates clockwise: If you call <code>Rotate(20); Rotate(10);</code> the absolute
    rotation is 30 degrees (if the initially rotation was 0)

    \see SetRotation

    \ex:
    \verbatim
               |  cos sin 0 |
     matrix' = | -sin cos 0 | x matrix
               |   0   0  1 |
    \endverbatim
    */
    bool Rotate( double angle );

    //! Skew Xaxis:
    /*!ex:
    \verbatim
               |   1   tan(a) 0 |
     matrix' = |   0   1      0 | x matrix
               |   0   0      1 |
    \endverbatim
    */
    bool SkewX( double degrees );

    //! Skew Yaxis:
    /*!ex:
    \verbatim
               |     1    0      0 |
     matrix' = | tan(a)   1      0 | x matrix
               |     0    0      1 |
    \endverbatim
    */
    bool SkewY( double degrees );

    //!Rotate counter clockwise with point of rotation
    /*!ex:
    \verbatim
               |  cos(r) -sin(r)    x(1-cos(r))+y(sin(r)|
     matrix' = |  sin(r)  cos(r)    y(1-cos(r))-x(sin(r)| x matrix
               |   0          0                       1 |
    \endverbatim
    */
    a2dAffineMatrix&  Rotate( const double& r, const double& x, const double& y );

    //! Transform a point
    void TransformPoint( double x, double y, double& tx, double& ty ) const;

    //! Transform a point
    void TransformPoint( double& tx, double& ty ) const;

    //! Transform a point
    void TransformPoint( a2dPoint2D* point ) const;

    //! Transform a point
    void TransformPoint( const a2dPoint2D& src, a2dPoint2D* dest ) const;

    //! Transform a vector
    /*!
        This transforms a free vector.  It is equivalent to transforming
        the two points (px,py) and (px+dx,py+dy) and then subtracting the
        former from the latter.  In effect this means it ignores the
        translation portion of the matrix, and just multiplies by the core 2x2
        linear transform.
    */
    void TransformVector( double dx, double dy, double& tdx, double& tdy ) const;

    //! Transform a distance
    /*!
        This transforms a relative distance.
        A horizontal and vertical line starting at the origin, and distance long,
        is transformed. The length of the line with the minimum distance in the transformed space is returned.

        \param distance distance to transform

        \return transformed distance
    */
    double TransformDistance( double distance ) const;

    //! Transform a distance
    void TransformDistance( double& dx, double& dy ) const;

    //! return scaling in X
    double Get_scaleX() const;

    //! return scaling in Y
    double Get_scaleY() const;

    //! return rotation
    /*!
        Returns the absolute rotation in the range –179.999... to 180 degrees
        \see SetRotation
    */
    double GetRotation() const;

    //! set rotation
    /*!
        In opposite to Rotate this method sets an absolute rotation.
        \see Rotate
    */
    void   SetRotation( double rotation );

    //! get the translation vector as a point
    a2dPoint2D GetTranslation() const { return a2dPoint2D( m_matrix[2][0], m_matrix[2][1] ); }

    //! Calculate the determinat of the linear transformation
    double GetDeterminant() const;
    //! Calculate the determinat of the shift vector and the transformed y-axis vector
    double GetDeterminant1() const;
    //! Calculate the determinat of the transformed x-axis vector and the shift vector
    double GetDeterminant2() const;
    //! Calculate the maximum absolute of the elemnts entering GetDeterminant
    double GetMaximum() const;
    //! Calculate the maximum absolute of the elemnts entering GetDeterminant1
    double GetMaximum1() const;
    //! Calculate the maximum absolute of the elemnts entering GetDeterminant2
    double GetMaximum2() const;

    //! Dump the Matrix to a debug window
    void DebugDump( const wxChar* text, double w, double h ) const;


public:

    //! array store the six double for the affine matrix
    double  m_matrix[3][2];

    //! true if identity matrix
    bool    m_isIdentity;

protected:
    //! A static identity matrix in case an identity matrix is to be returned as reference
    static const a2dAffineMatrix sm_Identity;
};

//! Is the matrix the identity matrix?
//! Each operation checks whether the result is still the identity matrix and sets a flag.
inline bool a2dAffineMatrix::IsIdentity1( void ) const
{
    return
        ( m_matrix[0][0] == 1.0 &&
          m_matrix[1][1] == 1.0 &&
          m_matrix[1][0] == 0.0 &&
          m_matrix[2][0] == 0.0 &&
          m_matrix[0][1] == 0.0 &&
          m_matrix[2][1] == 0.0
        );
}

inline  bool a2dAffineMatrix::IsTranslate( void ) const
{
    return
        ( m_matrix[0][0] == 1.0 &&
          m_matrix[1][1] == 1.0 &&
          m_matrix[1][0] == 0.0 &&
          m_matrix[0][1] == 0.0
        );
}

inline bool a2dAffineMatrix::IsTranslateScaleIso( void ) const
{
    return
        ( m_matrix[0][0] != 0.0 &&
          m_matrix[1][1] != 0.0 &&
          ( m_matrix[0][0] == m_matrix[1][1] || m_matrix[0][0] == -m_matrix[1][1] ) &&
          m_matrix[1][0] == 0.0 &&
          m_matrix[0][1] == 0.0
        );
}

//! global a2dAffineMatrix to set/pass the identity matrix.
A2DARTBASEDLLEXP_DATA( extern a2dAffineMatrix ) a2dIDENTITY_MATRIX;

//! function to parse a string in SVG/CVG format and return the resulting matrix
/*!
    \param matrix the resulting matrix.
    \param str the string to parse
    \param error error returned if not right

    \return true if correct string.
*/
extern A2DARTBASEDLLEXP bool ParseCvgTransForm( a2dAffineMatrix& matrix, const wxString& str, wxString& error );

//----------------------------------------------------------------------------
// a2dMatrixProperty
//----------------------------------------------------------------------------


//! declarations required for a2dMatrixProperty
class A2DARTBASEDLLEXP a2dMatrixProperty;
#if defined(WXART2D_USINGDLL)
template class A2DARTBASEDLLEXP a2dPropertyIdTyped<a2dAffineMatrix, a2dMatrixProperty>;
#endif
typedef a2dPropertyIdTyped<a2dAffineMatrix, a2dMatrixProperty> a2dPropertyIdMatrix;


//! property to hold a a2dAffineMatrix
/*!
    \ingroup property
*/
class A2DARTBASEDLLEXP a2dMatrixProperty: public a2dNamedProperty
{
public:

    a2dMatrixProperty();

    a2dMatrixProperty( const a2dPropertyIdMatrix* id, const a2dAffineMatrix& value = a2dIDENTITY_MATRIX );

    a2dMatrixProperty( const a2dMatrixProperty* ori );

    a2dMatrixProperty( const a2dMatrixProperty& other );

    virtual ~a2dMatrixProperty();

    virtual void Assign( const a2dNamedProperty& other );

    void SetValue( const a2dAffineMatrix& value );

    a2dAffineMatrix& GetValue() { return m_value; }
    const a2dAffineMatrix& GetValue() const { return m_value; }

    a2dAffineMatrix* GetValuePtr() { return & m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dMatrixProperty* CreatePropertyFromString( const a2dPropertyIdMatrix* id, const wxString& value );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dMatrixProperty )

protected:

    a2dAffineMatrix m_value;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;
};

//! declarations required for a2dPoint2DProperty
class A2DARTBASEDLLEXP a2dPoint2DProperty;
#if defined(WXART2D_USINGDLL)
template class A2DARTBASEDLLEXP a2dPropertyIdTyped<a2dPoint2D, a2dPoint2DProperty>;
#endif
typedef a2dPropertyIdTyped<a2dPoint2D, a2dPoint2DProperty> a2dPropertyIdPoint2D;

//! property to hold a a2dPoint2D plus an index in e.g a polygon.
/*!
    A wxPoint2d is held within this property, next to that an indedx into an array/list can be set.
    Also an afterinversion can used by Objects to transform first or not when using a point.

    This property is used in polygon editing to achieve undo, but you can use it for anything needing a point as property.

    \ingroup property
*/
class A2DARTBASEDLLEXP a2dPoint2DProperty: public a2dNamedProperty
{
public:

    //! constructor
    a2dPoint2DProperty();

    //! constructor
    a2dPoint2DProperty( const a2dPropertyIdPoint2D* id, const a2dPoint2D& value, int index = 0, bool afterinversion = false );

    //! constructor
    a2dPoint2DProperty( const a2dPoint2DProperty* ori );

    //! constructor
    a2dPoint2DProperty( const a2dPoint2DProperty& other );

    //! destructor
    virtual ~a2dPoint2DProperty();

    virtual void Assign( const a2dNamedProperty& other );

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dPoint2DProperty* CreatePropertyFromString( const a2dPropertyIdPoint2D* id, const wxString& value );

    //! set the property information to this
    void SetValue( double x, double y, int index = 0, bool afterinversion = false );

    //! set the property information to this
    void SetValue( const a2dPoint2D& P, int index = 0, bool afterinversion = false );

    //! get the point
    a2dPoint2D& GetValue() { return m_value; }
    const a2dPoint2D& GetValue() const { return m_value; }

    //! get the point its address
    a2dPoint2D* GetValuePtr() { return & m_value; }

    //! get property setting for m_index
    wxInt32 GetIndex() { return m_index; }

    //! get property setting for m_afterinversion
    bool GetAfterinversion() { return m_afterinversion; }

    //! a string form presentation for this property
    virtual wxString StringRepresentation() const;

    //! a string value presentation for this property
    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dPoint2DProperty )

protected:

    //! the point that is held within this property
    a2dPoint2D m_value;

    //! Can be used as position to insert a point into a polygon
    wxInt32 m_index;

    //! when adding this point to an object with a matrix, first inverse transform it to the position
    //! without the matrix.
    bool m_afterinversion;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;
};

#endif

