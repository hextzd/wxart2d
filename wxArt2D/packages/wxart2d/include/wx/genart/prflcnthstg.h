/*! \file wx/genart/prflcnthstg.h
    \brief profiling classes
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflcnthstg.h,v 1.6 2005/08/16 16:47:52 titato Exp $
*/

///////////////////////////////////////////////////////////////////////////////

#if(defined(TRACE_H))
#pragma message(__FILE__)
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  Schutz gegen doppelte Einbindung
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(PrflCntHstg_H))
#define PrflCntHstg_H

///////////////////////////////////////////////////////////////////////////////
//
//  Test der vorausgesetzten include Dateien
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(Prfl_H))
#error Please include Prfl.h before including PrflCntHstg.h
#endif

//! histogram profiling
/*!
    Passing the Cnt() will fill the histrogram with the value of some integer variable.
    In the end you know which value was used how often.


    Zusammenfassung der Anwendungsdetails der Klasse a2dProfilerHistogram

    a2dProfilerHistogram dient dazu die Verteilung bestimmter Werte zu ermitteln. Die
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
class a2dProfilerHistogram : public a2dProfiler
{
public:
    //! constructor
    a2dProfilerHistogram( wxString idIn, int hstgMinIn, int hstgMaxIn );

    //! destructor
    ~a2dProfilerHistogram ();

public:

    //! called when pssing the code where it is placed.
    void Cnt( int hstg )
    {
        if( hstg < hstgMin ) cntLess++;
        else if( hstg > hstgMax ) cntAbv++;
        else cnt[hstg - hstgMin]++;
    }

protected:

    int
    hstgMin,
    hstgMax;
    unsigned long
    cntLess,
    cntAbv,
    *cnt;
};


//*****************************************************************************
//*
//*  Definition von Macros
//*
//*****************************************************************************

// man könnte die Deklaration und Cnt in {} einschließen, so daß der id nicht
// eindeutig zu sein bräuchte. Es schadet aber nix, wenn jeder id eindeutig ist

#if(defined(PRFL_ENBL))
#define PRFL_CNT_HSTG(id,hstg,hstgMin,hstgMax) {static a2dProfilerHistogram prfl_##id(#id,hstgMin,hstgMax); prfl_##id.Cnt(hstg);}
#else
#define PRFL_CNT_HSTG(id,hstg,hstgMin,hstgMax)
#endif

#endif // PrflCntHstg_H
