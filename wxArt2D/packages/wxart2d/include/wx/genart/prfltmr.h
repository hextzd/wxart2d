/*! \file wx/genart/prfltmr.h
    \brief profiling classes
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prfltmr.h,v 1.7 2005/08/16 16:47:52 titato Exp $
*/
//
//  Zusammenfassung der Anwendungsdetails der Klasse a2dProfilerTimer
//
//  a2dProfilerTimer dient dazu die Zeit, die ein Programm in einem bestimmten Block
//  verbringt zu messen. Der Vorteil gegnüber dem normalen Profiler liegt
//  darin, daß das ganze Programm nicht wesentlich gebremst wird. Außerdem kann
//  man da messen, wo man es für richtig hält, und nicht nur an Funktionsen. Die
//  Anwendung ist denkbar einfach. Der zu profilende Block wird wie folgt
//  geklammert:
//
//  PRFL_BGN_TMR(InteressanteSchleife)
//
//  for(int i=0; i<100; i++) {
//    viel Rechnen
//  }
//
//  PRFL_END_TMR(InteressanteSchleife)
//
//  a2dProfilerTimer zählt auch die Anzahl der Aufrufe von PRFL_BNG_TMR, so daß auch die
//  Zeit pro Aufruf berechnet werden kann.
//  Die Profileinformation wird am Programmende automatisch in die Datei Profile.txt
//  gesichert.
//
//  Wenn auch die Zahl der Durchläufe in der Schleife interessieren, kann man
//  in der Schleife PRFL_CNT(Anzahl_Durchlaeufe) angeben.
//
//  Die Argumente in den Klammern der Makros müssen gültige C-Identifier sein.
//  Dem Identifier wird das Präfix prfl_ vorgestellt. Mit diesem Präfix muß
//  er in der Funktion eindeutig sein.
//  Der Identifier sollte nicht länger als 20 Zeichen sein
//
//  Profiling ist nur eingeschaltet, wenn das macro PRFL_ENBL definiert ist
//  oder wenn die Datei PrflEnbl.h includiert wird.
//
//  siehe auch:
//    prflCnt.h
//    prflCntTmr.h
//
///////////////////////////////////////////////////////////////////////////////

#if(defined(TRACE_H))
#pragma message(__FILE__)
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  Schutz gegen doppelte Einbindung
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(PrflTmr_H))
#define PrflTmr_H

///////////////////////////////////////////////////////////////////////////////
//
//  Test der vorausgesetzten include Dateien
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(Prfl_H))
#error Please include Prfl.h before including PrflTmr.h
#endif

//! profile timer
class A2DGENERALDLLEXP a2dProfilerTimer : public a2dProfiler
{

public:

    //! constructor
    a2dProfilerTimer( const wxString& idIn );

    //! destructor
    ~a2dProfilerTimer ();

public:

    //! Help-Struct to automatically call EndBlck() when going out of scope.
    struct SPrflTmrAuto
    {
        //! constructor calls BgnBlck()
        SPrflTmrAuto( a2dProfilerTimer* prflIn ) {prfl = prflIn; prfl->BgnBlck();}
        //! destructor calls EndBlck()
        ~SPrflTmrAuto() {prfl->EndBlck();}

        //! timer on which this applies
        a2dProfilerTimer* prfl;
    };

public:

    //! is called at the start of a scope
    void BgnBlck();

    //! is called at the end of a scope
    void EndBlck();

protected:

    wxInt64 time;
    unsigned long cnt;
};


//*****************************************************************************
//*
//*  Definition of Macros
//*
//*****************************************************************************

#if(defined(PRFL_ENBL))
#define PRFL_BGN_TMR(id) static a2dProfilerTimer prfl_##id(_T(#id)); prfl_##id.BgnBlck();
#define PRFL_END_TMR(id) prfl_##id.EndBlck();
#define PRFL_AUTO_TMR(id) static a2dProfilerTimer prfl_auto1_##id(_T(#id)); a2dProfilerTimer::SPrflTmrAuto prfl_auto2_##id(&prfl_auto1_##id);
#else
#define PRFL_BGN_TMR(id)
#define PRFL_END_TMR(id)
#define PRFL_AUTO_TMR(id)
#endif

#endif // PrflTmr_H
