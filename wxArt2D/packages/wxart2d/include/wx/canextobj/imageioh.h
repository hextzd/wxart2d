/*! \file wx/canextobj/imageioh.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: imageioh.h,v 1.17 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __A2D_WXIMAGEIOH_H__
#define __A2D_WXIMAGEIOH_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/editor/candoc.h"
#include "wx/canvas/sttool.h"


//! Input and output handler for a2dCanvasDocument to/from a wxImage
/*!
    \ingroup fileio
*/
class A2DEDITORDLLEXP a2dCanvasDocumentIOHandlerImageIn : public a2dIOHandlerStrIn
{
    DECLARE_DYNAMIC_CLASS( a2dCanvasDocumentIOHandlerImageIn )

public:
    //! Constructor.
    a2dCanvasDocumentIOHandlerImageIn( wxBitmapType type = wxBITMAP_TYPE_ANY );

    //! Destructor.
    ~a2dCanvasDocumentIOHandlerImageIn();

    //! The image loaded, will be added to this drawingpart
    void SetDrawingPart( a2dDrawingPart* drawingPart ) { m_drawingPart = drawingPart; };

    //! The image loaded, will be added to this drawing part
    /*!
        \return pointer to the drawingpart where the image will be added to.
    */
    a2dDrawingPart* GetDrawingPart() const { return m_drawingPart; }


    //! test header of the file to see if it can be loaded
    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL );

    //! load a complete document which contains an image
    /*!
    */
    virtual bool Load( a2dDocumentInputStream& stream , wxObject* doc );

    //! scale the a2dImage by this factor, taking width and height from image imported.
    void SetScale( double scale ) { m_scale = scale; }

    //! returns scale to be used for image after import.
    double GetScale() const { return m_scale; }

    //! set default resultion to be used for image import if not specified as image option.
    void SetPPI( double ppi ) { m_ppi = ppi; }

    //! returns default resultion to be used for image import if not specified as image option.
    double GetPPI() const { return m_ppi; }

    //! if set true, image created can be edited in such a way that aspect ratio is preserved
    void SetPreserveAspect( bool preserveAspectRatio ) { m_preserveAspectRatio = preserveAspectRatio; }

    //! if true image created can be edited in such a way that aspect ratio is preserved
    bool GetPreserveAspect() { return m_preserveAspectRatio; }

    //! set position to be used for image after import.
    void SetImagePosition( double x, double y ) { m_xImage = x; m_yImage = y; }

    //! set image size (if m_scale = -1 ) image will be sized to this width or height, keeping expect ratio intact.
    void SetImageSize( double w, double h ) { m_wImage = w; m_hImage = h; }

    //! set the image type to load from, wxBITMAP_TYPE_ANY means outdetect from filename in CanSave().
    void SetType( wxBitmapType type ) { m_type = type; }

    //! return the image type to load from, wxBITMAP_TYPE_ANY means outdetect from filename in CanSave().
    wxBitmapType GetType() { return m_type; }

    //! returns the values of x- and y-resolution options specified as the image options if any
    static wxImageResolution GetResolutionFromOptions(const wxImage& image, double *x, double *y);

protected:

    //! for the stream, detect what image type it contains
    wxBitmapType DetectImageType();

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    double m_ppi;
    double m_scale;
    bool m_preserveAspectRatio;

    double m_xImage;
    double m_yImage;

    double m_wImage;
    double m_hImage;

    //! image type
    wxBitmapType m_type;

    a2dDrawingPart* m_drawingPart;

};

//! Input and output handler for a2dCanvasDocument to/from a wxImage
/*!
    This IoHandler is to render parts of a a2dCanvasDocument to an wxImage, and save this as
    a bitmap to a stream. The maximum size of the bitmap in X or Y is given upfront or else asked for.
    The a2dCanvasObject to display is set, or the root object of the a2dCanvasDocument is used.
    The part to display is set as a rectangle defined in world coordinates. That part seen from the
    m_showObject, is rendered to the image. The image is sized, taking into account the maximum size, such
    that the view rectangle fits it. The image size can be set upfront, in which case the drawing will be
    centered on that image size.

    The rendering itself is done via a a2dDrawer2D, which can be set. The default is a2dMemDcDrawer.
    You can use a2dAggDrawer for antialiased drawing etc.

    \ingroup fileio
*/
class A2DEDITORDLLEXP a2dCanvasDocumentIOHandlerImageOut : public a2dIOHandlerStrOut
{
    DECLARE_DYNAMIC_CLASS( a2dCanvasDocumentIOHandlerImageOut )

public:
    //! Constructor.
    a2dCanvasDocumentIOHandlerImageOut( wxBitmapType type = wxBITMAP_TYPE_ANY );

    //! Destructor.
    ~a2dCanvasDocumentIOHandlerImageOut();

    //! this handler can also save as CVG
    bool CanSave( const wxObject* obj = NULL );

    //! save a2dCanvasDocument as image
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* obj );

    //! document will be rendered as seen from this object, if NULL, the root of the document will be used.
    void SetShowObject( a2dCanvasObject* showObject ) { m_showObject = showObject; };

    //!return pointer of then currently shown object on the drawer.
    /*!
        \return pointer to the current object that is shown.
    */
    a2dCanvasObject* GetShowObject() const { return m_showObject; }

    //! render the document into an image.
    wxImage RenderImage( a2dCanvasDocument* document );

    //! Set the size of the image to create.
    void SetImageSize( const wxSize& size ) { m_imageSize = size; }

    //! Give the virtual size to be rendered to an image.
    /*!
        Here you give the minimum and maximum in world coordinates, which must be displayed on the bitmap.

        SetImageSize() is used for the image size itself.

        \param minx minimum x coordinate of display area
        \param miny minimum y coordinate of display area
        \param maxx maximum x coordinate of display area
        \param maxy maximum y coordinate of display area

        \remark these setting are only used when m_automaticScale is set false.
    */
    void SetViewRectangle( double minx, double miny, double maxx, double maxy );

    //! Give the virtual size to be rendered to an image as boundingbox
    /*!
        Here you give the minimum and maximum in world coordinates, which must be displayed on the bitmap.

        \param viewBox boundingbox in world coordinates to display
    */
    void SetViewRectangle( a2dBoundingBox viewBox ) { m_viewBox = viewBox; }

    //!set if the Yaxis goes up or down
    void SetYaxis( bool up ) { m_yaxis = up; }

    //!get y axis orientation
    inline bool GetYaxis() const { return m_yaxis; }

    //! set the image type to save to, wxBITMAP_TYPE_ANY means outdetect from filename in CanSave().
    void SetType( wxBitmapType type ) { m_type = type; }

    //! return the image type to save to, wxBITMAP_TYPE_ANY means outdetect from filename in CanSave().
    long GetType() const { return m_type; }

    //! when image size is calculated, this is the maximum size possible
    void SetMaxWidthHeight( long maxWidthHeight ) { m_maxWidthHeight = maxWidthHeight; }

    //! leafs a border of this amount of pixels around the drawing
    void SetBorder( wxUint16 border, bool percentual = true );

    //! set the internal m_drawer2D to be used for rendering the document
    void SetDrawer2D( a2dDrawer2D* drawer2d );

    //! get the internal m_drawer2D that is used for rendering the document
    a2dDrawer2D* GetDrawer2D() const { return m_drawer2D; }

protected:

    //! the drawing engine to use
    a2dDrawer2D* m_drawer2D;

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    //! image size that will be created.
    wxSize m_imageSize;

    //! calculate image size as set by user when m_imageSize = (0,0) else eqauls m_imageSize
    wxSize m_imageSizeCalc;

    //! render the document using this object as ShowObject.
    a2dCanvasObject* m_showObject;

    //! create a border in this ammount of pixels.
    int m_border;

    //! border is not in pixel but a percentage of the smallest height/width
    bool m_borderpercentual;

    //! image type
    wxBitmapType m_type;

    //! image type as detected in CanSave(), wil be used in Save() if m_type is wxBITMAP_TYPE_ANY
    wxBitmapType m_typeDetectedFromFilename;

    //! axis is up or down
    bool m_yaxis;

    //! viewing area in world coordinates to display on bitmap
    a2dBoundingBox m_viewBox;

    long m_maxWidthHeight;
};

#endif

