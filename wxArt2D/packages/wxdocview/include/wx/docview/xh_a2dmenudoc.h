/////////////////////////////////////////////////////////////////////////////
// Name:        xh_a2dmenudoc.h
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

#include "wx/docview/docviewref.h"
#include "wx/docview/docmdiref.h"

class A2DDOCVIEWDLLEXP a2dMenuXmlHandlerDoc : public wxMenuXmlHandler
{
    DECLARE_DYNAMIC_CLASS( a2dMenuXmlHandlerDoc )
public:
    a2dMenuXmlHandlerDoc();
    virtual wxObject* DoCreateResource();
    virtual bool CanHandle( wxXmlNode* node );
protected:
    bool m_insideA2DMenu;
};

class A2DDOCVIEWDLLEXP a2dMenuBarXmlHandlerDoc : public wxMenuBarXmlHandler
{
    DECLARE_DYNAMIC_CLASS( a2dMenuBarXmlHandlerDoc )
public:
    a2dMenuBarXmlHandlerDoc();
    virtual wxObject* DoCreateResource();
    virtual bool CanHandle( wxXmlNode* node );
    static wxFrame* m_parentFrame;
};

class A2DDOCVIEWDLLEXP a2dToolBarXmlHandlerDoc : public wxToolBarXmlHandler
{
    DECLARE_DYNAMIC_CLASS( a2dToolBarXmlHandlerDoc )

public:
    a2dToolBarXmlHandlerDoc();
    virtual wxObject* DoCreateResource();
    virtual bool CanHandle( wxXmlNode* node );

protected:
    bool m_isInside;
    wxToolBar* m_toolbar;
};


#endif // _WX_XH_MENU_H_
