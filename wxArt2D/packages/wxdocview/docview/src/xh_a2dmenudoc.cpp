/////////////////////////////////////////////////////////////////////////////
// Name:        xh_a2dmenudoc.cpp
// Purpose:     XRC resource for a2dmenus and a2dmenubars
// Author:      Vaclav Slavik, Klaas Holwerda, Leo Kadisoff
// Created:     2006/05/19
// RCS-ID:      $Id: xh_a2dmenu.cpp,v 1.7 2009/09/30 19:17:00 titato Exp $
// Copyright:   (c) 2006 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "docviewprec.h"
#include <wx/xml/xml.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/frame.h"
#include "wx/menu.h"
#endif

#if wxUSE_XRC

#include "wx/xrc/xh_menu.h"

#include "wx/general/comevt.h"
#include "wx/docview/xh_a2dmenudoc.h"

IMPLEMENT_DYNAMIC_CLASS( a2dMenuXmlHandlerDoc, wxMenuXmlHandler )

a2dMenuXmlHandlerDoc::a2dMenuXmlHandlerDoc() :
    wxMenuXmlHandler()
{
}

wxObject* a2dMenuXmlHandlerDoc::DoCreateResource()
{
    if ( m_class == wxT( "a2dMenu" ) || m_class == wxT( "wxMenu" ) )
    {
        wxMenu* menu = new wxMenu( GetStyle() );
        wxString title = GetText( wxT( "label" ) );
        wxString help = GetText( wxT( "help" ) );

        bool oldins = m_insideA2DMenu;
        m_insideA2DMenu = true;
        CreateChildren( menu, true/*only this handler*/ );
        m_insideA2DMenu = oldins;

        wxMenuBar* p_bar = wxDynamicCast( m_parent, wxMenuBar );
        if ( p_bar )
            p_bar->Append( menu, title );
        else
        {
            wxMenu* p_menu = wxDynamicCast( m_parent, wxMenu );
            if ( p_menu )
            {
                p_menu->Append( GetID(), title, menu, help );
                if ( HasParam( wxT( "enabled" ) ) )
                    p_menu->Enable( GetID(), GetBool( wxT( "enabled" ) ) );
            }
        }

        return menu;
    }





    else if ( m_class == wxT( "a2dMenuIdItem" ) )
    {
        wxMenu* p_menu = wxDynamicCast( m_parent, wxMenu );

        int id = GetID();
        wxString label = GetText( wxT( "label" ) );
        wxString accel = GetText( wxT( "accel" ), false );
        wxString help = GetText( wxT( "help" ) );

        wxString a2dCmdIdName = GetName();// = GetText(wxT("a2dname"));
        const a2dMenuIdItem& a2dmenu = a2dMenuIdItem::GetItemByName( a2dCmdIdName );
        if( &a2dmenu == &a2dMenuIdItem::sm_noCmdMenuId )
        {
            wxString error = _T( "a2dMenuIdItem with id name: " ) + a2dCmdIdName + _T( " not know" );
            wxMessageBox( error, _( "XRC problem" ), wxICON_INFORMATION | wxOK );
            return NULL;
        }
        if ( label.IsEmpty() )
            label = a2dmenu.GetLabel();
        if ( help.IsEmpty() )
            help = a2dmenu.GetHelp();

        wxItemKind kind = a2dmenu.GetKind();

        wxString fullLabel = label;
        if ( !accel.IsEmpty() )
            fullLabel << wxT( "\t" ) << accel;

        id = a2dmenu.GetId();
        wxMenuItem* mitem = new wxMenuItem( p_menu, id, fullLabel, help, kind );

#if (!defined(__WXMSW__) && !defined(__WXPM__)) || wxUSE_OWNER_DRAWN
        if ( HasParam( wxT( "bitmap" ) ) )
            mitem->SetBitmap( GetBitmap( wxT( "bitmap" ), wxART_MENU ) );
#endif
        if ( a2dMenuBarXmlHandlerDoc::m_parentFrame )
        {
            a2dDocumentFrame* parentFrame = wxDynamicCast( a2dMenuBarXmlHandlerDoc::m_parentFrame, a2dDocumentFrame );
            if ( parentFrame )
                parentFrame->AddCmdMenu( p_menu, mitem );
            a2dDocumentMDIChildFrame* mdiChildFrame = wxDynamicCast( a2dMenuBarXmlHandlerDoc::m_parentFrame, a2dDocumentMDIChildFrame );
            if ( mdiChildFrame )
                mdiChildFrame->AddCmdMenu( p_menu, mitem );
            a2dDocumentMDIParentFrame* mdiParentFrame = wxDynamicCast( a2dMenuBarXmlHandlerDoc::m_parentFrame, a2dDocumentMDIParentFrame );
            if ( mdiParentFrame )
                mdiParentFrame->AddCmdMenu( p_menu, mitem );
        }

        mitem->Enable( GetBool( wxT( "enabled" ), true ) );
        if ( kind == wxITEM_CHECK )
            mitem->Check( GetBool( wxT( "checked" ) ) );

        return NULL;
    }
    else
        return wxMenuXmlHandler::DoCreateResource();
}

bool a2dMenuXmlHandlerDoc::CanHandle( wxXmlNode* node )
{
    return IsOfClass( node, wxT( "a2dMenu" ) ) || IsOfClass( node, wxT( "wxMenu" ) ) ||
           ( m_insideA2DMenu &&
             ( IsOfClass( node, wxT( "a2dMenuIdItem" ) ) ||
               IsOfClass( node, wxT( "wxMenuItem" ) ) ||
               IsOfClass( node, wxT( "break" ) ) ||
               IsOfClass( node, wxT( "separator" ) ) )
           );
}


/********************************************************************
* a2dMenuBarXmlHandler
*********************************************************************/

IMPLEMENT_DYNAMIC_CLASS( a2dMenuBarXmlHandlerDoc, wxMenuBarXmlHandler )

wxFrame* a2dMenuBarXmlHandlerDoc::m_parentFrame = NULL;

a2dMenuBarXmlHandlerDoc::a2dMenuBarXmlHandlerDoc() : wxMenuBarXmlHandler()
{
}

wxObject* a2dMenuBarXmlHandlerDoc::DoCreateResource()
{
    wxFrame* curFrame = m_parentFrame;
    wxMenuBar* menubar = NULL;
    if( m_parentAsWindow )
    {
        wxFrame* parentFrame = wxDynamicCast( m_parent, wxFrame );
        if( parentFrame )
        {
            m_parentFrame = parentFrame;
            menubar = m_parentFrame->GetMenuBar();
            CreateChildren( menubar );
        }
    }

    if ( !menubar )
    {
        menubar = new wxMenuBar( GetStyle() );
        CreateChildren( menubar );
        if ( m_parentAsWindow && m_parentFrame )
        {
            m_parentFrame->SetMenuBar( menubar );
        }
    }

    m_parentFrame = curFrame;

    return menubar;
}

bool a2dMenuBarXmlHandlerDoc::CanHandle( wxXmlNode* node )
{
    return IsOfClass( node, wxT( "a2dMenuBar" ) ) || IsOfClass( node, wxT( "wxMenuBar" ) );
}

#endif // wxUSE_XRC

/********************************************************************
* a2dToolBarXmlHandler
*********************************************************************/

#if wxUSE_XRC && wxUSE_TOOLBAR

#include "wx/xrc/xh_toolb.h"

#ifndef WX_PRECOMP
#include "wx/frame.h"
#include "wx/toolbar.h"
#endif

IMPLEMENT_DYNAMIC_CLASS( a2dToolBarXmlHandlerDoc, wxToolBarXmlHandler )

a2dToolBarXmlHandlerDoc::a2dToolBarXmlHandlerDoc()
    : wxToolBarXmlHandler(), m_isInside( false ), m_toolbar( NULL )
{
}

wxObject* a2dToolBarXmlHandlerDoc::DoCreateResource()
{
    if ( m_class == wxT( "a2dToolCmd" ) )
    {
        wxCHECK_MSG( m_toolbar, NULL, wxT( "Incorrect syntax of XRC resource: tool not within a toolbar!" ) );

        int id = GetID();
        wxString a2dCmdIdName = GetName();
        const a2dMenuIdItem& a2dmenu = a2dMenuIdItem::GetItemByName( a2dCmdIdName );
        if( &a2dmenu == &a2dMenuIdItem::sm_noCmdMenuId )
        {
            wxString error = _T( "a2dToolCmd with id name: " ) + a2dCmdIdName + _T( " not know" );
            wxMessageBox( error, _( "XRC problem" ), wxICON_INFORMATION | wxOK );
            return NULL;
        }

        id = a2dmenu.GetId();
        wxString label = GetText( wxT( "label" ) );
        wxString help = GetText( wxT( "tooltip" ) );
        wxBitmap selBitmap = GetBitmap( wxT( "bitmap" ), wxART_TOOLBAR );
        if ( ! selBitmap.Ok() )
            selBitmap = a2dmenu.GetBitmap( false );

        wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + a2dCmdIdName;
        wxASSERT_MSG( selBitmap.Ok(), error );

        if ( label.IsEmpty() )
            label = a2dmenu.GetLabel();
        if ( help.IsEmpty() )
            help = a2dmenu.GetHelp();


        if ( GetPosition() != wxDefaultPosition )
        {
            m_toolbar->InsertTool( GetPosition().x, id, label,
                                   selBitmap,
                                   GetBitmap( wxT( "bitmap2" ), wxART_TOOLBAR ),
                                   GetBool( wxT( "toggle" ) ) ? wxITEM_CHECK  : wxITEM_NORMAL,
                                   help,
                                   GetText( wxT( "longhelp" ) ) );
        }
        else
        {
            wxItemKind kind = wxITEM_NORMAL;
            if ( GetBool( wxT( "radio" ) ) )
                kind = wxITEM_RADIO;
            if ( GetBool( wxT( "toggle" ) ) )
            {
                wxASSERT_MSG( kind == wxITEM_NORMAL,
                              _T( "can't have both toggleable and radion button at once" ) );
                kind = wxITEM_CHECK;
            }
            m_toolbar->AddTool( id,
                                label,
                                selBitmap,
                                GetBitmap( wxT( "bitmap2" ), wxART_TOOLBAR ),
                                kind,
                                help,
                                GetText( wxT( "longhelp" ) ) );

            if ( GetBool( wxT( "disabled" ) ) )
                m_toolbar->EnableTool( GetID(), false );
        }


        if ( a2dToolBarXmlHandlerDoc::m_parentAsWindow )
        {
            a2dDocumentFrame* parentFrame = wxDynamicCast( a2dMenuBarXmlHandlerDoc::m_parentFrame, a2dDocumentFrame );
            if ( parentFrame )
                parentFrame->ConnectCmdId( a2dmenu );
            /*
                        a2dDocumentMDIChildFrame *mdiChildFrame = wxDynamicCast(a2dMenuBarXmlHandler::m_parentFrame, a2dDocumentMDIChildFrame);
                        if (mdiChildFrame)
                            mdiChildFrame->ConnectCmdId( a2dmenu );
                        a2dDocumentMDIParentFrame *mdiParentFrame = wxDynamicCast(a2dMenuBarXmlHandler::m_parentFrame, a2dDocumentMDIParentFrame);
                        if (mdiParentFrame)
                            mdiParentFrame->ConnectCmdId( a2dmenu );
            */
        }

        return m_toolbar; // must return non-NULL
    }
    else if ( m_class == wxT( "a2dToolBar" ) )
    {
        int style = GetStyle( wxT( "style" ), wxNO_BORDER | wxTB_HORIZONTAL );
#ifdef __WXMSW__
        if ( !( style & wxNO_BORDER ) ) style |= wxNO_BORDER;
#endif

        XRC_MAKE_INSTANCE( toolbar, wxToolBar )

        toolbar->Create( m_parentAsWindow,
                         GetID(),
                         GetPosition(),
                         GetSize(),
                         style,
                         GetName() );
        SetupWindow( toolbar );

        wxSize bmpsize = GetSize( wxT( "bitmapsize" ) );
        if ( !( bmpsize == wxDefaultSize ) )
            toolbar->SetToolBitmapSize( bmpsize );
        wxSize margins = GetSize( wxT( "margins" ) );
        if ( !( margins == wxDefaultSize ) )
            toolbar->SetMargins( margins.x, margins.y );
        long packing = GetLong( wxT( "packing" ), -1 );
        if ( packing != -1 )
            toolbar->SetToolPacking( packing );
        long separation = GetLong( wxT( "separation" ), -1 );
        if ( separation != -1 )
            toolbar->SetToolSeparation( separation );

        wxXmlNode* children_node = GetParamNode( wxT( "object" ) );
        if ( !children_node )
            children_node = GetParamNode( wxT( "object_ref" ) );

        if ( children_node == NULL ) return toolbar;

        m_isInside = true;
        m_toolbar = toolbar;

        wxXmlNode* n = children_node;

        while ( n )
        {
            if ( ( n->GetType() == wxXML_ELEMENT_NODE ) &&
                    ( n->GetName() == wxT( "object" ) || n->GetName() == wxT( "object_ref" ) ) )
            {
                wxObject* created = CreateResFromNode( n, toolbar, NULL );
                wxControl* control = wxDynamicCast( created, wxControl );
                if ( !IsOfClass( n, wxT( "tool" ) ) &&
                        !IsOfClass( n, wxT( "a2dToolCmd" ) ) &&
                        !IsOfClass( n, wxT( "separator" ) ) &&
                        control != NULL )
                    toolbar->AddControl( control );
            }
            n = n->GetNext();
        }

        m_isInside = false;
        m_toolbar = NULL;

        toolbar->Realize();

        if ( m_parentAsWindow && !GetBool( wxT( "dontattachtoframe" ) ) )
        {
            wxFrame* parentFrame = wxDynamicCast( m_parent, wxFrame );
            if ( parentFrame )
                parentFrame->SetToolBar( toolbar );
        }

        return toolbar;
    }
    else
        return a2dToolBarXmlHandlerDoc::DoCreateResource();

}

bool a2dToolBarXmlHandlerDoc::CanHandle( wxXmlNode* node )
{
    return ( ( !m_isInside && IsOfClass( node, wxT( "a2dToolBar" ) ) ) ||
             ( m_isInside && IsOfClass( node, wxT( "a2dToolCmd" ) ) ) ||
             wxToolBarXmlHandler::CanHandle( node ) );
}

#endif // wxUSE_XRC && wxUSE_TOOLBAR
