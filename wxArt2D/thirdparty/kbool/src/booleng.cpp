/*! \file src/booleng.cpp
    \author Klaas Holwerda
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: booleng.cpp,v 1.7 2009/09/14 16:50:12 titato Exp $
*/

#include "kbool/booleng.h"

#include <time.h>

#include "kbool/link.h"
#include "kbool/line.h"
#include "kbool/node.h"
#include "kbool/graph.h"
#include "kbool/graphlst.h"

B_INT bmin( B_INT const value1, B_INT const value2 )
{
    return( ( value1 < value2 ) ? value1 : value2 );
}

B_INT bmax( B_INT const value1, B_INT const value2 )
{
    return( ( value1 > value2 ) ? value1 : value2 );
}

B_INT babs( B_INT a )
{
    if ( a < 0 ) a = -a;
    return a;
}

//-------------------------------------------------------------------/
//----------------- kbBool_Engine_Error -------------------------------/
//-------------------------------------------------------------------/

kbBool_Engine_Error::kbBool_Engine_Error( string message, string header, int degree, int fatal )
{
    _message = message;
    _header = header;

    _degree = degree;
    _fatal = fatal;

}

kbBool_Engine_Error::kbBool_Engine_Error( const kbBool_Engine_Error& a )
{
    _message = a._message;
    _header = a._header;

    _degree = a._degree;
    _fatal = a._fatal;

}

kbBool_Engine_Error::~kbBool_Engine_Error()
{
    _message = "";
    _header = "";
}

string kbBool_Engine_Error::GetErrorMessage()
{
    return _message;
}

string kbBool_Engine_Error::GetHeaderMessage()
{
    return _header;
}

int kbBool_Engine_Error::GetErrorDegree()
{
    return _degree;
}

int kbBool_Engine_Error::GetFatal()
{
    return _fatal;
}

//-------------------------------------------------------------------/
//----------------- kbBool_Engine -------------------------------------/
//-------------------------------------------------------------------/

kbBool_Engine::kbBool_Engine()
{
    _linkiter = new TDLI<kbLink>();
    m_intersectionruns = 1;

    m_orientationEntryMode = false;
    m_holePolygon = false;
    m_doLinkHoles = true;
    m_allowNonTopHoleLinking = true;

    m_graphlist = new kbGraphList( this );
    m_ACCUR = 1e-4;
    m_WINDINGRULE = true;
    m_GraphToAdd = NULL;
    m_firstNodeToAdd = NULL;
    m_lastNodeToAdd = NULL;

    m_logfile = NULL;
    m_doDebug = false;

#if KBOOL_LOG == 1
    SetLog( true );
#else
    SetLog( false );
#endif
}

kbBool_Engine::~kbBool_Engine()
{
    if ( m_logfile != NULL )
        fclose ( m_logfile );

    delete _linkiter;
    delete m_graphlist;
}

void kbBool_Engine::SetLog( bool OnOff )
{
    m_doLog = OnOff;
    if ( m_doLog )
    {
        if ( m_logfile == NULL )
        {
            // create a new logfile
            m_logfile = fopen( KBOOL_LOGFILE, "w" );
            if ( m_logfile == NULL )
                fprintf( stderr, "kbBool_Engine: Unable to write to Boolean Engine logfile\n" );
            else
            {
                time_t timer;
                struct tm * today;
                timer = time( NULL );
                today = localtime( &timer );

                //fprintf( m_logfile, "Logfile created on:\t\t\t%s", ctime( &timer ) );
            }
        }
    }
    else
    {
        if ( m_logfile != NULL )
        {
            fclose ( m_logfile );
            m_logfile = NULL;
        }
    }
}

void kbBool_Engine::SetDebug( bool OnOff )
{
    m_doDebug = OnOff;
}

void kbBool_Engine::SetState( string process )
{
    Write_Log( process );
}

void kbBool_Engine::error( string text, string title )
{
    Write_Log( "FATAL ERROR: ", title );
    Write_Log( "FATAL ERROR: ", text );
    throw kbBool_Engine_Error( text, title, 9, 1 );
};

void kbBool_Engine::info( string text, string title )
{
    Write_Log( "FATAL ERROR: ", title );
    Write_Log( "FATAL ERROR: ", text );
};

void kbBool_Engine::SetMarge( double marge )
{
    m_MARGE = marge;
    Write_Log( "kbBool_Engine::m_MARGE = %f\n", m_MARGE );
}

double kbBool_Engine::GetAccur()
{
    return m_ACCUR;
}

void kbBool_Engine::SetRoundfactor( double roundfac )
{
    m_ROUNDFACTOR = roundfac;
    Write_Log( "Bool_Engine::m_ROUNDFACTOR = %f\n", m_ROUNDFACTOR );
}

double kbBool_Engine::GetRoundfactor()
{
    return m_ROUNDFACTOR;
}

double kbBool_Engine::GetMarge()
{
    return m_MARGE;
}

void kbBool_Engine::SetDGrid( double dgrid )
{
    m_DGRID = dgrid;
    Write_Log( "kbBool_Engine::m_DGRID = %f\n", m_DGRID );
}

double kbBool_Engine::GetDGrid()
{
    return m_DGRID;
}

void kbBool_Engine::SetGrid( B_INT grid )
{
    m_GRID = grid;
    Write_Log( "kbBool_Engine::m_GRID = %lld\n", m_GRID );
}

B_INT kbBool_Engine::GetGrid()
{
    return m_GRID;
}

void kbBool_Engine::SetCorrectionAber( double aber )
{
    m_CORRECTIONABER = aber;
    Write_Log( "kbBool_Engine::m_CORRECTIONABER = %f\n", m_CORRECTIONABER );
}

double kbBool_Engine::GetCorrectionAber()
{
    return m_CORRECTIONABER;
}

void kbBool_Engine::SetCorrectionFactor( double aber )
{
    m_CORRECTIONFACTOR = aber;
    Write_Log( "kbBool_Engine::m_CORRECTIONFACTOR = %f\n", m_CORRECTIONFACTOR );
}

double kbBool_Engine::GetCorrectionFactor()
{
    return m_CORRECTIONFACTOR;
}

void kbBool_Engine::SetSmoothAber( double aber )
{
    m_SMOOTHABER = aber;
    Write_Log( "kbBool_Engine::m_SMOOTHABER = %f\n", m_SMOOTHABER );
}

double kbBool_Engine::GetSmoothAber()
{
    return m_SMOOTHABER;
}

void kbBool_Engine::SetMaxlinemerge( double maxline )
{
    m_MAXLINEMERGE = maxline;
    Write_Log( "kbBool_Engine::m_MAXLINEMERGE = %f\n", m_MAXLINEMERGE );
}

double kbBool_Engine::GetMaxlinemerge()
{
    return m_MAXLINEMERGE;
}

void kbBool_Engine::SetWindingRule( bool rule )
{
    m_WINDINGRULE = rule;
}

bool kbBool_Engine::GetWindingRule()
{
    return m_WINDINGRULE;
}


void kbBool_Engine::SetInternalMarge( B_INT marge )
{
    m_MARGE = ( double )marge / m_GRID / m_DGRID;
}

B_INT kbBool_Engine::GetInternalMarge()
{
    return ( B_INT ) ( m_MARGE * m_GRID * m_DGRID );
}

double kbBool_Engine::GetInternalCorrectionAber()
{
    return  m_CORRECTIONABER * m_GRID * m_DGRID;
}

double kbBool_Engine::GetInternalCorrectionFactor()
{
    return m_CORRECTIONFACTOR * m_GRID * m_DGRID;
}

double kbBool_Engine::GetInternalSmoothAber()
{
    return m_SMOOTHABER * m_GRID * m_DGRID;
}

B_INT kbBool_Engine::GetInternalMaxlinemerge()
{
    return ( B_INT ) ( m_MAXLINEMERGE * m_GRID * m_DGRID );
}

#define TRIALS 30

bool kbBool_Engine::Do_Operation( BOOL_OP operation )
{

    kbGraphList * saveme = NULL;
    if ( m_doDebug )
        saveme = new kbGraphList( m_graphlist );

    try
    {
        switch ( operation )
        {
            case ( BOOL_OR ):
                        case ( BOOL_AND ):
                            case ( BOOL_EXOR ):
                                case ( BOOL_A_SUB_B ):
                                    case ( BOOL_B_SUB_A ):
                                            m_graphlist->Boolean( operation, m_intersectionruns );
                break;
            case ( BOOL_CORRECTION ):
                            m_graphlist->Correction();
                break;
            case ( BOOL_MAKERING ):
                            m_graphlist->MakeRings();
                break;
            case ( BOOL_SMOOTHEN ):
                            m_graphlist->Smoothen( GetInternalSmoothAber() );
                break;
            default:
            {
                error( "Wrong operation", "Command Error" );
                return false;
            }
        }
    }
    catch ( kbBool_Engine_Error & error )
    {

        if ( m_doDebug )
        {
            delete m_graphlist;
            m_graphlist = new kbGraphList( saveme );
            m_graphlist->WriteGraphsKEY( this );
        }

        if ( m_logfile != NULL )
        {
            fclose ( m_logfile );
            m_logfile = NULL;
        }

        info( error.GetErrorMessage(), "error" );
        throw error;
    }
    catch( ... )
    {

        if ( m_doDebug )
        {
            delete m_graphlist;
            m_graphlist = new kbGraphList( saveme );
            m_graphlist->WriteGraphsKEY( this );
        }

        if ( m_logfile != NULL )
        {
            fclose ( m_logfile );
            m_logfile = NULL;
        }

        info( "Unknown exception", "error" );
        throw ;
    }

    if ( m_doDebug )
        delete saveme;

    return true;
}

bool kbBool_Engine::StartPolygonAdd( GroupType A_or_B )
{
    m_holePolygon = false;
    if ( m_logfile != NULL )
        fprintf( m_logfile, "-> StartPolygonAdd(%d)\n", A_or_B );
    if ( m_GraphToAdd != NULL )
        return false;

    kbGraph *myGraph = new kbGraph( this );
    //insert at end, in order to have holes inside contour being the next after contour
    m_graphlist->insend( myGraph );
    m_GraphToAdd = myGraph;
    m_groupType = A_or_B;

    return true;
}

bool kbBool_Engine::StartHolePolygonAdd( GroupType A_or_B )
{
    m_holePolygon = true;
    if ( m_logfile != NULL )
        fprintf( m_logfile, "-> StartPolygonAdd(%d)\n", A_or_B );
    if ( m_GraphToAdd != NULL )
        return false;

    kbGraph *myGraph = new kbGraph( this );
    //insert at end, in order to have holes inside contour being the next after contour
    m_graphlist->insend( myGraph );
    m_GraphToAdd = myGraph;
    m_groupType = A_or_B;

    return true;
}

bool kbBool_Engine::AddPoint( double x, double y, bool arcPiece )
{
    if ( m_GraphToAdd == NULL ){return false;}

    double scaledx = x * m_DGRID * m_GRID;
    double scaledy = y * m_DGRID * m_GRID;

    if ( scaledx > MAXB_INT  || scaledx < MINB_INT )
        error( "X coordinate of vertex to big", "" );
    if ( scaledy > MAXB_INT || scaledy < MINB_INT )
        error( "Y coordinate of vertex to big", "" );


    B_INT rintx = ( ( B_INT ) ( x * m_DGRID ) ) * m_GRID;
    B_INT rinty = ( ( B_INT ) ( y * m_DGRID ) ) * m_GRID;
    kbNode *myNode = new kbNode( rintx, rinty, this );

    // adding first point to graph
    if ( m_firstNodeToAdd == NULL )
    {
        if ( m_logfile != NULL )
        {
            fprintf( m_logfile, "-> AddPt() *FIRST* :" );
            fprintf( m_logfile, " input: x = %f, y = %f\n", x, y );
            fprintf( m_logfile, " input: x = %I64d, y = %I64d\n", rintx, rinty ) ;
        }

        m_firstNodeToAdd = ( kbNode * ) myNode;
        m_lastNodeToAdd  = ( kbNode * ) myNode;
    }
    else
    {
        if ( m_logfile != NULL )
        {
            fprintf( m_logfile, "-> AddPt():" );
            fprintf( m_logfile, " input: x = %f, y = %f\n", x, y );
            fprintf( m_logfile, " input: x = %I64d, y = %I64d\n", rintx, rinty ) ;
        }

        kbLink* added = m_GraphToAdd->AddLink( m_lastNodeToAdd, myNode );
        added->SetArcPiece( arcPiece );
        if (m_holePolygon) 
            added->SetHole( true );
        m_lastNodeToAdd = ( kbNode * ) myNode;
    }

    return true;
}

bool kbBool_Engine::EndPolygonAdd( bool arcPiece )
{
    if ( m_GraphToAdd == NULL ) {return false;}

    kbLink* added = m_GraphToAdd->AddLink( m_lastNodeToAdd, m_firstNodeToAdd );
    added->SetArcPiece( arcPiece );
    if (m_holePolygon) added->SetHole( true );
    m_GraphToAdd->SetGroup( m_groupType );
    m_GraphToAdd = NULL;
    m_lastNodeToAdd  = NULL;
    m_firstNodeToAdd = NULL;

    return true;
}

bool kbBool_Engine::StartPolygonGet()
{
    if ( !m_graphlist->empty() )
    {
        m_getGraph = ( kbGraph* ) m_graphlist->headitem();
        m_getLink  = m_getGraph->GetFirstLink();
        m_getNode  = m_getLink->GetBeginNode();
        m_numPtsInPolygon = m_getGraph->GetNumberOfLinks();
        m_numNodesVisited = 0;
        return true;
    }
    else
    {
        return false;
    }
}

bool kbBool_Engine::PolygonHasMorePoints()
{
    // see if first point
    if ( m_numNodesVisited == 0 )
    {
        // don't need to touch the m_getNode
        m_numNodesVisited++;
        return true;
    }

    if ( m_numNodesVisited < m_numPtsInPolygon )
    {
        // traverse to the next node
        m_getNode = m_getLink->GetOther( m_getNode );
        m_getLink = m_getLink->Forth( m_getNode );

        m_numNodesVisited++;
        return true;
    }
    else
    {
        return false;
    }
}

void kbBool_Engine::EndPolygonGet()
{
    m_graphlist->removehead();
    delete m_getGraph;
}

double kbBool_Engine::GetPolygonXPoint()
{
    return m_getNode->GetX() / m_GRID / m_DGRID;
}

double kbBool_Engine::GetPolygonYPoint()
{
    return m_getNode->GetY() / m_GRID / m_DGRID;
}

bool kbBool_Engine::GetHoleSegment()
{
    if ( m_getLink->GetHole() )
        return true;
    return false;
}

bool kbBool_Engine::GetHoleConnectionSegment()
{
    if ( m_getLink->GetHoleLink() )
        return true;
    return false;
}

kbEdgeType kbBool_Engine::GetPolygonPointEdgeType()
{
    // see if the point is the beginning of a false edge
    if ( m_getLink->GetHoleLink() )
        return KB_FALSE_EDGE;
    else
        // it's either an outside or inside edge
        if ( m_getLink->GetHole() )
            return KB_INSIDE_EDGE;
        else
            return KB_OUTSIDE_EDGE;
}

bool kbBool_Engine::GetArcPiece()
{
    if ( m_getLink->GetArcPiece() )
        return true;
    return false;
}

void kbBool_Engine::Write_Log( string msg1 )
{
    if ( m_logfile == NULL )
        return;

    fprintf( m_logfile, "%s \n", msg1.c_str() );
}

void kbBool_Engine::Write_Log( string msg1, string msg2 )
{
    if ( m_logfile == NULL )
        return;

    fprintf( m_logfile, "%s %s\n", msg1.c_str(), msg2.c_str() );
}

void kbBool_Engine::Write_Log( string fmt, double dval )
{
    if ( m_logfile == NULL )
        return;

    fprintf( m_logfile, fmt.c_str(), dval );
}

void kbBool_Engine::Write_Log( string fmt, B_INT bval )
{
    if ( m_logfile == NULL )
        return;

    fprintf( m_logfile, fmt.c_str(), bval );
}
