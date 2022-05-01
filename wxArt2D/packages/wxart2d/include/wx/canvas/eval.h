/*! \file wx/canextobj/eval.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: eval.h,v 1.8 2008/09/05 19:01:10 titato Exp $
*/
#ifndef __WXEVAL_H__
#define __WXEVAL_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_EXCEPTIONS
#include <setjmp.h>
#endif

#ifdef A2DEDITORMAKINGDLL
#define A2DCANVASDLLEXP WXEXPORT
#define A2DCANVASDLLEXP_DATA(type) WXEXPORT type
#define A2DCANVASDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DCANVASDLLEXP WXIMPORT
#define A2DCANVASDLLEXP_DATA(type) WXIMPORT type
#define A2DCANVASDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DCANVASDLLEXP
#define A2DCANVASDLLEXP_DATA(type) type
#define A2DCANVASDLLEXP_CTORFN
#endif

/* Some of you may choose to define TYPENUMBER as a "float" instead... */
#define TYPENUMBER            double          /*!< Type of numbers to work with */

#define VARLEN          15              /*!< Max length of variable names */
#define TOKLEN          30              /*!< Max token length */

#define VAR             1
#define DEL             2
#define NUM             3

//! holds a variable in an evaluation string for a2dEval
/*!
    \ingroup  global
*/
class A2DCANVASDLLEXP a2dEvalVar
{
public:

    a2dEvalVar( const wxString& name, TYPENUMBER value )
    {
        m_name = name;
        m_value = value;
    }

    ~a2dEvalVar() {}

    wxString m_name;               /* Variable name */
    TYPENUMBER m_value;            /* Variable value */
};


//! holds a function in an evaluation string for a2dEval
/*!
    \ingroup  global
*/
class A2DCANVASDLLEXP a2dEvalFunction
{
    typedef TYPENUMBER  ( *fArgX )( ... ); // Sloppy function type used internally

public:

    // Function types I can accept in my ctors
    typedef TYPENUMBER ( *fArg1 )( TYPENUMBER );
    typedef TYPENUMBER ( *fArg2 )( TYPENUMBER, TYPENUMBER );

    //! name, nr args, function to call
    a2dEvalFunction( const wxString& name, fArg1 func )
    {
        m_name = name;
        m_args = 1;
        m_func = ( fArgX )func;
    }

    a2dEvalFunction( const wxString& name, fArg2 func )
    {
        m_name = name;
        m_args = 2;
        m_func = ( fArgX )func;
    }

    wxString m_name;                          /* Function name */
    int   m_args;                          /* Number of arguments to expect */
    fArgX m_func;
};

#include <wx/listimpl.cpp>
WX_DECLARE_LIST_WITH_DECL( a2dEvalFunction, a2dFunctionList, class A2DCANVASDLLEXP );
WX_DECLARE_LIST_WITH_DECL( a2dEvalVar, a2dVariableList, class A2DCANVASDLLEXP );


/* The following macros are ASCII dependant, no EBCDIC here! */
#define iswhite(c)  (c == _T(' ') || c == _T('\t'))
#define isnumer(c)  ((c >= _T('0') && c <= _T('9')) || c == _T('.'))
#define isalphaeval(c)  ((c >= _T('a') && c <= _T('z')) || (c >= _T('0') && c <= _T('9')) \
                         || c == _T('_'))
#define isdelim(c)  (c == _T('+') || c == _T('-') || c == _T('*') || c == _T('/') || c == _T('%') \
                     || c == _T('^') || c == _T('(') || c == _T(')') || c == _T(',') || c == _T('='))

/* Codes returned from the evaluator */
#define E_OK           0        /*!< Successful evaluation */
#define E_SYNTAX       1        /*!< Syntax error */
#define E_UNBALAN      2        /*!< Unbalanced parenthesis */
#define E_DIVZERO      3        /*!< Attempted division by zero */
#define E_UNKNOWN      4        /*!< Reference to unknown variable */
#define E_MAXVARS      5        /*!< Maximum variables exceeded */
#define E_BADFUNC      6        /*!< Unrecognised function */
#define E_NUMARGS      7        /*!< Wrong number of arguments to funtion */
#define E_NOARG        8        /*!< Missing an argument to a funtion */
#define E_EMPTY        9        /*!< Empty expression */

//! expression(s) evaluation based on a string.
/*!
The a2dEval module provides a class that allows you to incorporate
mathematical expression evaluation into your programs, that can be changed
at run time.  For example, using the class you can evaluate such expressions as:
    \verbatim
        1+1
        10 * (x=5)                 <== Assigns 5 to X first!
        ((1/3) * sin(45))^2
        X=50
        Y=100
        z=hypot(x,y)
    \endverbatim

You can extend Constants and Functions that can be used.

    \ingroup  global
*/
class A2DCANVASDLLEXP a2dEval
{

public:

    a2dEval();

    ~a2dEval();

    void SetEvalString( const wxString& toEval ) { m_eval = toEval; }

    bool GetSymbol( const wxString& envname, TYPENUMBER* envValue );

    void ClearAllVars();

    bool ClearVar( const wxString&  name );

    int Evaluate( TYPENUMBER* result, int* a );

    void Parse();

    bool GetValue( const wxString& name, TYPENUMBER* value );

    bool SetValue( const wxString& name, TYPENUMBER value );

    a2dFunctionList m_functions;
    a2dVariableList m_vars;
    a2dVariableList m_const;

protected:

    int  Level1( TYPENUMBER* r );
    void Level2( TYPENUMBER* r );
    void Level3( TYPENUMBER* r );
    void Level4( TYPENUMBER* r );
    void Level5( TYPENUMBER* r );
    void Level6( TYPENUMBER* r );

private:

    wxString m_eval;

    //! Pointer to the user's expression
#if wxUSE_UNICODE
    const wxStringCharType* m_expression;
    //! Holds the current token
    const wxStringCharType* m_token;
#else
    wxChar* m_expression;
    //! Holds the current token
    wxChar* m_token;
#endif // wxUSE_UNICODE

    wxString m_tokenstr;

    //! Type of the current token
    int      m_type;

#if !wxUSE_EXCEPTIONS
    //! jmp_buf for errors
    jmp_buf  m_jb;
#endif
};

#endif

