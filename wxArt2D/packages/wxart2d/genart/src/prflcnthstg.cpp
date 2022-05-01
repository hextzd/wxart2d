/*! \file general/src/prflcnthstg.cpp
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflcnthstg.cpp,v 1.2 2005/06/27 19:52:43 titato Exp $
*/

///////////////////////////////////////////////////////////////////////////////
//
//  .h Includedateien
//  In diesen Dateien darf kein Aufruf von new erfolgen
//
///////////////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/genart/prfl.h"

#include "wx/genart/prflcnthstg.h"

///////////////////////////////////////////////////////////////////////////////
//
//  .inl Includedateien
//  In diesen Dateien darf kein new Operator deklariert werden
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Speicherschutz unter Debug fuer Implemenatation
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//*****************************************************************************
//*
//*  Klasse a2dProfilerHistogram: Erzeugungs und Vernichtungsoperatoren
//*
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
//  Standard-Erzeugungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerHistogram::a2dProfilerHistogram( wxString idIn, int hstgMinIn, int hstgMaxIn )
    : a2dProfiler( idIn )
{
    hstgMin = hstgMinIn;
    hstgMax = hstgMaxIn;

    cntLess = 0;
    cntAbv = 0;
    cnt = new unsigned long[hstgMax - hstgMin + 1];
    for( int i = hstgMin; i <= hstgMax; i++ ) cnt[i - hstgMin] = 0;
    wxASSERT( cnt );
}

///////////////////////////////////////////////////////////////////////////////
//
//  Vernichtungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerHistogram::~a2dProfilerHistogram ()
{
    OpenPrflFile();

    if( !prflFile.IsOpened() )
        return;

    unsigned int
    intg = 0;
    unsigned int
    n = 0;
    double
    sum = 0,
    sumSqr = 0;

    wxString stro;

    intg += cntLess;
    stro.Printf( wxT( "%-20s|%10d|intg: %10d| unterhalb\n" ),
                 id.c_str(),
                 cntLess,
                 intg
               );
    prflFile.Write( stro );


    for( int i = hstgMin; i <= hstgMax; i++ )
    {
        intg += cnt[i - hstgMin];
        n += cnt[i - hstgMin];
        sum += cnt[i - hstgMin] * double( i );
        sumSqr += cnt[i - hstgMin] * double( i ) * double( i );
        stro.Printf( wxT( "%-20s|%10d|intg: %10d|%10d\n" ),
                     id.c_str(),
                     cnt[i - hstgMin],
                     intg,
                     i
                   );
        prflFile.Write( stro );

    }

    intg += cntAbv;
    stro.Printf( wxT( "%-20s|%10d|intg: %10d|  oberhalb\n" ),
                 id.c_str(),
                 cntAbv,
                 intg
               );
    prflFile.Write( stro );

    stro.Printf( wxT( "%-20s|Summe x           %20.3lf\n" ),
                 id.c_str(),
                 sum
               );
    prflFile.Write( stro );
    stro.Printf( wxT( "%-20s|Mittelwert x      %20.3lf\n" ),
                 id.c_str(),
                 sum / n
               );
    prflFile.Write( stro );
    stro.Printf( wxT( "%-20s|Mittelwert x^2    %20.3lf\n\n" ),
                 id.c_str(),
                 sumSqr / n
               );
    prflFile.Write( stro );

    wxASSERT( cnt );

    delete [] cnt;
}

//*****************************************************************************
//*
//*  Klasse a2dProfilerHistogram: Zugriffsfunktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfilerHistogram: Verarbeitungsfunktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfilerHistogram: interne Funktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfilerHistogram: statische Variablen
//*
//*****************************************************************************
