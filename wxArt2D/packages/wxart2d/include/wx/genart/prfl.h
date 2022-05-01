/*! \file wx/genart/prfl.h
    \brief profiling classes
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prfl.h,v 1.5 2005/08/16 16:47:52 titato Exp $
*/


#if(defined(TRACE_H))
#pragma message(__FILE__)
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/ffile.h>

///////////////////////////////////////////////////////////////////////////////
//
//  Schutz gegen doppelte Einbindung
//
///////////////////////////////////////////////////////////////////////////////

#if(!defined(Prfl_H))
#define Prfl_H

#include "wx/general/smrtptr.h"

//! base profiler class
//!
//!  Zusammenfassung der Anwendungsdetails der Klasse a2dProfiler
//!
//!  a2dProfiler verwaltet die Profile-Ausgabedatei profile.txt
//!
//!  Mit PRFL_DUMP(...) kann man wie mit printf in die Profile-Datei schreiben
//!
//!  Profiling ist nur eingeschaltet, wenn das macro PRFL_ENBL definiert ist
//!  oder wenn die Datei PrflEnbl.h includiert wird.
//!
//!  siehe auch:
//!    prflCnt.h
//!    prflTmr.h
class A2DGENERALDLLEXP a2dProfiler
{
public:

    //! constructor
    a2dProfiler( const wxString& idIn );

    //! destructor
    ~a2dProfiler ();

    //! opens the profiling data file, if not yet open.
    static void OpenPrflFile();


protected:
    wxString
    id;

    //!Number of aktive PrflCnt ( the last one closes the file. )
    static int nmbrActv;
    static wxFFile prflFile; //! output file
};

#if(defined(PRFL_ENBL))
#define PRFL_DUMP a2dProfiler::PrflDump
#else
#define PRFL_DUMP 1 ? (void)0 : a2dProfiler::PrflDump
#endif

#endif // Prfl_H
