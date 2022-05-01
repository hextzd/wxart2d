/*! \file wx/genart/prflcnt.h
    \brief profiling classes
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflcnt.h,v 1.6 2005/08/16 16:47:52 titato Exp $
*/

#if(defined(TRACE_H))
#pragma message(__FILE__)
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  Schutz gegen doppelte Einbindung
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(PrflCnt_H))
#define PrflCnt_H

///////////////////////////////////////////////////////////////////////////////
//
//  Test der vorausgesetzten include Dateien
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(Prfl_H))
#error Please include prfl.h before including prflcnt.h
#endif

/*! profiling time

      Zusammenfassung der Anwendungsdetails der Klasse a2dProfilerCount

      a2dProfilerCount dient dazu die Anzahl der Durchläufe durch eine bestimmte
      Programmzeile zu zählen. Der Vorteil gegnüber dem normalen Profiler liegt
      darin, daß das ganze Programm nicht wesentlich gebremst wird. Außerdem kann
      man da messen, wo man es für richtig hält, und nicht nur an Funktionsen. Die
      Anwendung ist denkbar einfach. An der zu zählenden Stell wird das folgende
      Makro eingefügt:

      PRFL_CNT(InteressanteStelle)

      Möchte man mehrere Zähler an verschiedenen Stellen zusammenfassen, kann man
      den Zähler an einer Stelle mit dem folgenden Makro deklariern

      PRFL_CNT_DCLR(InteressanteStelle)

      und dann an mehreren Stellen mit dem folgenden Makro zählen

      PRFL_CNT_CNT(InteressanteStelle)

      Die Profileinformation wird am Programmende automatisch in die Datei Profile.txt
      gesichert.

      Die Argumente in den Klammern der Makros müssen gültige C-Identifier sein.
      Dem Identifier wird das Präfix prfl_ vorgestellt. Mit diesem Präfix muß
      er in der Funktion eindeutig sein.
      Der Identifier sollte nicht länger als 20 Zeichen sein

      Profiling ist nur eingeschaltet, wenn das macro PRFL_ENBL definiert ist
      oder wenn die Datei PrflEnbl.h includiert wird.

      siehe auch:
        prflCntHstg.h
        prflTmr.h
        prfl.h
*/
class a2dProfilerCount : public a2dProfiler
{
public:
    // Standard Erzeugungsoperator
    a2dProfilerCount( wxString idIn );

    // Wird bei jedem Durchlauf aufgerufen
    void Cnt() {cnt++;}

    // Vernichtungsoperator
    ~a2dProfilerCount ();

protected:
    double
    cnt;
};


//*****************************************************************************
//*
//*  Definition von Macros
//*
//*****************************************************************************

// man könnte die Deklaration und Cnt in {} einschließen, so daß der id nicht
// eindeutig zu sein bräuchte. Es schadet aber nix, wenn jeder id eindeutig ist

#if(defined(PRFL_ENBL))
#define PRFL_CNT(id) {static a2dProfilerCount prfl_##id( wxT(#id) ); prfl_##id.Cnt();}
#define PRFL_CNT_DCLR(id) {static a2dProfilerCount prfl_##id( wxT(#id) );}
#define PRFL_CNT_CNT(id) {prfl_##id.Cnt();}
#else
#define PRFL_CNT(id)
#define PRFL_CNT_DCLR(id)
#define PRFL_CNT_CNT(id)
#endif

#endif // PrflCnt_H
