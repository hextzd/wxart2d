/*! \file editor/samples/viewmul/view.cpp
    \brief View classes
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: view.cpp,v 1.4 2006/12/13 21:43:21 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "view.h"
#include "docv.h"
#include "docframe.h"









IMPLEMENT_DYNAMIC_CLASS( a2dMultiDrawingConnector, a2dFrameViewConnector )

BEGIN_EVENT_TABLE( a2dMultiDrawingConnector, a2dFrameViewConnector )
    EVT_POST_CREATE_DOCUMENT( a2dMultiDrawingConnector::OnPostCreateDocument )
END_EVENT_TABLE()

a2dMultiDrawingConnector::a2dMultiDrawingConnector()
    : a2dFrameViewConnector( )
{

}

a2dMultiDrawingConnector::~a2dMultiDrawingConnector()
{
}

void a2dMultiDrawingConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();

    msDocFrame* viewFrame = new msDocFrame( templ,
                                            m_docframe, -1, wxT( "msDocument" ),
                                            GetInitialPosition(),
                                            GetInitialSize(),
                                            GetInitialStyle()
                                          );
    viewFrame->CreateViews( ( msDocument* ) event.GetDocument() );
    viewFrame->Show( true );
}
