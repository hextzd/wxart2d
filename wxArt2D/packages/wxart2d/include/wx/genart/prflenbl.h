/*! \file wx/genart/prflenbl.h
    \brief profiling classes
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflenbl.h,v 1.6 2005/08/16 16:47:52 titato Exp $
*/
//
//  Zusammenfassung der Anwendungsdetails der Klasse a2dProfiler
//
//  Profiling ist nur eingeschaltet, wenn das macro PRFL_ENBL definiert ist
//  oder wenn die Datei PrflEnbl.h includiert wird.
//
//  siehe auch:
//    prflCnt.h
//    prflCntHstg.h
//    prflTmr.h
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

#if(!defined(PrflEnbl_H))
#define PrflEnbl_H

///////////////////////////////////////////////////////////////////////////////
//
//  Test der vorausgesetzten include Dateien
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(Prfl_H))
#error Please include 'prfl.h' before including prflenbl.h
#endif


//*****************************************************************************
//*
//*  Einschalten des Profiling
//*
//*****************************************************************************

#undef PRFL_DUMP
#define PRFL_DUMP a2dProfiler::PrflDump

#undef PRFL_CNT
#undef PRFL_CNT_DCLR
#undef PRFL_CNT_CNT
#define PRFL_CNT(id) {static a2dProfilerCount prfl_##id(#id); prfl_##id.Cnt();}
#define PRFL_CNT_DCLR(id) {static a2dProfilerCount prfl_##id(#id);}
#define PRFL_CNT_CNT(id) {prfl_##id.Cnt();}

#undef PRFL_CNT_HSTG
#define PRFL_CNT_HSTG(id,hstg,hstgMin,hstgMax) {static a2dProfilerHistogram prfl_##id(#id,hstgMin,hstgMax); prfl_##id.Cnt(hstg);}

#undef PRFL_CNT_MEAN
#define PRFL_CNT_MEAN(id,val) {static a2dProfilerMean prfl_##id(_T(#id)); prfl_##id.Cnt(val);}

#undef PRFL_BGN_TMR
#undef PRFL_END_TMR
#undef PRFL_AUTO_TMR
#define PRFL_BGN_TMR(id) static a2dProfilerTimer prfl_##id(_T(#id)); prfl_##id.BgnBlck();
#define PRFL_END_TMR(id) prfl_##id.EndBlck();
#define PRFL_AUTO_TMR(id) static a2dProfilerTimer prfl_auto1_##id(_T(#id)); a2dProfilerTimer::SPrflTmrAuto prfl_auto2_##id(&prfl_auto1_##id);

#undef PRFL_BGN_TMR_MIN
#undef PRFL_END_TMR_MIN
#define PRFL_BGN_TMR_MIN(id) static CPrflTmrMin prfl_##id(_T(#id)); prfl_##id.BgnBlck();
#define PRFL_END_TMR_MIN(id) prfl_##id.EndBlck();

#define PRFL_ENBL

#endif // PrflEnbl_H
