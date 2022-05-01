/*! \file wx/canvas/candoc.h
    \brief Contains a2dCanvasDocument Class to hold a drawing.

    a2dCanvasDocument is the class where a hierarchy of a2dCanvasObject's is stored.
    Updating pending/changed objects and drawing a document is all from this class.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: candoc.h,v 1.31 2009/07/10 19:23:13 titato Exp $
*/

#ifndef __WXCANDOC_H__
#define __WXCANDOC_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "a2dprivate.h"

#include "wx/docview/docviewref.h"

#include "wx/canvas/canobj.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/polygon.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/recur.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/cansim.h"
#include "wx/canvas/canvasogl.h"
#include "wx/canvas/drawing.h"

#include "wx/canvas/canprop.h"

#if wxART2D_USE_SVGIO
#include "wx/svgio/parssvg.h"
#endif //wxART2D_USE_SVGIO

#if wxART2D_USE_GDSIO
#include "wx/gdsio/gdsio.h"
#include "wx/gdsio/gdserr.h"
#endif //wxART2D_USE_GDSIO

class a2dCanvasCommandProcessor;
class a2dIOHandlerCVGIn;
class a2dIOHandlerCVGOut;
class a2dCameleon;

//----------------------------------------------------------------------------
// decls
//----------------------------------------------------------------------------

class A2DEDITORDLLEXP a2dViewPrintout;
class A2DEDITORDLLEXP a2dCanvasView;

#if wxUSE_PRINTING_ARCHITECTURE
//! to print what is displayed on a a2dCanvasView or the whole document as seen from the showobject of the drawer.
/*!
    Internal a second wxDarwer class is initiated with a mapping that nicely fits that mapping of the input
    a2dCanvasView to a piece of paper. This mapping depends on the type of print. If only what is on the view needs
    to be printed, the mapping of the input drawer is used, but if the whole document is wanted as seen
    from the ShowObject() of the input drawer, the boundingbox of the showobject will be used for the mapping.


    \ingroup docview
*/
class A2DEDITORDLLEXP a2dViewPrintout: public a2dDocumentPrintout
{
public:

    //!initialize mapping based on an existing canvas
    /*!
       \param drawer the a2dCanvasView from which the print is wanted.

       \param title title at top of the print

       \param filename the name of the file to be printed (may be empty )

       \param typeOfPrint When called from a2dDocumentCommandProcessor, the a2dCommand* which lead to this call.

        Depending on the command one can organize printing features.
        Like in the default implementation:
        \code
            a2dPrintWhat
            {
                Print,
                Preview,
                PrintView,
                PreviewView,
                PrintDocument,
                PreviewDocument,
                PrintSetup
            };
        \endcode

        Here View is to only print what is the visible view.
        Document print the document as seen from the
        a2dView::ShowObject(), it fits this to the paper.

        \param drawframe print a frame rectangle in bounding box of drawing/view
        \param scalelimit limits the scaling (world/pixel) to the given value, so that small graphics are not zoomed to full page
        \param fitToPage scale to fit the page
    */
    a2dViewPrintout( const wxPageSetupDialogData& pageSetupData, a2dCanvasView* drawer, const wxString& title, const wxString& filename, a2dPrintWhat typeOfPrint, bool drawframe, double scalelimit, bool fitToPage );

    //!destructor
    ~a2dViewPrintout( void );

    //! if set, printing is done via a bitmap which is drawn into, and next bitmap is printed.
    static void SetPrintAsBitmap( bool printAsBitmap ) { m_printAsBitmap = printAsBitmap; }

    //! if set, printing is done via a bitmap which is drawn into, and next bitmap is printed.
    static bool GetPrintAsBitmap() { return m_printAsBitmap; }

    //! called for every page to print, for a2dCanvasDocument in general just one.
    /*!
       It redraws/rerenders without double buffering the view or document on the pinter its wxDC.
       Internal a a2dDcDrawer is used to redraw the a2dCanvasDocument on the device.
    */
    bool OnPrintPage( int );

protected:

    const wxPageSetupDialogData& m_pageSetupData;

    //! type of print requested
    a2dPrintWhat m_typeOfPrint;

    //! maping defined by this canvas
    a2dCanvasView*  m_drawingView;

    //!title put above printout
    wxString    m_title;

    //!filename put below printout
    wxString    m_filename;

    //! limit scaling to this value (world/pixel)
    double m_scalelimit;

    //! draw a frame around the page
    bool m_drawframe;

    //! draw a view without real scale, the scaling in X and Y may differ.
    //! The drawing is adjusted in X and Y seperately to draw until the sides/border of the printer area
    bool m_fitToPage;

    //! if set print a bitmap that was drawn into
    static bool m_printAsBitmap;
};
#endif

//! view to display the size of a2dCanvasView compared to the whole of the a2dCanvasDocument that is viewed.
/*!
    \ingroup docview
*/
class A2DEDITORDLLEXP a2dZoomedView: public wxWindow
{
    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dZoomedView( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, a2dCanvasView* canvasview );

    //!destructor
    virtual ~a2dZoomedView();

protected:

    //! called for a2dComEvent events.
    void OnComEvent( a2dComEvent& event );

    //! \cond

    void OnUpdate( a2dDocumentEvent& event );

    //! repaint damaged araes.
    void OnPaint( wxPaintEvent& event );

    void OnRemoveView( a2dDocumentEvent& event );

    //! resize
    void OnSize( wxSizeEvent& event );

    //! \endcond

    //! the view to view zoomed
    a2dCanvasView* m_canvasDocview;

    DECLARE_CLASS( a2dZoomedView )

private:


};



//! View on a a2dCanvasDocument
/*!
    a2dCanvasView is a specialized view as connecting class to a2dCanvas via a2dViewCanvas.
    a2DrawingPart eventually display a part of a2dDrawing in a2dCanvas. This a2dDrawing is part of a a2dCanvasDocument.
    a2dCanvasDocument can contain a hierarchy of drawings.
    It depends on the a2dCanvas created by this view or via a derived a2dViewConnector class, what part of a drawing 
    and which drawing is displayed.
*/
class A2DEDITORDLLEXP a2dCanvasView: public a2dView
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dInitCurrentSmartPointerOwner m_initCurrentSmartPointerOwner;
#endif
    DECLARE_EVENT_TABLE()

public:


    //!constructor
    /*!
        \remark
        Do not forget to call SetDocument() if used standalone
        (in a a2dDocumentCommandProcessor setting this is taken care of).
    */
    a2dCanvasView( int width = 1000, int height = 1000 );

 
    //!constructor
    /*!
        \remark
        Do not forget to call SetDocument() if used standalone
        (in a a2dDocumentCommandProcessor setting this is taken care of)
    */
    a2dCanvasView( const wxSize& size );

    //!copy constructor
    a2dCanvasView( const a2dCanvasView& other );

    a2dDrawingPart* GetDrawingPart() const;

    a2dCanvasDocument* GetCanvasDocument() const;

    //!destructor
    virtual ~a2dCanvasView();

    //! Special event handling for a2dCanvasView class
    /*!
        Redirects events to the a2dViewCanvas.
    */
    virtual bool ProcessEvent( wxEvent& event );

	void OnCreateView( a2dViewEvent& event );

#if wxUSE_PRINTING_ARCHITECTURE
    //! to create a a2dViewPrintout, used to print a view or its document
    /*!
        The a2dViewPrintout created will take as much possible from the this view.
        a2dViewPrintout will create itself a view which fits the size of the paper, but important setting are taken from this view.
    */
    virtual wxPrintout* OnCreatePrintout( a2dPrintWhat typeOfPrint, const wxPageSetupDialogData& pageSetupData );
#endif

 
    //! Not implemented, use a2dViewPrintout to print
    /*!
       OnDraw is called when printing a view via wxView::OnPrint with wxDocPrintout::OnPrintPage.
       But for a2dCanvasView, i implemented it in a2dViewPrintout::OnPrintPage.

       This funcion is and should NOT be used to Redraw the a2dCanvasView views, this is taken care of automatically,
       and if needed can be forced with OnUpdate or Update.
       Paint events are intercepted to blit damaged parts caused by overlaping windows,
       and in Idle time changes or updated in a2dCanvasView.
    */
    virtual void OnDraw( wxDC* );

    //! If true render the printout with a title string, otherwise not
    void SetPrintTitle( bool val ) { m_printtitle = val; }

    //! If true render the printout with a filename string, otherwise not
    void SetPrintFilename( bool val ) { m_printfilename = val; }

    //! Set the scaling limit for printing, so that small stuff is not zoomed to full page
    void SetPrintScaleLimit( double val ) { m_printscalelimit = val; }

    //! If true, draw a frame around printouts
    void SetPrintFrame( bool val ) { m_printframe = val; }

    //! If true, draw a view on all page without real scale
    void SetPrintFitToPage( bool val ) { m_printfittopage = val; }

protected:

    //! if true, a printout is done with title (document name (description?)), otherwise not
    bool m_printtitle;

    //! if true, a printout is done with filename (document file path), otherwise not
    bool m_printfilename;

    //! Set the scaling limit for printing, so that small stuff is not zoomed to full page
    double m_printscalelimit;

    //! If true, draw a frame around printouts
    bool m_printframe;

    //! If true, draw a view on all page without real scale
    bool m_printfittopage;

public:

    DECLARE_DYNAMIC_CLASS( a2dCanvasView )

private:

    //initialize a drawer
    void Init();

private:
    virtual a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

class a2dDrawingId;
 
//!Each a2dCanvasView needs to have a a2dCanvasDocument set in order to render data.
/*!
    Several a2dCanvasView objects can share the same a2dCanvasDocument.
    The a2dCanvasDocument has one top a2dCanvasObject, called m_drawing.
    All other a2dCanvasObject's are added as children to this root object.
    The docuemnt also contains the layer settings for the canvas objects to be rendered.
    The a2dCanvasDocument is derived from a2dDocument, and it is used to store a hierarchy of a2dCanvasObject's.
    The updating of modified a2dCanvasObject's is also organized as part of this class. One can load a canvas document from
    a file, and save it to a file, in the CVG format. But other formats are possible, via the document its a2dDocumentTemplate and
    associated a2dIOHandlers.

    Rendering a a2dCanvasDocument is started at a given a2dCanvasObject,
    which needs to be part of the data structure below the a2dCanvasDocument.
    Since a2dCanvasDocument owns a a2dCanvasObject as root, rendering the complete document will start at the m_drawing.
    But any nested child a2dCanvasObject can be used to start rendering, and only that object and it children will be displayed
    on the a2dCanvasView ( and a2dCanvas which is the display window ).
    The a2dCanvasObject to be displayed on a certain a2dCanvasView is maintained in the
    a2dCanvasView object. It can be different for all a2dCanvasView objects sharing the same a2dCanvasDocument.

    a2dCanvasDocument senets update events when a2dCanvasObject have changed internal. The a2dView's using the document
    are intercepting the update events, and react by updating the data that they display. For a a2dCanvasView this means
    redrawing those parts that have changed.

    Any nested child from a a2dCanvasDocument Object can be displayed on several a2dCanvasView Objects at the same time.
    The child displayed does not have to be the same on each a2dCanvasView.
    The active a2dCanvasView is used to render and convert coordinates from world to device.
    So it is important to set the active a2dCanvasView based on the a2dCanvasView that has the focus
    or is scrolled etc. This is normally done within a2dCanvasView when appropriate.


    \remark Use a2dDocumentTemplate's to get the type of the file, where this data was read from.
            based on this the document can be saved in the same type as read from.

\sa a2dCanvasView
\sa a2dCanvas
\sa wxWorldCanvas
\sa a2dCanvasObject

    \ingroup docview canvasobject
*/
class A2DEDITORDLLEXP a2dCanvasDocument : public a2dDocument
{
public:

    //!Construct a  a2dCanvasDocument to store a complete drawing in
    /*!
        This class contains nested a2dCanvasObject's and other drawing object derived from a2dCanvasObject's.
        The class itself is derived from a2dDocument.
        All objects are stored are as childs of the m_drawing.
        A complete drawing is stored here, and displaying the drawing, is done by setting a pointer to this a2dCanvasDocument
        for a a2dCanvasView derived class.
        Every change inside the wxCavasObject's stored inside this class, will report themselfs as pending to an instance
        of this class.
        The a2dCanvasView classes, having this instance of a2dCanvasDocument set, will be updated from the document via update events.
        a2dCanvasObject that are changed or set pending. Those pending objects are reported to the document also. In Idle time the pending
        objects are redrawn on the a2dCanvasView's that display the document.
        The same a2dCanvasDocument can be displayed on reveral a2dCanvasView's at the same time.
        And therefore a pending a2dCanvasObject maybe be (re)displayed on several a2dCanvasView's also.
        The a2dCanvasObjects stored in a a2dCanvasDocument, will have their m_root member set in order to get to the a2dCanvasDocument
        in which they are stored. Through the a2dCanvasDocument they will be able to reach the a2dCanvasView class that is currently active.
        The a2dCanvasView class is set to the document when needed on a higher level. e.g. when rendering a a2dCanvasDocument.
        This way each a2dCanvasObject stored inside the a2dCanvasDocument, can draw itself on the active a2dCanvasView.
        The a2dCanvasView in this case can be seen as a sort of drawing context.
        Indirectly a a2dCanvasView can use a2dCanvas as the window/device to draw up on.
        Another type of a2dCanvasView can draw to a bitmap, image or printer device.

        \remark The initially layer setup is defined my a2dCanvasGlobal::GetLayerSetup

        \see SetLayerSetup for an example for introduce your own layers
    */
    a2dCanvasDocument();

    //! destructor
    ~a2dCanvasDocument();

    //! like it to be protected, but this does not work with wxList macros
    //void operator delete(void* recordptr);

    //! constructor with other document
    a2dCanvasDocument( const a2dCanvasDocument& other );

    void CreateCommandProcessor();

    //! get the root object, which holds the objects in the document.
    inline a2dDrawing* GetDrawing() const { return m_drawing; }

    //! set new root object ( may not ne NULL )
    void SetDrawing( a2dDrawing* newRoot );

    //! Sets a description of the document
    /*!
        A description may describe the content of this document or contain keywords etc.
        The default value is "a2dCanvasDocument generated by wxArt2D".

        \param desc description of the document
    */
    void SetDescription( const wxString& desc ) { m_description = desc; }

    //! Returns the description of the document
    /*!
        \see SetDescription

        \return the description
    */
    wxString& GetDescription() { return m_description; }

    //! to name the document as a library (independent of a path and filename )
    void SetLibraryName( const wxString& libraryname ) { m_libraryname = libraryname; }

    //! Get name the document as a library (independent of a path and filename )
    wxString& GetLibraryName() { return m_libraryname; }

    //! set version of library or document
    void SetVersion( wxString version ) { m_version = version; }

    //! get version of library or document
    wxString GetVersion() { return m_version; }

    //! set version of library or document being parsed for
    void SetFormatVersion( wxString formatVersion ) { m_formatVersion = formatVersion; }

    //! get version of library or document
    wxString GetFormatVersion() { return m_formatVersion; }

    //! if true, editing document is allowed
    void SetMayEdit( bool mayEdit ) { m_mayEdit = mayEdit; }

    //! if true, editing document is allowed
    bool GetMayEdit() const { return m_mayEdit; }

    //! delete all object in this document (also nested groups etc.)
    virtual void DeleteContents();

    //!write as SVG to a file starting at given object
    /*!
        \param drawer    drawing view context
        \param filename  filename of stream to write the SVG content to.
        \param top       start writting at this object.
        \param Width     width in unit will be the physical width of the drawing.
        \param Height    height in unit will be the physical height of the drawing.
        \param unit      Unit of width and height e.g. "cm" "um".
    */
    bool WriteSVG( a2dCanvasView* drawer, const wxString& filename, a2dCanvasObject* top, double Width, double Height, wxString unit );

    //!load form a file containing the a2dCanvas specific XML called CVG format
    virtual a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler );

#if wxART2D_USE_CVGIO
    //! used by CVG parser
    /*!
        \remark do not use directly
     */
    void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );
#endif //wxART2D_USE_CVGIO

    //! load document from a file with the given filename and given parser type
    bool Load( const wxString& filename, a2dIOHandlerStrIn* handler );

    //! save layer settings to CVG file
    bool SaveLayers( const wxString& filename );

    //! load layers from another file
    /*!
        Only the layer settings are read from the file and put in place of the
        current layers settings in the document.
    */
    bool LoadLayers( const wxString& filename );

    //! Update a loaded layer setup
    /*! This is called after loading a layer setup. The base class function does
        nothing. It is usefull to override this to update a loaded layer setup to
        e.g. a new software version */
    virtual void UpdateLoadedLayerSetup();

    //!save as the a2dCanvas specific XML called CVG format
    a2dDocumentOutputStream& SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler );

#if wxART2D_USE_CVGIO
    void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );

    //! called from a2dIOHandlerCVGOut after a SaveObject.
    /*!
        \param out CVG io handler
        \param start start wrting from this object
    */
    void Save( a2dIOHandlerCVGOut& out, a2dCanvasObject* start = 0 );
#endif //wxART2D_USE_CVGIO

    //! save document to a file with the given name and given output writer type
    bool Save( const wxString& filename,  a2dIOHandlerStrOut* handler ) const;

    //! update all pending objects in the document
    /*
        At the minumum all boudingboxes are brought up to date, and if a layersetup is set,
        in the end all layers used in the document are known.

        This function is part of the update cycle e.g. in AddPendingUpdatesOldNew()
    */
    void Update( a2dCanvasObject::UpdateMode mode );

    //!set the layersettings for the canvas.
    /*!
        A default Layer Setting is created in the constructor a2dCanvasDocument::a2dCanvasDocument

        An example how to set another layer setup
        \code
        // Clone the current layer setup
        a2dLayers* docLayers = doc->GetLayerSetup()->Clone( bool deep = true );

        // Create a new layer with an index above the predefined layers
        a2dLayerInfo* lr = new a2dLayerInfo(wxLAYER_USER_FIRST, "MyLayer");

        // Set layer available
        lr->Set_Available(true);

        // Add layer to to the a2dLayers
        docLayers->Append(lr);

        doc->SetLayerSetup(docLayers);
        \endcode

        \remark
        You don't have to call Set_UpdateAvailableLayers it will be done automatically

        \param layersetup the new layersetup
    */
    void SetLayerSetup( a2dLayers* layersetup );

    //!Get the layersettings for the canvas.
    /*!
        A default Layer Setting is taken from a2dCanvasGlobal.
        \sa a2dCanvasGlobal for central layers settings, used for initializing a a2dCanvasDocument
    */
    a2dLayers* GetLayerSetup() { return m_layersetup; }

    //!Set true if the document read from a file did not have on erootobject but several
    /*!
        Some formats as GDS-II and KEY, can contain many structure, which are not placed in
        a single parent object. Still in a a2dCanvasDocument they will be placed in the m_drawing.
        This flag can be used when writing the data back in the same way, skipping the rootobject if possible.
    */
    void SetMultiRoot( bool multiRoot = true ) { m_multiRoot = multiRoot; }

    //! \see SetMultiRoot()
    bool GetMultiRoot() { return m_multiRoot; }

    //! what is the top a2dcameleon in the drawing to display after opening a document, when m_start is not set.
    a2dCameleon* GetTopCameleon() { return m_main; }
    //! what is the top a2dcameleon in the drawing to display after opening a document, when m_start is not set.
    void SetTopCameleon( a2dCameleon* root ) { m_main = root; }

    //! what is the start object in document, in case of multiroot
    a2dCanvasObject* GetStartObject() { return m_start; }
    //! what is the start object in document, in case of multiroot
    void SetStartObject( a2dCanvasObject* start ) { m_start = start; }

    //! what to display when document is openened
    a2dCanvasObject* GetShowObject() { return m_show; }
    //! what to display when document is openened
    void SetShowObject( a2dCanvasObject* show ) { m_show = show; }

    //!this is the number that defines the number of decimal places in the fraction
    /*!when writing numbers to a file.
       When reading a file, this number is set to the accuracy information available in the file.
       1/GetUnitsAccuracy() == 100 means 2 decimal places will be written, or the data just
       read is accurate to this number of decimal places.

       \remark GetUnitsAccuracy() can also be > 1
    */
    double GetUnitsAccuracy() { return m_units_accuracy; }

    //!this is the number that defines the number of decimal places in the fraction
    /*!when writing numbers to a file.
       When reading a file, this number is set to the accuracy information available in the file.
       1/GetAccuracyUserUnits() == 100 means 2 decimal places will be written, or the data just
       read is accurate to this number of decimal places.
       \remark SetUnitsAccuracy() can also be > 1
    */
    void   SetUnitsAccuracy( double accuracy ) { m_units_accuracy = accuracy; }

    //!this is the number that defines the physical dimension in meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()
    */
    double GetUnitsScale() { return m_units_scale; }


    //!this is the number that defines the physical dimension in meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()
    */
    void   SetUnitsScale( double scale ) { m_units_scale = scale; }

    //! Normalize objects ( defined in coordinates -1 to 1 ranges ) will be multiplied by this factor.
    /*!
        Libraries containing object which are defined in normalized coordinates,
        need to be multiplied by a certain factor before they can be added to a document.
        This factor defines by how much that should be done.
        As an example a libary containing arrow objects for placing at the end of a2dEndsLine
        objects, will use this factor.
    */
    double GetNormalizeScale() { return m_normalize_scale; }

    //! see GetNormalizeScale()
    void SetNormalizeScale( double scale ) { m_normalize_scale = scale; }

    //!this string defines the unit e.g. meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()

        \remark the value may also be "trees" "pixels" or "people"  etc.

        \remark conversion to other units is not just simply changing this string.
        You must Change the SetUnitsScale(double) or really scale the data itself.
    */
    wxString GetUnits() { return m_units; }

    //!this string defines the unit e.g. meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()

        \remark the value may also be "trees" "pixels" or "people" etc.

        \remark conversion to other units is not just simply changing this string.
        You must Change the SetUnitsScale(double) or really scale the data itself.
    */
    void SetUnits( const wxString& unitString ) { m_units = unitString; }

    //! Sets the time of last access to this doc.
    /*!
        \param datatime new last access time
    */
    void SetAccessTime( const wxDateTime& datatime ) { m_accesstime = datatime; }

    //! Returns the time of last access.
    wxDateTime& GetAccessTime() { return m_accesstime; }

    //! gives time when a change was made to the document which may effect rendering
    /*!
        Comparing this time in a dialogs its own update time, one can easily keep them up to date.

        e.g. set in SetUpdatesPending()
    */
    wxDateTime& GetInternalChangedTime() { return m_changedInternalAccesstime; }

    DECLARE_DYNAMIC_CLASS( a2dCanvasDocument );

protected:

    //! Called by ProcessEvent(wxEvent& event) of document
    /*!
        Event processing is called by wxView and therefore a2dCanvasView.
        In a a2dDocumentCommandProcessor controller application wxDocChildFrame and wxDocParentFrame
        redirect events to wxView and a2dDocumentCommandProcessor.
        In a non a2dDocumentCommandProcessor the a2dCanvas is calling ProcessEvent(wxEvent& event) of document.
        Onidle updates all drawers if pending updates are available
    */
    void OnIdle( wxIdleEvent& event );

    //! uses wxBufferedInputStream instead of a2dDocumentInputStream
    void OnOpenDocument( a2dDocumentEvent& event );

    //! track modification of document
    void OnDoEvent( a2dCommandProcessorEvent& event );

    //! track modification of document
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    //! track modification of document
    void OnUndoEvent( a2dCommandProcessorEvent& event );

    //! called when a drawing in a document did change.
    void OnChangeDrawings( a2dDrawingEvent& event );

    //! called when new cameleon is added.
    void OnNewCameleon(  a2dCameleonEvent& event );

	a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

protected:

#if wxART2D_USE_CVGIO
    //! do not use directly, part of CVG format writing
    void DoSave( a2dIOHandlerCVGOut& out, a2dCanvasObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! start a2dCanvasObject containing all primitives
    a2dDrawingPtr m_drawing;

    //! what is the root a2dcameleon in the drawing to display after opening a document, when m_start is not set.
    a2dSmrtPtr<a2dCameleon> m_main;

    //! what is the start object in document, in case of multiroot
    a2dCanvasObjectPtr m_start;

    //! what to display when document is openened
    a2dCanvasObjectPtr m_show;

    //!setup for layers in this document
    /*!
        \remark will contain if a layer is available/visible for rendering
        \remark object without style will take the layer style to draw itself.
    */
    a2dSmrtPtr<a2dLayers>  m_layersetup;

    //! multi root document
    bool m_multiRoot;

    //! how many decimal places in fraction are valid
    double m_units_accuracy;

    //! scaling factor (how many "m_units_measure" is one unit)
    double m_units_scale;

    //! normalization factor to scale normalized objects.
    double m_normalize_scale;

    //! unit (e.g. Meters Inch Microns etc.)
    wxString m_units;

    //! measuring unit (e.g. Meters Inch Microns etc.)
    wxString m_units_measure;

    //! when was the document last accessed.
    wxDateTime m_accesstime;

    //! gives time when a change was made to the document which may effect rendering
    /*!
        e.g. set in SetUpdatesPending()
    */
    wxDateTime m_changedInternalAccesstime;

    wxString m_formatVersion;

    //! name of library or document
    wxString m_libraryname;

    //! description for document
    wxString m_description;

    //! version of library or document
    wxString m_version;

    //! if true, editing drawing is allowed
    bool m_mayEdit;

public:

    //! layer info changed id sent around when m_layersetup is changed.
    static const a2dSignal sig_layersetupChanged;

    //! when an object is removed from a layer,
    static const a2dSignal sig_changedLayer;

    DECLARE_EVENT_TABLE()

};

#if defined(WXART2D_USINGDLL)
template class A2DEDITORDLLEXP a2dSmrtPtr<a2dCanvasDocument>;
#endif

//!  A specific a2dCanvas and a2dCanvasView meet eachother here.
/*!
    a2dCanvasView will use this canvas window to display part of a document.
*/
template< class Tcanvas >
class a2dDocviewCanvas : public Tcanvas
{
    DECLARE_EVENT_TABLE()

public:

    a2dDocviewCanvas( a2dCanvasView* view, wxWindow* parent, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxScrolledWindowStyle, a2dDrawer2D* drawer2D = 0 );

    a2dDocviewCanvas( a2dCanvasView* view, a2dDrawingPart* drawingpart, wxWindow* parent, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxScrolledWindowStyle );

    a2dCanvasView* GetView() { return m_view; }

    void SetView( a2dCanvasView* view );

protected:

    //! called when document of the a2dView has changed.
    /*!
        Sets the a2dCanvasDocument where the objects for this canvas are stored
        It will trigger boundingbox calculation and other administrative tasks
        to properly render the document onto this view
    */
    void OnSetDocument( a2dViewEvent& event );

	void OnActivate(  a2dViewEvent& viewevent );

	void OnCloseView( a2dCloseViewEvent& event );

    //! calls a2dDrawingpart::Update()
    void OnUpdate( a2dDocumentEvent& event );

    a2dCanvasView* m_view;

};

typedef a2dDocviewCanvas< a2dCanvas > a2dViewCanvas;
typedef a2dDocviewCanvas< a2dCanvasSim > a2dViewCanvasSim;
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
typedef a2dDocviewCanvas< a2dOglCanvas > a2dViewCanvasOgl;
#endif //wxUSE_GLCANVAS 

#define wxBEGIN_EVENT_TABLE_TEMPLATE_B(theClass, baseclass) \
    template<typename baseclass> \
    const wxEventTable theClass<baseclass>::sm_eventTable = \
        { &baseclass::sm_eventTable, &theClass<baseclass>::sm_eventTableEntries[0] }; \
    template<typename baseclass> \
    const wxEventTable *theClass<baseclass>::GetEventTable() const \
        { return &theClass<baseclass>::sm_eventTable; } \
    template<typename baseclass> \
    wxEventHashTable theClass<baseclass>::sm_eventHashTable(theClass<baseclass>::sm_eventTable); \
    template<typename baseclass> \
    wxEventHashTable &theClass<baseclass>::GetEventHashTable() const \
        { return theClass<baseclass>::sm_eventHashTable; } \
    template<typename baseclass> \
    const wxEventTableEntry theClass<baseclass>::sm_eventTableEntries[] = { \

wxBEGIN_EVENT_TABLE_TEMPLATE_B( a2dDocviewCanvas, baseclass )
    EVT_SET_DOCUMENT( a2dDocviewCanvas::OnSetDocument )
    EVT_ACTIVATE_VIEW( a2dDocviewCanvas::OnActivate )
    EVT_CLOSE_VIEW( a2dDocviewCanvas::OnCloseView )
    EVT_UPDATE_VIEWS( a2dDocviewCanvas::OnUpdate )
    //EVT_DISCONNECT_ALLVIEWS( a2dCanvasView::OnDisConnectView )
END_EVENT_TABLE()

template<class baseclass>
a2dDocviewCanvas< baseclass >::a2dDocviewCanvas( a2dCanvasView* view, wxWindow* parent, wxWindowID id,
           const wxPoint& pos, const wxSize& size, long style, a2dDrawer2D* drawer2D )
:
  baseclass( parent, id, pos, size, style, drawer2D )
{
    SetView( view );
}

template<class baseclass>
a2dDocviewCanvas< baseclass >::a2dDocviewCanvas( a2dCanvasView* view, a2dDrawingPart* drawingpart, wxWindow* parent, wxWindowID id,
           const wxPoint& pos, const wxSize& size, long style )
:
    baseclass( drawingpart, parent, id, pos, size, style )
{
    SetView( view );
}

template<class baseclass>
void a2dDocviewCanvas< baseclass >::SetView( a2dCanvasView* view ) 
{ 
    m_view = view; 
    if ( m_view )
    {
        a2dCanvasDocument* doc = wxStaticCast( m_view->GetDocument(), a2dCanvasDocument );
        m_view->SetDisplayWindow( this );
        if ( baseclass::GetDrawingPart() && doc )
            baseclass::GetDrawingPart()->SetShowObject( doc->GetDrawing()->GetRootObject() );
    }
}

template<class baseclass>
void a2dDocviewCanvas< baseclass >::OnCloseView( a2dCloseViewEvent& event )
{
    a2dCanvasView* theView = wxDynamicCast( event.GetEventObject(), a2dCanvasView );
    if ( baseclass::GetDrawingPart() && theView->GetDisplayWindow() == this )
    {
		if ( baseclass::GetDrawingPart()->GetCanvasToolContr() )
			baseclass::GetDrawingPart()->GetCanvasToolContr()->StopAllTools();
		baseclass::GetDrawingPart()->SetShowObject( NULL );
        if ( a2dCanvasGlobals->GetActiveDrawingPart() == baseclass::GetDrawingPart() )
            a2dCanvasGlobals->SetActiveDrawingPart( NULL );
    }
    event.Skip();
}

template<class baseclass>
void a2dDocviewCanvas< baseclass >::OnActivate(  a2dViewEvent& viewevent )
{
    a2dCanvasView* view = (a2dCanvasView*) viewevent.GetEventObject();
    bool activate = viewevent.GetActive();

    if ( activate &&  wxStaticCastNull( m_view->GetDocument(), a2dCanvasDocument ) && view == m_view )
    {
        if ( !baseclass::HasFocus() )
        {
            // WARNING doing a setfocus here ruins it in wxAuiNotebook (Tab gets the END focus instead of this window here).
            // wxLogDebug( "a2dCanvasView::OnActivate set focus %p", GetDrawingPart() );
            // GetDisplayWindow()->SetFocus();
        }
    }

    activate = viewevent.GetActive();

    if ( activate && baseclass::GetDrawingPart() && baseclass::GetDrawingPart()->GetDrawing() && view == m_view )
    {
	    a2dCanvasGlobals->SetActiveDrawingPart( baseclass::GetDrawingPart() );
    }

    viewevent.Skip();
}

template<class baseclass>
void a2dDocviewCanvas< baseclass >::OnUpdate( a2dDocumentEvent& event )
{
  	if ( baseclass::GetDrawingPart() )
	{
        baseclass::GetDrawingPart()->Update();
	}
}

template<class baseclass>
void a2dDocviewCanvas< baseclass >::OnSetDocument( a2dViewEvent& event )
{
    a2dCanvasDocument* canvasdoc = wxDynamicCast( ( ( a2dView* )event.GetEventObject() )->GetDocument(), a2dCanvasDocument );
    if ( canvasdoc && m_view->GetDocument() )
    {
		a2dCanvasView* view = wxDynamicCast( ( ( a2dView* )event.GetEventObject() ), a2dCanvasView );

		if ( baseclass::GetDrawingPart() && view->GetDisplayWindow() == this )
		{
			//why? view->GetDocument()->ConnectEvent( a2dEVT_COM_EVENT, this );
			baseclass::GetDrawingPart()->SetShowObject( canvasdoc->GetDrawing()->GetRootObject() );
		}
	}
}


#endif /* __WXCANDOC_H__ */
