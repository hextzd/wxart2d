/*! \file wx/genart/prflcntmean.h
    \brief profiling classes
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflcntmean.h,v 1.7 2005/08/16 16:47:52 titato Exp $
*/


#if(defined(TRACE_H))
#pragma message(__FILE__)
#endif

#if(!defined(PrflCntMean_H))
#define PrflCntMean_H

///////////////////////////////////////////////////////////////////////////////
//
//  Test der vorausgesetzten include Dateien
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(Prfl_H))
#error Please include prfl.h before including prflcntmean.h
#endif

//! to find minimum and maximum values and sum of all values encountered in some pice of code.
/*!

    Zusammenfassung der Anwendungsdetails der Klasse a2dProfilerMean

    a2dProfilerMean dient dazu die Verteilung bestimmter Werte zu ermitteln. Die
    Anwendung ist denkbar einfach. An geeigneter Stelle fügt man folgendes Makro ein:

    PRFL_CNT_HSTG(HstgName,HstgWert,HstgMin,HstgMax);

    HstgMin und HstgMax sind die Grenzwerte für die Aufzeichnung. HstgWert ist
    der aktuelle Wert. HstgName dient zur Identifikation und muß ein gültiger
    C-Identifier sein. Dem Identifier wird das Präfix prfl_ vorgestellt.
    Mit diesem Präfix muß er in der Funktion eindeutig sein.
    Der Identifier sollte nicht länger als 20 Zeichen sein

    Die Profileinformation wird am Programmende automatisch in die Datei Profile.txt
    gesichert.

    Profiling ist nur eingeschaltet, wenn das macro PRFL_ENBL definiert ist
    oder wenn die Datei PrflEnbl.h includiert wird.

    siehe auch:
      prflTmr.h
      prflCnt.h
      prfl.h
*/
class A2DGENERALDLLEXP a2dProfilerMean : public a2dProfiler
{
public:
    // Standard Erzeugungsoperator
    a2dProfilerMean( wxString idIn );

public:
    // Wird bei jedem Durchlauf aufgerufen
    void Cnt( double val )
    {
        cnt++;
        sum += val;
        if( cnt )
        {
            if( val < min ) min = val;
            if( val > max ) max = val;
        }
        else
        {
            min = val;
            max = val;
        }
    }

public:
    // Vernichtungsoperator
    ~a2dProfilerMean ();

protected:

    int
    cnt;
    double
    sum;
    double
    min;
    double
    max;
};

//*****************************************************************************
//*
//*  Definition von Macros
//*
//*****************************************************************************

// man könnte die Deklaration und Cnt in {} einschließen, so daß der id nicht
// eindeutig zu sein bräuchte. Es schadet aber nix, wenn jeder id eindeutig ist

#if(defined(PRFL_ENBL))
#define PRFL_CNT_MEAN(id,val) {static a2dProfilerMean prfl_##id(_T(#id)); prfl_##id.Cnt(val);}
#else
#define PRFL_CNT_MEAN(id,val)
#endif

#endif // PrflCntMean_H
