/*! \file wx/editor/candocproc.h
    \brief holds the central a2dCentralCanvasCommandProcessor which is

    used for doing more advanged tasks from a central point in the application.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: candocproc.h,v 1.55 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __WXCANDOCPROC_H__
#define __WXCANDOCPROC_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/cmdproc.h"

#include "wx/canvas/edit.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/sttool2.h"
#include "wx/canvas/sttoolmes.h"
#include "wx/canvas/snap.h"
#if wxART2D_USE_CANEXTOBJ
#include "wx/canextobj/canextmod.h"
#include "wx/canextobj/imageioh.h"
#endif //wxART2D_USE_CANEXTOBJ
#include "wx/editor/cancom.h"


//--------------------------------------------------------------------
// a2dEditorModule
//--------------------------------------------------------------------

//! initiation editor module for the wxArt2D library
/*!
    \ingroup global
*/
class A2DEDITORDLLEXP a2dEditorModule : public wxModule
{
public:

    a2dEditorModule();

    virtual bool OnInit();
    virtual void OnExit();

private:
    DECLARE_DYNAMIC_CLASS( a2dEditorModule )

};

#endif

