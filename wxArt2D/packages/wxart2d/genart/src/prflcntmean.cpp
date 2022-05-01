/*! \file general/src/prflcntmean.cpp
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflcntmean.cpp,v 1.2 2005/06/27 19:52:43 titato Exp $
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

#include "wx/genart/prflcntmean.h"

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
//*  Klasse a2dProfilerMean: Erzeugungs und Vernichtungsoperatoren
//*
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
//  Standard-Erzeugungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerMean::a2dProfilerMean( wxString idIn )
    : a2dProfiler( idIn )
{
    cnt = 0;
    sum = 0;
    min = 0;
    max = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Vernichtungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerMean::~a2dProfilerMean ()
{
    OpenPrflFile();

    if( !prflFile.IsOpened() )
        return;
    wxString stro;
    stro.Printf( wxT( "%-20s|%10d|avg=%12g|max=%12g|min=%12g\n" ),
                 id.c_str(),
                 cnt,
                 sum / cnt,
                 max,
                 min
               );
    prflFile.Write( stro );
}

//*****************************************************************************
//*
//*  Klasse a2dProfilerMean: Zugriffsfunktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfilerMean: Verarbeitungsfunktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfilerMean: interne Funktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfilerMean: statische Variablen
//*
//*****************************************************************************
