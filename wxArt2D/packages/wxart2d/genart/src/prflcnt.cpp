/*! \file general/src/prflcnt.cpp
    \author Michael Sögtrop
    \date Created 08/05/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: prflcnt.cpp,v 1.4 2005/08/12 21:21:48 titato Exp $
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

#include "wx/genart/prflcnt.h"

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
//*  Klasse a2dProfilerCount: Erzeugungs und Vernichtungsoperatoren
//*
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
//  Standard-Erzeugungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerCount::a2dProfilerCount( wxString idIn )
    : a2dProfiler( idIn )
{
    cnt = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Vernichtungsoperator
//
///////////////////////////////////////////////////////////////////////////////

a2dProfilerCount::~a2dProfilerCount ()
{
    OpenPrflFile();

    if( !prflFile.IsOpened() )
        return;
    wxString stro;
    stro.Printf( wxT( "%-20s|%10.0lf|----------------|----------\n" ),
                 id.c_str(),
                 cnt
               );
    prflFile.Write( stro );
}

