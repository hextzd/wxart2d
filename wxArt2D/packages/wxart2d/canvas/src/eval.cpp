/*! \file canextobj/src/eval.cpp
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: eval.cpp,v 1.16 2009/06/07 13:28:08 titato Exp $
*/

/*************************************************************************
**                                                                       **
** EE.C         Expression Evaluator                                     **
**                                                                       **
** AUTHOR:      Mark Morley                                              **
** COPYRIGHT:   (c) 1992 by Mark Morley                                  **
** DATE:        December 1991                                            **
** HISTORY:     Jan 1992 - Made it squash all command line arguments     **
**                         into one big long string.                     **
**                       - It now can set/get VMS symbols as if they     **
**                         were variables.                               **
**                       - Changed max variable name length from 5 to 15 **
**              Jun 1992 - Updated comments and docs                     **
**                                                                       **
** You are free to incorporate this code into your own works, even if it **
** is a commercial application.  However, you may not charge anyone else **
** for the use of this code!  If you intend to distribute your code,     **
** I'd appreciate it if you left this message intact.  I'd like to       **
** receive credit wherever it is appropriate.  Thanks!                   **
**                                                                       **
** I don't promise that this code does what you think it does...         **
**                                                                       **
** Please mail any bug reports/fixes/enhancments to me at:               **
**      morley@camosun.bc.ca                                             **
** or                                                                    **
**      Mark Morley                                                      **
**      3889 Mildred Street                                              **
**      Victoria, BC  Canada                                             **
**      V8Z 7G1                                                          **
**      (604) 479-7861                                                   **
**                                                                       **
 *************************************************************************/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/defs.h"
#endif


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <wx/utils.h>



#include "wx/canvas/eval.h"


#define SAVE_ERR(n) {ERROR2=n; ERPOS=m_expression-ERANC-1; ERTOK = m_token;}

#if wxUSE_EXCEPTIONS
class EvalErr
{
    // Simple type to throw as exception.
};
#define ERR(n) { SAVE_ERR(n); throw EvalErr();}
#else
#define ERR(n) { SAVE_ERR(n); longjmp(m_jb,1);}
#endif

/* These defines only happen if the values are not already defined!  You may
   want to add more precision here, if your machine supports it. */



#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_E
#define M_E     2.71828182845904523536
#endif


/*************************************************************************
**                                                                       **
** PROTOTYPES FOR CUSTOM MATH FUNCTIONS                                  **
**                                                                       **
 *************************************************************************/

double deg( double x );
double rad( double x );


/*************************************************************************
**                                                                       **
** VARIABLE DECLARATIONS                                                 **
**                                                                       **
*************************************************************************/

int   ERROR2;                 /* The error number */
wxString ERTOK;     /* The token that generated the error */
int   ERPOS;                 /* The offset from the start of the expression */
#if wxUSE_UNICODE
    wxStringCharType* ERANC;                 /* Used to calculate ERPOS */
#else
    wxChar* ERANC;                 /* Used to calculate ERPOS */
#endif // wxUSE_UNICODE

    

WX_DEFINE_LIST( a2dFunctionList );
WX_DEFINE_LIST( a2dVariableList );

/*************************************************************************
**                                                                       **
** Some custom math functions...   Note that they must be prototyped     **
** above (if your compiler requires it)                                  **
**                                                                       **
** deg( x )             Converts x radians to degrees.                   **
** rad( x )             Converts x degrees to radians.                   **
**                                                                       **
 *************************************************************************/

double
deg( double x )
{
    return( x * 180.0 / M_PI );
}

double
rad( double x )
{
    return( x * M_PI / 180.0 );
}

a2dEval::a2dEval()
{
    m_tokenstr.Alloc( VARLEN );
#if wxUSE_UNICODE
    m_token = m_tokenstr.wx_str();
#else
    m_token = ( wxChar* ) m_tokenstr.c_str();
#endif

    ClearAllVars();

    /*
    Add any "constants" here...  These are "read-only" values that are
    provided as a convienence to the user.  Their values can not be
    permanently changed.  The first field is the variable name, the second
    is its value.
    */
    m_const.Append( new a2dEvalVar( wxT( "pi" ), M_PI ) );
    m_const.Append( new a2dEvalVar( wxT( "e" ),  M_E ) );

    /*
    Add any math functions that you wish to recognise here. Use the
    MATH_FUNC helper macro below to define the functions. Note that
    only those math functions for which there are matching
    a2dEvalFunction can be used. If you need to add other kinds of
    functions (eg, with more than two arguments), you also need to
    add the correponding a2dEvalFunction constructor.
    */
#define MATH_FUNC(f) m_functions.Append(new a2dEvalFunction(wxT(#f), f))
    MATH_FUNC( sin );
    MATH_FUNC( cos );
    MATH_FUNC( tan );
    MATH_FUNC( asin );
    MATH_FUNC( acos );
    MATH_FUNC( atan );
    MATH_FUNC( sinh );
    MATH_FUNC( cosh );
    MATH_FUNC( tanh );
    MATH_FUNC( exp );
    MATH_FUNC( log );
    MATH_FUNC( log10 );
    MATH_FUNC( sqrt );
    MATH_FUNC( floor );
    MATH_FUNC( ceil );
    MATH_FUNC( fabs );
    MATH_FUNC( deg );
    MATH_FUNC( rad );
    MATH_FUNC( hypot );
}

/*************************************************************************
**                                                                       **
** GetSymbol( char* s )                                                  **
**                                                                       **
** This routine obtains a value from the program's environment.          **
** This works for DOS and VMS (and other OS's???)
**                                                                       **
 ************************************************************************/

bool a2dEval::GetSymbol( const wxString& envname, TYPENUMBER* envValue )
{
    wxString senvValue;
    if( !wxGetEnv( envname, &senvValue ) )
        return false;   // Environment name not found

    if ( senvValue.ToDouble( envValue ) )
        return true;
    return false;
}

a2dEval::~a2dEval()
{
    a2dVariableList::compatibility_iterator node = m_vars.GetFirst();
    while ( node )
    {
        delete node->GetData();
        m_vars.DeleteNode( node );
        node = m_vars.GetFirst();
    }

    node = m_const.GetFirst();
    while ( node )
    {
        delete node->GetData();
        m_const.DeleteNode( node );
        node = m_const.GetFirst();
    }

    a2dFunctionList::compatibility_iterator node2 = m_functions.GetFirst();
    while ( node2 )
    {
        delete node2->GetData();
        m_functions.DeleteNode( node2 );
        node2 = m_functions.GetFirst();
    }
}
/*************************************************************************
**                                                                       **
** ClearAllVars()                                                        **
**                                                                       **
** Erases all user-defined variables from memory. Note that constants    **
** can not be erased or modified in any way by the user.                 **
**                                                                       **
** Returns nothing.                                                      **
**                                                                       **
 *************************************************************************/

void a2dEval::ClearAllVars()
{
    a2dVariableList::compatibility_iterator node = m_vars.GetFirst();
    while ( node )
    {
        delete node->GetData();
        m_vars.DeleteNode( node );
        node = m_vars.GetFirst();
    }
}


/*************************************************************************
**                                                                       **
** ClearVar( char* name )                                                **
**                                                                       **
** Erases the user-defined variable that is called NAME from memory.     **
** Note that constants are not affected.                                 **
**                                                                       **
** Returns 1 if the variable was found and erased, or 0 if it didn't     **
** exist.                                                                **
**                                                                       **
 *************************************************************************/

bool a2dEval::ClearVar( const wxString& name )
{
    a2dVariableList::compatibility_iterator node = m_vars.GetFirst();
    while ( node )
    {
        a2dEvalVar* obj = node->GetData();
        if( ( !obj->m_name.IsEmpty () ) && obj->m_name == name )
        {
            a2dVariableList::compatibility_iterator h = node;
            //remove this node
            node = node->GetNext();
            delete h->GetData();
            m_vars.DeleteNode( h );
            return true;
        }
        else
            node = node->GetNext();
    }

    return false;
}


/*************************************************************************
**                                                                       **
** GetValue( char* name, TYPENUMBER* value )                                   **
**                                                                       **
** Looks up the specified variable (or constant) known as NAME and       **
** returns its contents in VALUE.                                        **
**                                                                       **
** First the user-defined variables are searched, then the constants are **
** searched.                                                             **
**                                                                       **
** Returns true if the value was found, or false if it wasn't.                  **
**                                                                       **
 *************************************************************************/

bool a2dEval::GetValue( const wxString& name, TYPENUMBER* value )
{
    /* First check for an environment variable reference... */
    if( name[0] == wxT( '_' ) )
        return( GetSymbol( name.Mid( 1 ), value ) );


    /* Now check the user-defined variables. */
    a2dVariableList::compatibility_iterator node = m_vars.GetFirst();
    while ( node )
    {
        a2dEvalVar* obj = node->GetData();
        if ( obj->m_name == name )
        {
            *value = obj->m_value;
            return true;
        }
        node = node->GetNext();
    }


    /* Now check the programmer-defined constants. */
    node = m_const.GetFirst();
    while ( node )
    {
        a2dEvalVar* obj = node->GetData();
        if( obj->m_name == name )
        {
            *value = obj->m_value;
            return true;
        }
        node = node->GetNext();
    }

    return false;
}


/*************************************************************************
**                                                                       **
** SetValue( char* name, TYPENUMBER* value )                                   **
**                                                                       **
** First, it erases any user-defined variable that is called NAME.  Then **
** it creates a new variable called NAME and gives it the value VALUE.   **
**                                                                       **
** Returns 1 if the value was added, or 0 if there was no more room.     **
**                                                                       **
 *************************************************************************/

bool a2dEval::SetValue( const wxString& name, TYPENUMBER value )
{
    ClearVar( name );
    m_vars.Append( new a2dEvalVar( name, value ) );
    return true;
}


/*************************************************************************
**                                                                       **
** Parse()   Internal use only                                           **
**                                                                       **
** This function is used to grab the next token from the expression that **
** is being evaluated.                                                   **
**                                                                       **
 *************************************************************************/

void a2dEval::Parse()
{
#if wxUSE_UNICODE
    wxStringCharType* t;
#else
    wxChar* t;
#endif // wxUSE_UNICODE

    m_type = 0;
#if wxUSE_UNICODE
    t = (wxStringCharType*) m_token;
#else
    t = m_token;
#endif // wxUSE_UNICODE
    while( iswhite( *m_expression ) )
        m_expression++;
    if( isdelim( *m_expression ) )
    {
        m_type = DEL;
        *t++ = *m_expression++;
    }
    else if( isnumer( *m_expression ) )
    {
        m_type = NUM;
        while( isnumer( *m_expression ) )
            *t++ = *m_expression++;
    }
    else if( isalphaeval( *m_expression ) )
    {
        m_type = VAR;
        while( isalphaeval( *m_expression ) )
            *t++ = *m_expression++;
        //m_token[VARLEN] = 0;
    }
    else if( *m_expression )
    {
        *t++ = *m_expression++;
        *t = 0;
        ERR( E_SYNTAX );
    }
    *t = 0;
    while( iswhite( *m_expression ) )
        m_expression++;
}


/*************************************************************************
**                                                                       **
** Level1( TYPENUMBER* r )   Internal use only                           **
**                                                                       **
** This function handles any variable assignment operations.             **
** It returns a value of 1 if it is a top-level assignment operation,    **
** otherwise it returns 0                                                **
**                                                                       **
 *************************************************************************/

int a2dEval::Level1( TYPENUMBER* r )
{
    wxString t;

    if( m_type == VAR )
        if( *m_expression == wxT( '=' ) )
        {
            t = m_token;
            Parse();
            Parse();
            if( !*m_token )
            {
                ClearVar( t );
                return( 1 );
            }
            Level2( r );
            if( ! SetValue( t, *r ) )
                ERR( E_MAXVARS );
            return( 1 );
        }
    Level2( r );
    return( 0 );
}


/*************************************************************************
**                                                                       **
** Level2( TYPENUMBER* r )   Internal use only                                 **
**                                                                       **
** This function handles any addition and subtraction operations.        **
**                                                                       **
 *************************************************************************/

void a2dEval::Level2( TYPENUMBER* r )
{
    TYPENUMBER t = 0;
    wxChar o;

    Level3( r );
    while( ( o = *m_token ) == wxT( '+' ) || o == wxT( '-' ) )
    {
        Parse();
        Level3( &t );
        if( o == wxT( '+' ) )
            *r = *r + t;
        else if( o == wxT( '-' ) )
            *r = *r - t;
    }
}


/*************************************************************************
**                                                                       **
** Level3( TYPENUMBER* r )   Internal use only                           **
**                                                                       **
** This function handles any multiplication, division, or modulo.        **
**                                                                       **
 *************************************************************************/

void a2dEval::Level3( TYPENUMBER* r )
{
    TYPENUMBER t;
    wxChar o;

    Level4( r );
    while( ( o = *m_token ) == wxT( '*' ) || o == wxT( '/' ) || o == wxT( '%' ) )
    {
        Parse();
        Level4( &t );
        if( o == wxT( '*' ) )
            *r = *r * t;
        else if( o == wxT( '/' ) )
        {
            if( t == 0 )
                ERR( E_DIVZERO );
            *r = *r / t;
        }
        else if( o == wxT( '%' ) )
        {
            if( t == 0 )
                ERR( E_DIVZERO );
            *r = fmod( *r, t );
        }
    }
}


/*************************************************************************
**                                                                       **
** Level4( TYPENUMBER* r )   Internal use only                           **
**                                                                       **
** This function handles any "to the power of" operations.               **
**                                                                       **
 *************************************************************************/

void a2dEval::Level4( TYPENUMBER* r )
{
    TYPENUMBER t;

    Level5( r );
    if( *m_token == wxT( '^' ) )
    {
        Parse();
        Level5( &t );
        *r = pow( *r, t );
    }
}


/*************************************************************************
**                                                                       **
** Level5( TYPENUMBER* r )   Internal use only                                 **
**                                                                       **
** This function handles any unary + or - signs.                         **
**                                                                       **
 *************************************************************************/

void a2dEval::Level5( TYPENUMBER* r )
{
    wxChar o = 0;

    if( *m_token == wxT( '+' ) || *m_token == wxT( '-' ) )
    {
        o = *m_token;
        Parse();
    }
    Level6( r );
    if( o == wxT( '-' ) )
        *r = -*r;
}


/*************************************************************************
**                                                                       **
** Level6( TYPENUMBER* r )   Internal use only                           **
**                                                                       **
** This function handles any literal numbers, variables, or functions.   **
**                                                                       **
 *************************************************************************/

void  a2dEval::Level6( TYPENUMBER* r )
{
    int  n;
    TYPENUMBER a[3];

    if( *m_token == wxT( '(' ) )
    {
        Parse();
        if( *m_token == wxT( ')' ) )
            ERR( E_NOARG );
        Level1( r );
        if( *m_token != wxT( ')' ) )
            ERR( E_UNBALAN );
        Parse();
    }
    else
    {
        if( m_type == NUM )
        {
            wxString numstr = m_token;
            numstr.ToDouble( r );
            Parse();
        }
        else if( m_type == VAR )
        {
            if( *m_expression == wxT( '(' ) )
            {
                a2dFunctionList::compatibility_iterator node = m_functions.GetFirst();
                while ( node )
                {
                    a2dEvalFunction* obj = node->GetData();

                    if( m_token == obj->m_name )
                    {
                        Parse();
                        n = 0;
                        do
                        {
                            Parse();
                            if( *m_token == wxT( ')' ) || *m_token == wxT( ',' ) )
                                ERR( E_NOARG );
                            a[n] = 0;
                            Level1( &a[n] );
                            n++;
                        }
                        while( n < 4 && *m_token == wxT( ',' ) );
                        Parse();
                        if( n != obj->m_args )
                        {
                            m_tokenstr = obj->m_name;
                            ERR( E_NUMARGS );
                        }
                        *r = obj->m_func( a[0], a[1], a[2] );
                        return;
                    }
                    if( obj->m_name.IsEmpty () )
                        ERR( E_BADFUNC );

                    node = node->GetNext();
                }
            }
            else if( ! GetValue( m_token, r ) )
                ERR( E_UNKNOWN );

            Parse();
        }
        else
            ERR( E_SYNTAX );
    }
}


/*************************************************************************
**                                                                       **
** Evaluate( TYPENUMBER* result, int* a )                                **
**                                                                       **
** This function is called to evaluate the expression E and return the   **
** answer in RESULT.  If the expression was a top-level assignment, a    **
** value of 1 will be returned in A, otherwise it will contain 0.        **
**                                                                       **
** Returns E_OK if the expression is valid, or an error code.            **
**                                                                       **
 *************************************************************************/

int a2dEval::Evaluate( TYPENUMBER* result, int* a )
{
#if wxUSE_EXCEPTIONS
    try
    {
#else
    if( setjmp( m_jb ) )
        return( ERROR2 );
#endif


#if wxUSE_UNICODE
        m_expression = m_eval.wx_str();
        ERANC = (wxStringCharType*) m_expression;
#else
        m_expression = ( wxChar* ) m_eval.c_str();
        ERANC = ( wxChar* ) m_eval.c_str();
#endif // wxUSE_UNICODE

        //strlwr( m_expression );
        *result = 0;
        Parse();
        if( ! *m_token )
            ERR( E_EMPTY );
        *a = Level1( result );
        return( E_OK );
#if wxUSE_EXCEPTIONS
    }
    catch( const EvalErr& )
    {
        return( ERROR2 );
    }
#endif
}


/*************************************************************************
**                                                                       **
** What follows is a main() routine that evaluates the command line      **
** arguments one at a time, and displays the results of each expression. **
** Without arguments, it becomes an interactive calculator.              **
**                                                                       **
 *************************************************************************/

#include <stdio.h>

const wxChar* ErrMsgs[] =
{
    wxT( "Syntax error" ),
    wxT( "Unbalanced parenthesis" ),
    wxT( "Division by zero" ),
    wxT( "Unknown variable" ),
    wxT( "Maximum variables exceeded" ),
    wxT( "Unrecognised funtion" ),
    wxT( "Wrong number of arguments to funtion" ),
    wxT( "Missing an argument" ),
    wxT( "Empty expression" )
};


/*
int main( int argc, char* argv[] )
{
   TYPENUMBER  result;
   int   i;
   int   ec;
   int   a;
   char  line[1024];
   char* v;
   a2dEval toeval;

   // If we have command line arguments then we evaluate them and exit.
   if( argc > 1 )
   {
      // Concatenate all arguments into one string.
      strcpy( line, argv[1] );
      for( i = 2; i < argc; i++ )
         strcat( line, argv[i] );

      toeval.ClearAllVars();
      toeval.SetEvalString(line);
      // Call the evaluator.
      if( (ec = toeval.Evaluate( &result, &a )) == E_OK )
      {
         // If we didn't assign a variable, then print the result.
         if( ! a )
            printf( "%g\n", result );
      }
      else if( ec != E_EMPTY )
      {
         // Display error info.  In this example, an E_EMPTY error is ignored.
         printf( "ERROR: %s - %s", ErrMsgs[ERROR2 - 1], ERTOK );
         printf( "\n%s", ERANC );
         printf( "\n%*s^\n", ERPOS, "" );
      }
      return 1;
   }

    toeval.ClearAllVars();
    double x;
    for ( x=-10;x<10;x++)
    {
        toeval.SetValue("x",x);
        sprintf(line,"y=x*2");
        toeval.SetEvalString(line);
        toeval.Evaluate( &result, &a );

        double y;
        toeval.GetValue("y",&y);
        printf( "=>%f\n", y );
    }


   // There were no command line arguments, so lets go interactive.
   printf( "\nEE - Equation Evaluator" );
   printf( "\nBy Mark Morley  December 1991" );
   printf( "\nEnter EXIT to quit.\n" );
   printf( "\nEE> " );

   // Input one line at a time from the user.  Note that it uses stdin, so on
   //   DOS or UNIX you could pipe a list of expressions into it...
   for( gets( line ); !feof( stdin ); gets( line ) )
   {
      strlwr( line );
      toeval.SetEvalString(line);

      // Did the user ask to exit?
      if( ! strcmp( line, "exit" ) )
         return 1;

      // Did the user ask to see the variables in memory?
      else if( ! strcmp( line, "vars" ) )
      {
            a2dVariableList::compatibility_iterator node = toeval.m_vars.GetFirst();
            while (node)
            {
                 a2dEvalVar *obj = node->GetData();
                 printf( "%s = %g\n", obj->m_name, obj->m_value );
                 node = node->GetNext();
            }
      }

      // Did the user ask to see the constants in memory?
      else if( ! strcmp( line, "cons" ) )
      {
            a2dVariableList::compatibility_iterator node = toeval.m_const.GetFirst();
            while (node)
            {
                 a2dEvalVar *obj = node->GetData();
                 printf( "%s = %g\n", obj->m_name, obj->m_value );
                 node = node->GetNext();
            }
      }

      // Did the user ask to clear all variables?
      else if( ! strcmp( line, "clr" ) )
         toeval.ClearAllVars();

      // If none of the above, then we attempt to evaluate the user's input.
      else
      {
         // Call the evaluator.
         if( (ec = toeval.Evaluate( &result, &a )) == E_OK )
         {
            // Only display the result if it was not an assignment.
            if( ! a )
               printf( "%g\n", result );
         }
         else if( ec != E_EMPTY )
         {
            // Display error information.  E_EMPTY is ignored.
            printf( "ERROR: %s - %s", ErrMsgs[ERROR - 1], ERTOK );
            printf( "\n%s", ERANC );
            printf( "\n%*s^\n", ERPOS, "" );
         }
      }
      printf( "EE> " );
   }
}
*/
