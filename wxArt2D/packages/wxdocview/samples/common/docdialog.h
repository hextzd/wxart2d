/*! \file docview/samples/common/docdialog.h
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: docdialog.h,v 1.1 2007/01/17 21:47:33 titato Exp $
*/

#ifndef __DOCSH__
#define __DOCSH__

#include "wx/docview/docviewref.h"

class DocumentListDialog: public wxDialog
{
public:

    //! constructor.
    DocumentListDialog( wxFrame* parent, const a2dDocumentList& alldocuments );

    //! destructor.
    ~DocumentListDialog();

    //! Pointer to object selected.
    a2dDocument*  GetDocument();

    //! Close window if OK-button is pressed.
    void CmOk( wxCommandEvent& event );

    //! Close window if CANCEL-button is pressed.
    void CmCancel( wxCommandEvent& event );

    //! Close window if EXIT \-button is pressed.
    void OnCloseWindow( wxCloseEvent& event );

protected:

    //! Pointer to object list
    const a2dDocumentList*  m_documentlist;

    wxListBox* m_listbox;

    //! Pointer to the choosen object.
    a2dDocument*    m_object;

    //! Declare used events.
    DECLARE_EVENT_TABLE()
};

class ViewListDialog: public wxDialog
{
public:

    //! constructor.
    ViewListDialog( wxFrame* parent, const a2dViewList& allviews );

    //! destructor.
    ~ViewListDialog();

    //! Pointer to object selected.
    a2dView*  GetView();

    //! Close window if OK-button is pressed.
    void CmOk( wxCommandEvent& event );

    //! Close window if CANCEL-button is pressed.
    void CmCancel( wxCommandEvent& event );

    //! Close window if EXIT \-button is pressed.
    void OnCloseWindow( wxCloseEvent& event );

protected:

    //! Pointer to object list
    const a2dViewList*  m_viewlist;

    wxListBox* m_listbox;

    //! Pointer to the choosen object.
    a2dView* m_object;

    //! Declare used events.
    DECLARE_EVENT_TABLE()
};

#endif
