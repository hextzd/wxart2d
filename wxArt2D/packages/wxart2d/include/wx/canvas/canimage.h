/*! \file wx/canvas/rectangle.h
    \brief rectangular shapes derived from a2dCanvasObject

    rectangle shapes

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canimage.h,v 1.3 2009/07/17 16:03:34 titato Exp $
*/

#ifndef __WXIMAGEPRIM_H__
#define __WXIMAGEPRIM_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/rectangle.h"

//! a2dImage (will  scale/rotate image when needed)
/*!
        Holds a wxImage.

     \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dImage: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:

    //!constructor
    a2dImage();

    //!Image position is middle point of image
    /*!
        \param image: reference to image
        \param xc: x center
        \param yc: y center
        \param w: Width of object.
        \param h: Heigth of object.
     */
    a2dImage( const wxImage& image, double xc, double yc, double w, double h );

    //!Image position is middle point of image
    /*!
        \param imagefile filename of image
        \param type type of file
        \param xc x center
        \param yc y center
        \param w Width of object.
        \param h Heigth of object.
    */
    a2dImage( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h );

    //! construct the object by rendering the given object into the image of this object
    /*!
        \param torender object to render into image
        \param xc: x center
        \param yc: y center
        \param w: Width of this object.
        \param h: Heigth of this object.
        \param imagew: Width in pixels to initiate image.
        \param imageh: Heigth in pixels to initiate image.

        \remark the (imagew, imageh) is used by the rendering engine and defines the resolution

        \remark for the layers in the object the a2dCanvasDocument of torender is used,
        if not set, a temporary document is created, using the default layer set.
    */
    a2dImage( a2dCanvasObject* torender, double xc, double yc, double w, double h, int imagew = 100, int imageh = 100 );

    a2dImage( const a2dImage& ori, CloneOptions options, a2dRefMap* refs );

    ~a2dImage();

    //! How transparent is this image
    /*!
        \param OpacityFactor opacity is set 0 total transparent 255 non transparent
    */
    void SetOpacityFactor( wxUint8 OpacityFactor );

    //! see SetOpacityFactor()
    wxUint8 GetOpacityFactor() { return m_OpacityFactor; }

    //! return a list of a2dRect, for each pixel which has a colour in the box formed by col1 and col2
    /*!
        Can be used to vectorize an image.
    */
    a2dCanvasObjectList* GetAsRectangles( const wxColour& col1,  const wxColour& col2, bool transform );

    //! render the given object into the image of this object
    /*!
        \param torender object to render into image
        \param imagew: Width in pixels to initiate image.
        \param imageh: Heigth in pixels to initiate image.

        \remark the (imagew, imageh) is used by the rendering engine and defines the resolution

        \remark for the layers in the object the a2dCanvasDocument of torender is used,
        if not set, a temporary document is created, using the default layer set.
    */
    void RenderObject( a2dCanvasObject* torender, int imagew = 100 , int imageh = 100 );

    //!get width of image
    double GetWidth() const { return m_width; }

    //!get height of image
    double GetHeight() const { return m_height; }

    //!set width
    /*! \param width width of image */
    void SetWidth( double width ) { m_width = width; SetPending( true ); }

    //!set height
    /*!! \param height height of image */
    void SetHeight( double height ) { m_height = height; SetPending( true ); }

    //! get internal wxImage object
    wxImage& GetImage() { return m_image; }

    //! set image
    void SetImage( const wxImage& image ) { m_image = image; SetPending( true ); }

    //! get filename
    wxString& GetFilename() { return m_filename; }

    //! set filename and type of image for saving.
    /*!
        \param filename filename of image
        \param type the type of the image
        \param doread when true the image is now read from the file.
    */
    void SetFilename( const wxString filename, wxBitmapType type, bool doread = true );

    //! set type of image for saving
    void SetImageType( wxBitmapType type ) { m_type = type; }

    //! return type of image as was read from the file, or just set.
    wxBitmapType GetImageType() { return m_type; }

    //! Sets if the pattern (a rectangle) will be drawn on top of this image
    /*!
        If set to <code>true</code> the pattern (a rect) will be drawn on top
        of the image, else the pattern will be drawn behind the image.

        \remark By default the rectangle / pattern will be drawn behind the image

        \param  drawPatternOnTop Set <code>true</code> to draw on top of the image, else <code>false</code>
    */
    void SetDrawPatternOnTop( bool drawPatternOnTop );

    //! Returns if the pattern will be drawn on top of the image
    /*!
        \see SetDrawPatternOnTop

        \return <code>true</code> if pattern will be drawn on top of this image, else <code>false</code> (default)
    */
    bool GetDrawPatternOnTop() { return m_drawPatternOnTop; }

    DECLARE_DYNAMIC_CLASS( a2dImage )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! only disables skew and rotation editing
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnHandleEvent( a2dHandleMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    double      m_width;
    double      m_height;

    wxImage     m_image;

    wxString    m_filename;

    wxBitmapType m_type;

    bool        m_drawPatternOnTop;

    wxUint8     m_OpacityFactor;

public:
    static a2dPropertyIdDouble* PROPID_Width;
    static a2dPropertyIdDouble* PROPID_Height;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dImage( const a2dImage& other );
};

//! a2dRgbaImage
/*!
   Rectangle with radius and Min and Max coordinates.
   This rectangle has extra information such that its origin is not always the position of the matrix.
   It can be placed relative to its matrix in any way.
   The need for this type of rectangle, becomes clear when one needs to resize a rectangular shape around
   its children objects.


    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dRgbaImage: public a2dRectMM
{

    DECLARE_EVENT_TABLE()

public:

    a2dRgbaImage();

    //!constructor
    /*!
    \param x: x minimum
    \param y: y minimum
    \param image width and hight taken from here
    \param alpha alhpa of image set to this
    */
    a2dRgbaImage( double x, double y, wxImage& image, wxUint8 OpacityFactor );

    a2dRgbaImage( const a2dRgbaImage& image, CloneOptions options, a2dRefMap* refs );

    ~a2dRgbaImage();

    //! How transparent is this image
    /*!
        \param OpacityFactor opacity is set 0 total transparent 255 non transparent
    */
    void SetOpacityFactor( wxUint8 OpacityFactor );

    //! see SetOpacityFactor()
    wxUint8 GetOpacityFactor() { return m_OpacityFactor; }

    //!set width of rectangle
    /*!
        \param w width of rectangle
    */
    void    SetWidth( double w )  { m_maxx = m_minx + w; SetPending( true ); }

    //!set height of rectangle
    /*!
        \param h height of rectangle
    */
    void    SetHeight( double h ) { m_maxy = m_miny + h; SetPending( true ); }

    a2dImageRGBA& GetImage() { return m_glimage; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dRgbaImage )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    a2dImageRGBA m_glimage;

    bool m_flip;

    bool m_drawPatternOnTop;

    wxUint8 m_OpacityFactor;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRgbaImage( const a2dRgbaImage& other );
};

//! a2dImageMM (will  scale/rotate image when needed)
/*!
        Holds a wxImage.

     \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dImageMM: public a2dRectMM
{
public:

    //!constructor
    a2dImageMM();

    //!Image position is middle point of image
    /*!
        \param image: reference to image
        \param xc: x center
        \param yc: y center
        \param w: Width of object.
        \param h: Heigth of object.
     */
    a2dImageMM( const wxImage& image, double xc, double yc, double w, double h );

    //!Image position is middle point of image
    /*!
        \param imagefile filename of image
        \param type type of file
        \param xc x center
        \param yc y center
        \param w Width of object.
        \param h Heigth of object.
    */
    a2dImageMM( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h );

    //!constructor
    /*!
    \param x: x minimum
    \param y: y minimum
    \param image width and hight taken from here
    \param alpha alhpa of image set to this
    */
    a2dImageMM( double x, double y, wxImage& image, unsigned char alpha );

    //! construct the object by rendering the given object into the image of this object
    /*!
        \param torender object to render into image
        \param xc: x center
        \param yc: y center
        \param w: Width of this object.
        \param h: Heigth of this object.
        \param imagew: Width in pixels to initiate image.
        \param imageh: Heigth in pixels to initiate image.

        \remark the (imagew, imageh) is used by the rendering engine and defines the resolution

        \remark for the layers in the object the a2dCanvasDocument of torender is used,
        if not set, a temporary document is created, using the default layer set.
    */
    a2dImageMM( a2dCanvasObject* torender, double xc, double yc, double w, double h, int imagew = 100, int imageh = 100 );

    a2dImageMM( const a2dImageMM& ori, CloneOptions options, a2dRefMap* refs );

    ~a2dImageMM();

    //! return a list of a2dRect, for each pixel which has a colour in the box formed by col1 and col2
    /*!
        Can be used to vectorize an image.
    */
    a2dCanvasObjectList* GetAsRectangles( const wxColour& col1,  const wxColour& col2, bool transform );

    //! render the given object into the image of this object
    /*!
        \param torender object to render into image
        \param imagew: Width in pixels to initiate image.
        \param imageh: Heigth in pixels to initiate image.

        \remark the (imagew, imageh) is used by the rendering engine and defines the resolution

        \remark for the layers in the object the a2dCanvasDocument of torender is used,
        if not set, a temporary document is created, using the default layer set.
    */
    void RenderObject( a2dCanvasObject* torender, int imagew = 100 , int imageh = 100 );

    //! get internal wxImage object
    wxImage& GetImage() { return m_image; }

    //! set image
    void SetImage( const wxImage& image ) { m_image = image; SetPending( true ); }

    //! get filename
    wxString& GetFilename() { return m_filename; }

    //! set filename and type of image for saving.
    /*!
        \param filename filename of image
        \param type the type of the image
        \param doread when true the image is now read from the file.
    */
    void SetFilename( const wxString filename, wxBitmapType type, bool doread = true );

    //! set type of image for saving
    void SetImageType( wxBitmapType type ) { m_type = type; }

    //! return type of image as was read from the file, or just set.
    wxBitmapType GetImageType() { return m_type; }

    //! Sets if the pattern (a rectangle) will be drawn on top of this image
    /*!
        If set to <code>true</code> the pattern (a rect) will be drawn on top
        of the image, else the pattern will be drawn behind the image.

        \remark By default the rectangle / pattern will be drawn behind the image

        \param  drawPatternOnTop Set <code>true</code> to draw on top of the image, else <code>false</code>
    */
    void SetDrawPatternOnTop( bool drawPatternOnTop );

    //! Returns if the pattern will be drawn on top of the image
    /*!
        \see SetDrawPatternOnTop

        \return <code>true</code> if pattern will be drawn on top of this image, else <code>false</code> (default)
    */
    bool GetDrawPatternOnTop() { return m_drawPatternOnTop; }

    DECLARE_DYNAMIC_CLASS( a2dImageMM )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    //! only disables skew and rotation editing
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    wxImage     m_image;

    wxString    m_filename;

    wxBitmapType m_type;

    bool        m_drawPatternOnTop;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dImageMM( const a2dImageMM& other );
};

#endif /* __WXIMAGEPRIM_H__ */
