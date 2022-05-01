/*! \file general/src/prfl.cpp
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prfl.cpp,v 1.2 2005/06/27 19:52:43 titato Exp $
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
//*  Klasse a2dProfiler: Erzeugungs und Vernichtungsoperatoren
//*
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
//  Standard-Erzeugungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfiler::a2dProfiler( const wxString& idIn )
{
    id = idIn;
    nmbrActv++;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Vernichtungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfiler::~a2dProfiler ()
{
    nmbrActv--;
    if( !nmbrActv )
    {
    }
}

//*****************************************************************************
//*
//*  Klasse a2dProfiler: Zugriffsfunktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfiler: Verarbeitungsfunktionen
//*
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
//  Öffnet die Profile-Datei, wenn sie noch nicht offen ist
//
///////////////////////////////////////////////////////////////////////////////

void a2dProfiler::OpenPrflFile()
{
    if( !prflFile.IsOpened() )
    {
        prflFile.Open( wxT( "Profile.log" ), wxT( "a" ) );
        if ( prflFile.IsOpened() )
        {
            wxString str = wxT( "\n\nProfile-Id          |Count     |Total Gigacycles|Avg. Cycles/call\n" );
            prflFile.Write( str );
        }
    }
}

//*****************************************************************************
//*
//*  Klasse a2dProfiler: interne Funktionen
//*
//*****************************************************************************

//*****************************************************************************
//*
//*  Klasse a2dProfiler: statische Variablen
//*
//*****************************************************************************

// Bei der Initialisierung verläßt man sich auf die 0-Vorinitialisierung

int a2dProfiler::nmbrActv;
