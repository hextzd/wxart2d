/*! \file general/src/prfltmr.cpp
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prfltmr.cpp,v 1.3 2005/08/12 21:21:48 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/genart/prfl.h"
#include "wx/genart/prfltmr.h"

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
//*  Klasse a2dProfilerTimer: Erzeugungs und Vernichtungsoperatoren
//*
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
//  Standard-Erzeugungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerTimer::a2dProfilerTimer( const wxString& idIn )
    : a2dProfiler( idIn )
{
    cnt = 0;
    time = 0;
}

a2dProfilerTimer::~a2dProfilerTimer ()
{
    OpenPrflFile();

    if( !prflFile.IsOpened() )
        return;
    wxString stro;
    stro.Printf( wxT( "%-20s|%10lu|%16.6lf|%16.0lf\n" ),
                 id.c_str(),
                 cnt,
                 double( time ) * 1e-9,
                 double( time ) / cnt
               );
}

///////////////////////////////////////////////////////////////////////////////
//
// Wird am Anfang eines Blocks aufgerufen
//
///////////////////////////////////////////////////////////////////////////////

void a2dProfilerTimer::BgnBlck()
{
/*
// On fedora you defined(i386) could be used, but sus does not like it.
// _X86_ only detects mingw/cygwin.
#if defined(__GNUG__) && ( defined(_X86_))
    register __int64 TSC asm( "eax" );
    asm volatile ( "rdtsc" : : : "eax", "edx" );
    time -= TSC;
#elif defined(__VISUALC__)
    enum
    {
        timeOffs = offsetof( a2dProfilerTimer, time )
    };

    __asm
    {
        _emit 0x0F;
        _emit 0x31;
        sub [ecx+timeOffs], EAX
        sbb [ecx+timeOffs+4], EDX
    }
    cnt++;
#else
#warning Profile timer not (yet) supported on this system
#endif

*/
}

///////////////////////////////////////////////////////////////////////////////
//
// Wird am Ende eines Blocks aufgerufen
//
///////////////////////////////////////////////////////////////////////////////

void a2dProfilerTimer::EndBlck()
{
/*
#if defined(__GNUG__) && ( defined(_X86_))
    register __int64 TSC asm( "eax" );
    asm volatile ( "rdtsc" : : : "eax", "edx" );
    time += TSC;
#elif defined(__VISUALC__)
    enum
    {
        timeOffs = offsetof( a2dProfilerTimer, time )
    };
    __asm
    {
        _emit 0x0F;
        _emit 0x31;
        add [ecx+timeOffs], EAX
        adc [ecx+timeOffs+4], EDX
    }
#endif
*/
}

