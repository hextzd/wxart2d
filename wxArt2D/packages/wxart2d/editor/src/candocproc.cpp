/*! \file editor/src/candocproc.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: candocproc.cpp,v 1.105 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/general/gen.h"

#if wxART2D_USE_CANEXTOBJ
#include "wx/canextobj/canextmod.h"
#endif //wxART2D_USE_CANEXTOBJ

#include "wx/editor/editmod.h"

#include <wx/tokenzr.h>
#include <wx/regex.h>

#if wxUSE_STD_IOSTREAM
#include "wx/ioswrap.h"
#if wxUSE_IOSTREAMH
#include <fstream.h>
#include <strstrea.h>
#else
#include <fstream>
#include <sstream>
#endif
#else
#include "wx/wfstream.h"
#endif

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dEditorModule, wxModule )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_ShowDlg, a2dCommand )

a2dEditorModule::a2dEditorModule()
{
    AddDependency( CLASSINFO( a2dFreetypeModule ) );
    AddDependency( CLASSINFO( a2dDocviewModule ) );
    AddDependency( CLASSINFO( a2dArtBaseModule ) );
    AddDependency( CLASSINFO( a2dCanvasModule ) );
}

bool a2dEditorModule::OnInit()
{
    return true;
}

void a2dEditorModule::OnExit()
{
}

