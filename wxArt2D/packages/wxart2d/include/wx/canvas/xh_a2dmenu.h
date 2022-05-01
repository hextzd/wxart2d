/////////////////////////////////////////////////////////////////////////////
// Name:        xh_a2dmenu.h
// Purpose:     XML resource handler for a2dmenus/a2dmenubars
// Author:      Vaclav Slavik, Klaas Holwerda, Leo Kadisoff
// Created:     2006/05/19
// RCS-ID:      $Id: xh_a2dmenu.h,v 1.3 2009/09/26 20:40:32 titato Exp $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_A2DMENU_H_
#define _WX_XH_A2DMENU_H_

#include "wx/xrc/xmlres.h"
#include "wx/xrc/xh_menu.h"
#include "wx/xrc/xh_toolb.h"
#include "wx/canvas/candefs.h"

class A2DCANVASDLLEXP a2dMenuXmlHandler : public wxMenuXmlHandler
{
    DECLARE_DYNAMIC_CLASS( a2dMenuXmlHandler )
public:
    a2dMenuXmlHandler();
    virtual wxObject* DoCreateResource();
    virtual bool CanHandle( wxXmlNode* node );
protected:
    bool m_insideA2DMenu;
};

class A2DCANVASDLLEXP a2dMenuBarXmlHandler : public wxMenuBarXmlHandler
{
    DECLARE_DYNAMIC_CLASS( a2dMenuBarXmlHandler )
public:
    a2dMenuBarXmlHandler();
    virtual wxObject* DoCreateResource();
    virtual bool CanHandle( wxXmlNode* node );
    static wxFrame* m_parentFrame;
};

class A2DCANVASDLLEXP a2dToolBarXmlHandler : public wxToolBarXmlHandler
{
    DECLARE_DYNAMIC_CLASS( a2dToolBarXmlHandler )

public:
    a2dToolBarXmlHandler();
    virtual wxObject* DoCreateResource();
    virtual bool CanHandle( wxXmlNode* node );

protected:
    bool m_isInside;
    wxToolBar* m_toolbar;
};


#endif // _WX_XH_MENU_H_
