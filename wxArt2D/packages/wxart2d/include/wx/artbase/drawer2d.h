/*! \file wx/artbase/drawer2d.h
    \brief Contains graphical drawing context specific classes.
    a2dDrawer2D and derived classes are used for drawing primitives.

    A drawing context which is wxDrawer2 derived, can be used to draw in general.
    Still it has special features in order to use it for drawing document containing a drawing.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawer2d.h,v 1.53 2009/10/06 18:40:31 titato Exp $
*/

#ifndef __WXDRAWER2D_H__
#define __WXDRAWER2D_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/prntbase.h"
#include <wx/module.h>

#include "wx/general/genmod.h"
#include "wx/artbase/artglob.h"
#include "wx/artbase/stylebase.h"
#include "wx/artbase/liner.h"
#include "wx/artbase/afmatrix.h"
#include "wx/artbase/bbox.h"
#include "wx/artbase/polyver.h"
#include "wx/genart/imagergba.h"

#include <vector>


//! Hold openGl vertex array id's
/*! 
*/
class A2DARTBASEDLLEXP a2dListId
{

public:

    a2dListId( long fill = 0, long stroke = 0, long start = 0, long offset = 0 )
    {
         m_fill = fill;
         m_stroke = stroke;
         m_start = start;
         m_offset = offset;
    }

    ~a2dListId()
    {
    }

    //!id of vertex list
    long m_fill;
    long m_stroke;
    long m_start;
    long m_offset;
};

typedef std::vector<a2dListId> a2dShapeIdVector;

#ifdef Round
#undef Round
#endif
#define Round( x ) (int) floor( (x) + 0.5 )

#if wxCHECK_VERSION(2,7,1)
#  define WX_COLOUR_HAS_ALPHA 1
#else
#  define WX_COLOUR_HAS_ALPHA 0
#endif

static const char* HORIZONTAL_HATCH_XPM[] =
{
    "32 32 2 1 0 0",
    "  c #000000",
    "! c #FFFFFF",
    "                                ",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "                                ",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "                                ",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "                                ",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
};

static const char* VERTICAL_HATCH_XPM[] =
{
    "32 32 2 1 0 0",
    "  c #000000",
    "! c #FFFFFF",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!"
};

static const char* FDIAGONAL_HATCH_XPM[] =
{
    "32 32 2 1 0 0",
    "  c #000000",
    "! c #FFFFFF",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! "
};

static const char* BDIAGONAL_HATCH_XPM[] =
{
    "32 32 2 1 0 0",
    "  c #000000",
    "! c #FFFFFF",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "!!!!!!! !!!!!!! !!!!!!! !!!!!!! ",
    "!!!!!! !!!!!!! !!!!!!! !!!!!!! !",
    "!!!!! !!!!!!! !!!!!!! !!!!!!! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! !!!!!!! !!!!!!! !!!!!!! !!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "! !!!!!!! !!!!!!! !!!!!!! !!!!!!",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!"
};

static const char* CROSSDIAG_HATCH_XPM[] =
{
    "32 32 2 1 0 0",
    "  c #000000",
    "! c #FFFFFF",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    " !!!!!!! !!!!!!! !!!!!!! !!!!!!!",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! ",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!!!! !!!!!!! !!!!!!! !!!!!!! !!!",
    "!!! ! !!!!! ! !!!!! ! !!!!! ! !!",
    "!! !!! !!! !!! !!! !!! !!! !!! !",
    "! !!!!! ! !!!!! ! !!!!! ! !!!!! "
};

static const char* CROSS_HATCH_XPM[] =
{
    "32 32 2 1 0 0",
    "  c #000000",
    "! c #FFFFFF",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "                                ",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "                                ",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "                                ",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "                                ",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!",
    "!! !!!!!!! !!!!!!! !!!!!!! !!!!!"
};


//! Used for defining how a ClippingRegion defined as a polygon is combined with
/*! the existing clipping region in a a2dDrawer2D.
    \ingroup drawer
*/
enum a2dBooleanClip
{
    a2dCLIP_AND,  /*!< AND operation, the new region is the union/intersection with existing clipping region */
    a2dCLIP_COPY, /*!< COPY operation, replaces existing clipping region */
    a2dCLIP_DIFF, /*!< DIFF operation subtracts new region from the existing clipping region */
    a2dCLIP_OR,   /*!< OR operation, the new region is the merge of the given with the existing clipping region */
    a2dCLIP_XOR   /*!< XOR operation, the new region is the complement of the intersection with existing clipping region */
};


#include "wx/artbase/graphica.h"

//! Drawing context abstraction.
/*!
    a2dDrawer2D presents a unified abstract view of underlying system-dependent
    drawing contexts.  It provides a simple low-level interface for
    drawing rectangles and other basic primitives, and it also handles the
    mapping and transformations from world coordinates to device coordinates.

    The current a2dFill and a2dStroke need to be set, those classes are always reference counted,
    and a2dDrawer2D will increment and decrement them when setting and releasing them.
    This means that if the reference count reaches zero it will actually be
    deleted.

    Mapping from device to Logical/World coordinates needs to be set for a drawer.
    The Yaxis orientation can also be set via the SetYaxis() method.

    \par About Transformations

    There are three different coordinate systems used by a2dDrawer2D, and
    two transformations that map between them.  These are referred to in a2d as
      - User coordinates
      - World coordinates
      - Device coordinates

    User coordinates are the coordinates you give to drawing functions like
    DrawCircle().  User coordinates are transformed to World coordinates
    by the transformation returned from GetTransform().
    The idea behind user coordinates is that one can draw in a relative coordinate system,
    which can be at a position, angle and scale relative to the world coordinates.
    The drawing context will correctly transform the relative coordinates, to world and device.
    This way of drawing especialy becomes important when working with canvas objects.
    Inside such object one can use drawing functions relative to the origin of that object,
    and one does not have to take into account how and where this object is placed in the world
    coordinate system.
    World coordinates are transformed to Device coordinates using the
    transformation returned by GetMappingMatrix().  One can draw
    in device coordinates without transformations by calling PushIdentityTransform().
    PopTransform() will restore the previous transformation.
    As an example one can draw the vertexes in a plot as a circle in device
    coordinates (with a fixed radius in pixels that is), while the curve itself is draw in normal
    user coordinates.

    Given an input point (x,y), the final location on the output device
    (e.g. screen) in device units (e.g. pixels) for any of the Draw*() methods
    is given by
    \code
        (devx,devy) = GetMappingMatrix() * GetTransform() * (x,y)
    \endcode
    The compound of the world-to-device mapping and user-to-world transform
    is also kept up-to-date. It can be obtained by calling
    GetUserToDeviceTransform().
    So we can also say
    \code
        (devx,devy) = GetUserToDeviceTransform() * (x,y)
    \endcode

    We can also depict this relationship as
    \verbatim
                User coordinates
               /                \
      [ GetTransform() ]         |
              |                  |
        World coordinates     [ GetUserToDeviceTransform() ]
              |                  |
    [ GetMappingMatrix() ]       |
               \                /
              Device coordiantes
    \endverbatim

    \par The Mapping Matrix

    The mapping matrix returned by GetMappingMatrix() contains only translation
    and axis-aligned scaling.  So an input of an axis-aligned rectangle is
    always transformed to another axis-aligned drawline
    .  This lets you have
    an unlimited virtual canvas (aka "world") and map different portions of
    it to the drawing surface.  Additionally, some objects like fonts are
    aware of the orientation of the Y axis defined by the mapping, and will
    draw right-side up, even if the Y axis is mirrored in the mapping.

    The mapping matrix can be manipulated using the following methods:
      - SetMappingUpp()
      - SetMappingDeviceRect()
      - SetMinX(), SetMinY()
      - SetUppX(), SetUppY()
      - SetMappingWidthHeight()
      - SetYaxis()

    \par The User to World Transform Matrix

    The user-to-world transform matrix returned by GetTransform() is a 2x3
    affine matrix that can contain translation, rotation, and scaling
    (including shearing and mirroring).

    The relative transform can be manipulated with these methods:
      - SetTransform()
      - PushTransform()
      - PopTransform()
      - Transform()


    \par double buffering

    A derived drawing context can be buffered or not. But this class is prepared as a
    buffered drawing context, so it assumes in certain member fucntions that one is drawing
    to a buffer. A drawing context which does not use a buffer needs to implement those
    pur virtual members and give assert.

    \ingroup drawer docview style

    \sa a2dDcDrawer, a2dMemDcDrawer, a2dAggDrawer

*/
class A2DARTBASEDLLEXP a2dDrawer2D: public wxObject
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dInitCurrentSmartPointerOwner m_initCurrentSmartPointerOwner;
#endif

public:

    //!constructor
    /*!
        \remark
        Do not forget to call SetDocument() if used standalone
        (in a a2dDocumentCommandProcessor setting this is taken care of).
    */
    a2dDrawer2D( int width = 0, int height = 0 );

    //!constructor
    /*!
        \remark
        Do not forget to call SetDocument() if used standalone
        (in a a2dDocumentCommandProcessor setting this is taken care of)
    */
    a2dDrawer2D( const wxSize& size );

    //!copy constructor
    a2dDrawer2D( const a2dDrawer2D& other );

    //!destructor
    virtual ~a2dDrawer2D();

    //! the display
    void SetDisplay( wxWindow* window ) { m_display = window; }

    //!set at what size to stop drawing
    inline void SetPrimitiveThreshold( wxUint16 pixels, bool asrect = true ) { m_drawingthreshold = pixels; m_asrectangle = asrect; }

    //!get drawing threshold \sa SetDrawingThreshold
    inline wxUint16 GetPrimitiveThreshold() const { return m_drawingthreshold; }

    //! underneath the threshold draw a rectangle instead of the real object.
    inline bool GetThresholdDrawRectangle() const { return m_asrectangle; }

    //!set threshold at which polygon is drawn filled or only outline
    inline void SetPolygonFillThreshold( wxUint16 pixels ) { m_polygonFillThreshold = pixels; }

    //!get threshold at which polygon is drawn filled or only outline
    inline wxUint16 GetPolygonFillThreshold() const { return m_polygonFillThreshold; }

    //!get the DC that is used for rendering
    virtual wxDC* GetRenderDC() const { return NULL; }

    //!get the DC that is used for rendering
    virtual wxDC* GetDeviceDC() const { return NULL; }

    //! Set the display aberration of curved shapes
    /*!
        The drawing of curved shapes will not deviate more than this from the ideal curve.
        \remark the smaller the number the longer the drawing takes.

        \param aber maximum deviation in device coordinates
    */
    void SetDisplayAberration( double aber ) { m_displayaberration = aber; }

    //! Returns the display aberration of curved shapes
    /*!,
        The drawing of curved shapes will not deviate more than this from the ideal curve.
    */
    double GetDisplayAberration() const { return m_displayaberration; }

    //! Return the buffer as a bitmap
    virtual wxBitmap GetBuffer() const = 0;

    //!Change the buffer size
    /*!
        \remark
        SetMappingDeviceRect() is NOT reset.
    */
    virtual void SetBufferSize( int w, int h ) = 0;

    virtual void CopyIntoBuffer( const wxBitmap& bitm ) {};

    //! blit whole buffer to device
    void BlitBuffer();

    //! blit given rect contents within buffer to device
    void BlitBuffer( int x, int y, int width, int height, int xbuf = 0, int ybuf = 0  );

    //! blit given rect contents within buffer to device
    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) ) = 0;

    //! used for blitting to a wxDC.
    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) ) = 0;

    //! quick scroll over small distance
    virtual void ShiftBuffer( int WXUNUSED( dxy ), bool WXUNUSED( yshift ) ) {};

    //!A way to get parts form the buffer that is drawn into.
    /*!
        Used for freetype text at the moment since it mixes with the colors already there.
    */
    virtual wxBitmap GetSubBitmap( wxRect sub_rect ) const = 0;

    //!get y axis orientation
    inline bool GetYaxis() const { return m_yaxis; }

    //!set if the Yaxis goes up or down
    virtual void SetYaxis( bool up );

    //!get y axis orientation of the device coordinate system
    inline bool GetYaxisDevice() const { return m_yaxisDevice; }

    //!set y axis orientation of the device coordinate system
    virtual void SetYaxisDevice( bool up );

    //! to change the default mapping (to the complete buffer).
    /*!
        By default the mapping from world coordinates to device coordinates is
        based on the size of the double buffer.
        For application where the a2dDrawer2D is used as a double buffered device context within
        a a2dCanvas window, it is better to oversize the buffer of the a2dDrawer2D.
        This prevents continuously reallocating/redrawing of the buffer after even the smallest resize.
        To still keep the mapping to what is visible on the window client window, one can
        independently from the buffer size set the mapping rectangle.
        Even a rectangle bigger then the buffer rectangle is possible, but of course the a2dDrawer2D
        will never draw outside the buffer eventually.

        \param remap if true change mapping to
    */
    void SetMappingDeviceRect( int mapx, int mapy, int mapWidth, int mapHeight, bool remap = false );

    //!Give the virtual size to be displayed, the mapping matrix will be calculated.
    /*!
        The current buffer size is used to calculate how to at least display all of the area given.

        \see SetMappingDeviceRect to map to a different device/buffer rectangle.

        \remark do not use during start up since window/buffer size is not well defined in that case resulting in

        \remark bad settings for the mapping.

        \param vx1: minimum x coordinate of display area
        \param vy1: minimum y coordinate of display area
        \param width: width of displayed area in world coordinates
        \param height: height of displayed area in world coordinates
    */
    virtual void SetMappingWidthHeight( double vx1, double vy1, double width, double height );

    //!Give the virtual size to be displayed, the mapping matrix will be calculated.
    /*!
        The current buffer size is used to calculate how to at least display all of the area given.

        \remark do not use during start up since window/buffer size is not well defined in that case resulting in

        \remark bad settings for the mapping.

        \see SetMappingDeviceRect to map to a different device/buffer rectangle.

        \param box a boundingbox to set mapping
    */
    void SetMappingWidthHeight( const a2dBoundingBox& box );

    //! Give the virtual size to be displayed, the mapping matrix will be calculated.
    /*!
        To display all of a drawing, set this here to the boundingbox of the root object
        of the canvas.

        So vx1 and vx2 to the minimum x and y of the boundingbox.
        Calculate xpp and ypp in such a manner that it will show the whole drawing.

        The buffer size or SetMappingDeviceRect() is used when setting the mapping with this function.

        \remark when a user-to-world matrix SetTransform() is set, the m_usertodevice is recalculated to take
        the new mapping matrix and the already set user-to-world matrix into account.

        \param vx1: minimum x coordinate of display area
        \param vy1: minimum y coordinate of display area
        \param xpp: Number of user units per pixel in x
        \param ypp: Number of user units per pixel in y
    */
    void SetMappingUpp( double vx1, double vy1, double xpp, double ypp );

    //!return xpp Number of user units per pixel in x
    double GetUppX() const { return m_xpp; }

    //!return ypp Number of user units per pixel in y
    double GetUppY() const { return m_ypp; }

    //!Set Minimal X of the visible part in world coordinates
    void SetMinX( double x );

    //!Set Minimal Y of the visible part in world coordinates
    void SetMinY( double y );

    //!set world units per pixel in X
    void SetUppX( double x );

    //!set world units per pixel in Y
    void SetUppY( double y );

    //!if the virtual area has been set already return true else false.
    /*!
        Use this during initialization of an application.
    */
    bool GetVirtualAreaSet() const { return m_virtualarea_set;}

    //! when called a mapping change will result not result in a refresh of the m_display.
    /*!
        m_refreshDisplay is incremented, and EndRefreshDisplayDisable() does decrement it.
        When m_refreshDisplay > 0 no refresh is done when changing mapping.

        Used to prevent looping and unneeded refresh when changing mapping several times within a function.
    */
    void StartRefreshDisplayDisable() { m_refreshDisplay++; }

    //! see StartRefreshDisplayDisable()
    void EndRefreshDisplayDisable();

    //! see StartRefreshDisplayDisable()
    bool GetRefreshDisplayDisable() const { return m_refreshDisplay > 0; }

    //! get buffer/device width
    inline int GetWidth() const { return m_width; }

    //! get buffer/device height
    inline int GetHeight() const { return m_height; }

    //! get mapping to device width
    inline int GetMapWidth() const { return m_mapWidth; }

    //! get mapping to device height
    inline int GetMapHeight() const { return m_mapHeight; }

    //! X mapping position in device coordinates
    inline int GetMapX() const { return m_mapX; }

    //! Y mapping position in device coordinates
    inline int GetMapY() const { return m_mapY; }

    //!get Minimal X of the visible part in world coordinates
    double GetVisibleMinX() const;

    //!get Minimal X of the visible part in world coordinates
    double GetVisibleMinY() const;

    //!get Maximum X of the visible part in world coordinates
    virtual double GetVisibleMaxX() const;

    //!get Maximum Y of the visible part in world coordinates
    virtual double GetVisibleMaxY() const;

    //!get Width of visible part in world coordinates
    virtual double GetVisibleWidth() const;

    //!get Height of visible part in world coordinates
    virtual double GetVisibleHeight() const;

    //!get visible area as a boundingbox in world coordinates
    a2dBoundingBox GetVisibleBbox() const;

    //!convert the bounding box in world coordinates to device coordinates and return that rectangle.
    wxRect ToDevice( const a2dBoundingBox& bbox );

    //!convert the rect in  device coordinates to a bounding box in world coordinates and return that boundingbox.
    a2dBoundingBox ToWorld( const wxRect& rect );

    //! convert x from device to world coordinates
    inline double DeviceToWorldX( double x ) const { return ( x - m_worldtodevice.GetValue( 2, 0 ) ) / m_worldtodevice.GetValue( 0, 0 ); }
    //! convert y from device to world coordinates
    inline double DeviceToWorldY( double y ) const { return ( y - m_worldtodevice.GetValue( 2, 1 ) ) / m_worldtodevice.GetValue( 1, 1 ); }
    //! convert x relative from device to world coordinates
    /*!
        Use this to convert a length of a line for instance
    */
    inline double DeviceToWorldXRel( double x ) const { return x / m_worldtodevice.GetValue( 0, 0 ); }
    //! convert y relative from device to world coordinates
    /*!
        Use this to convert a length of a line for instance
    */
    inline double DeviceToWorldYRel( double y ) const { return y / m_worldtodevice.GetValue( 1, 1 ); }


    //! convert x from world to device coordinates
    inline int WorldToDeviceX( double x ) const { return ( int ) floor( m_worldtodevice.GetValue( 0, 0 ) * x + m_worldtodevice.GetValue( 2, 0 ) + 0.5 ); }
    //! convert y from world to device coordinates
    inline int WorldToDeviceY( double y ) const { return ( int ) floor( m_worldtodevice.GetValue( 1, 1 ) * y + m_worldtodevice.GetValue( 2, 1 ) + 0.5 ); }
    //! convert x relative from world to device coordinates
    /*!
        Use this to convert a length of a line for instance
    */
    inline int WorldToDeviceXRel( double x ) const {return ( int ) floor( m_worldtodevice.GetValue( 0, 0 ) * x + 0.5 ); }
    //! convert y relative from world to device coordinates
    /*!
        Use this to convert a length of a line for instance
    */
    inline int WorldToDeviceYRel( double y ) const {return ( int ) floor( m_worldtodevice.GetValue( 1, 1 ) * y + 0.5 ); }
    //! convert x relative from world to device coordinates (result not rounded to integer)
    /*!
        Use this to convert a length of a line for instance
    */
    inline double WorldToDeviceXRelNoRnd( double x ) const {return m_worldtodevice.GetValue( 0, 0 ) * x; }
    //! convert y relative from world to device coordinates (result not rounded to integer)
    /*!
        Use this to convert a length of a line for instance
    */
    inline double WorldToDeviceYRelNoRnd( double y ) const {return m_worldtodevice.GetValue( 1, 1 ) * y; }

    //!get the world-to-device (aka mapping) matrix
    const a2dAffineMatrix& GetMappingMatrix() { return m_worldtodevice; }

    //! set world to device matrix ( better use SetMappingWidthHeight() etc. )
    //! Side ffect m_usertodevice is modified to m_worldtodevice * m_usertoworld
    void SetMappingMatrix( const a2dAffineMatrix& mapping );

    //!set user-to-world transform matrix.
    /*!
        Sets the current user-to-world transform matrix at the top of the
        transform stack.
        The user-to-world matrix transforms all drawing primitives from user
        coordinates to world coordinates.

        \param userToWorld The matrix for transforming from user to world.
    */
    virtual void SetTransform( const a2dAffineMatrix& userToWorld );

    //!get the user-to-world transform matrix.
    /*!
        The user-to-world matrix transforms all drawing primitives from user
        coordinates to world coordinates.
    */
    inline const a2dAffineMatrix& GetTransform() const { return m_usertoworld; }

    //! get matrix which transforms directly from user coordinates to device
    inline const a2dAffineMatrix& GetUserToDeviceTransform() const { return m_usertodevice; }

    //! Save the current user-to-world transform on the affine stack.
    virtual void PushTransform();

    //! push no transform, to draw directly in device coordinates
    virtual void PushIdentityTransform();

    //! Save the current transform on the affine stack and then multiply it by the given affine.
    /*! This is a convenience function that is equivalent to calling
        \code
        PushTransform(); Transform(affine);
        \endcode
     */
    virtual void PushTransform( const a2dAffineMatrix& affine );

    //! Recall the previously saved user-to-world transform off the matrix stack.
    virtual void PopTransform( void );

    //!Set the detail level for spline drawing
    /*!
        \param aber Accuracy in world coordinates
    */
    void SetSplineAberration( double aber );

    //! set a pre-defined style reseting cashed values.
    /*! setting a style ( stroke fill ) for a a2dDrawer2D,
        takes into account the style that is currently set for it.
        If the style being set is equal to the current style,
        internal nothing is done for the underlying device.
        Of course this only works if everyone obeys the rules.
        Routines not using a2dDrawer2D as an entry to device, may fool
        this trick. Therefore to be sure to start with a proper a2dDrawer2D,
        first call this function.
        It will make the a2dDrawer2D style the actual style used on the device.
    */
    virtual void ResetStyle();

    //!Used to set the current stroke.
    /*!The a2dStroke objects are reference counted, so passing a stroke pointer
     * allocated with 'new' to this function will not result in a leak.
     */
    void SetDrawerStroke( const a2dStroke& stroke );

    //!get the current stroke
    a2dStroke GetDrawerStroke() const { return m_currentstroke; }

    //!Used to set the current fill
    /*!The a2dFill objects are reference counted, so passing a fill pointer
     * allocated with 'new' to this function will not result in a leak.
     */
    void SetDrawerFill( const a2dFill& fill );

    //!get the current fill
    a2dFill GetDrawerFill() const { return m_currentfill; }

    //!set font to use for drawing text
    void SetFont( const a2dFont& font );

    //!get font used for drawing text
    a2dFont GetFont() const { return m_currentfont; }

    //! Use a fixed device height for drawing text.
    /*!
        Setting this, will use a fixed device size for the font in case of freetype fonts are where appropriate.
        This prevents resizing fonts face for every text objects. Only when font device height is smaller than m_smallTextThreshold,
        it will start setting the device height again, to enable hinting.
        In case of DC font, a normalized font can be used when editing text, which renders text as separate characters in order to draw carret in between.
    */
    void SetNormalizedFont( bool forceNormalizedFont ) { m_forceNormalizedFont = forceNormalizedFont; }

    //! Get setting for a fixed device height for drawing text.
    bool GetNormalizedFont() { return m_forceNormalizedFont; }

    //!set device text size at which textdrawing will switch to device specific text drawing.
    //! Small text will only be drawn  correctly, when fitting text to pixels.
    //! For bigger text it is no problem, so at a certain size one switches to device specific drawing
    //! of text.
    //! only works if m_forceNormalizedFont = false;
    void SetSmallTextThreshold( wxUint16 pixels ) { m_smallTextThreshold = pixels; }

    //! See SetSmallTextThreshold
    wxUint16 GetSmallTextThreshold() const { return m_smallTextThreshold; }

    //!set the current layer
    /*!
        The layer can be used to set a depth value in some implementation like when using OpenGl to draw.
    */
    void SetLayer( wxUint16 layer ) { m_layer = layer; }

    //!get the current layer
    wxUint16 GetLayer() const { return m_layer; }

    //!Draw vector path in world coordinates
    virtual void DrawVpath( const a2dVpath* path );

    //! draw a list of polygons ( contour clockwise becomes hole )
    virtual void DrawPolyPolygon( a2dListOfa2dVertexList polylist, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //!Draw polygon in world coordinates using pointarray
    virtual void DrawPolygon( a2dVertexArray* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //!Draw polygon in world coordinates using pointlist
    virtual void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //!Draw polyline in world coordinates using pointarray
    virtual void DrawLines( a2dVertexArray* points, bool spline = false );

    //!Draw polyline in world coordinates using pointlist
    virtual void DrawLines( const a2dVertexList* list, bool spline = false );

    //!Draw line in world coordinates
    virtual void DrawLine( double x1, double y1, double x2, double y2 );

    //!Draw Arc in world coordinates
    virtual void DrawArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord );

    //!Draw Elliptic Arc in world coordinates
    virtual void DrawEllipticArc( double xc, double yc, double width, double height , double sa, double ea, bool chord );

    //!Draw RoundedRectangle in world coordinates
    virtual void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    //!Draw CenterRoundedRectangle in world coordinates
    /*! Draws a rounded rectangle centered at the point (xc,yc) in world
        coordinates.
     */
    virtual void DrawCenterRoundedRectangle( double xc, double yc, double width, double height, double radius, bool pixelsize = false );

    //!Draw Circle in world coordinates
    /*! \remark circle gets rotates and scaled etc. in x and y when required.*/
    virtual void DrawCircle( double x, double y, double radius );

    //!Draw Ellipse in world coordinates
    /*! \remark ellipse gets rotates and scaled etc. in x and y when required.*/
    virtual void DrawEllipse( double x, double y, double width, double height );

    //! Draw wxImage in world coordinates
    /*! Draw the wxImage to the screen in such a manner that the
     image is mapped into the rectangle defined by
     points (x-width/2,y-width/2) and (x+width/2,y+height/2) in
     world coordinates, with the center of the image at (x,y).

     \param image image to draw
     \param x world x position of center of image
     \param y world y position of center of image
     \param width width in world coordinates to map width of image to
     \param height height in world coordinates to map height of image to
     */
    virtual void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 ) = 0;

    virtual void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 ) = 0;

    //! Draw wxImage in world coordinates
    /*! Equivalent to calling
        DrawImage(image, x,y, image.GetWidth(),image.GetHeight())

        \param image image to draw
        \param x world x position of center of image
        \param y world y position of center of image
    */
    void DrawImage( const wxImage& image, double x = 0, double y = 0, wxUint8 Opacity = 255 )
    { DrawImage( image, x, y, image.GetWidth(), image.GetHeight(), Opacity ); }

    //! draw a single point
    virtual void DrawPoint( double xc, double yc ) = 0;

    //! If true use real scale else different scale by x and y
    void SetRealScale( bool realScale ) { m_realScale = realScale;}

    //! Draw text in user coordinates.
    /*! Draws text at the given position in user coordinates.

        Adding rotation etc. to the user transform matrix makes it possible
        to draw rotated text.
        \param text The text which should be drawn.
        \param x,y The position to draw at.
        \param alignment Use this to align the text (eg. wxMINX | wxMINY). See also a2dFontAlignment.
        \param Background with or without background rectangle
        \sa SetFont()
    */
    virtual void DrawText( const wxString& text, double x, double y, int alignment = wxMINX | wxMINY, bool Background = true );

    //! text drawn in device coordinates
    virtual void DeviceDrawAnnotation( const wxString& WXUNUSED( text ), wxCoord WXUNUSED( x ), wxCoord WXUNUSED( y ), const wxFont& WXUNUSED( font ) ) {}

    //! set drawstyle to use for drawing,
    /*!
        The draw style is used for drawing in a certain fashion.

        The current fill, stroke are not changed while setting a diffrent drawstyle.
        Therefore first set the required fill and stroke.
        Setting a2dFIX_STYLE or a2dFIX_STYLE_INVERT can only be reset using ResetFixedStyle().
        Still it can be temprarely overruled with OverRuleFixedStyle() to be restored with ReStoreFixedStyle().

        New settings for fill, stroke may or may not have effect in how is drawn, but do change the current fill are stroke,
        independent of the drawing style.
        This depends if it makes sense to have a different style when the drawstyle is in action.
        When drawstyle is set, the last set fill, stroke are made active when needed.

        \param  drawstyle one of the draw styles
    */
    void SetDrawStyle( a2dDrawStyle drawstyle );

    //!get drawstyle used for drawing.
    a2dDrawStyle GetDrawStyle() const {return m_drawstyle;}

    //! to modify drawing feature when used as context for printing
    void SetPrintMode( bool onOff ) { m_printingMode = onOff; }

    //! id style is FIXED, saves current style and sets style to a2dFILLED
    void OverRuleFixedStyle();

    //! only way to reset style after SetDrawStyle( a2dFIXED*** );
    void ResetFixedStyle();

    //! when fixed drawing style is set, it can be overruled.
    /*!
        The last fixed style stroke and fill are restored.
    */
    void ReStoreFixedStyle();


    //! when set, all drawing functions return immediately.
    /*!
     This can be used to render only nested parts of documents. The parent part are drawn from their
     a2dCanvasObjects, but in reality nothing will be drawn until drawing is enabled for the deeper nested objects.
    */
    void SetDisableDrawing( bool disableDrawing ) { m_disableDrawing = disableDrawing; }

    //! see SetDisableDrawing()
    bool GetDisableDrawing() { return m_disableDrawing; }

    /*
    // clipping region
    */

    //!set clipping region from polygon to which everything drawn will be clipped.
    virtual void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE ) = 0;

    //!extend clipping region with a polygon to which everything drawn will be clipped.
    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND ) = 0;

    //! push on stack the current clipping region and extend clipping region
    /*!
         The clipping region will be extended with a polygon to which everything drawn will be clipped.
    */
    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND ) = 0;

    //!pop a previously pushed clipping region
    virtual void PopClippingRegion() = 0;

    //!set clipping region using x y values in device coordinates
    virtual void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord maxx, wxCoord maxy ) = 0;

    //!set clipping region using x y values in world coordinates
    virtual void SetClippingRegion( double minx, double miny, double maxx, double maxy ) = 0;

    //!set clipping region off
    virtual void DestroyClippingRegion() = 0;

    //!what is the current clipping region in world coordinates
    void GetClippingBox( double& x, double& y, double& w, double& h ) const;

    //!what is the current clipping region in world coordinates
    a2dBoundingBox& GetClippingBox() { return m_clipboxworld; }

    //!what is the current clipping region in world coordinates
    void GetClippingMinMax( double& xmin, double& ymin, double& xmax, double& ymax ) const;

    //!what is the current clipping region in device coordinates
    wxRect& GetClippingBoxDev() { return m_clipboxdev; }

    //!what is the current clipping region in device coordinates
    void GetClippingBoxDev( int& x, int& y, int& w, int& h ) const;

    //! Initialize a drawer
    void Init();

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw() = 0;

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw() = 0;

    //! when enabling m_useOpacityFactor, this is how transparent
    /*!
        \param OpacityFactor Real opacity is set opacity * m_OpacityFactor/255
    */
    void SetOpacityFactor( wxUint8 OpacityFactor );

    //! see SetOpacityFactor()
    wxUint8 GetOpacityFactor() { return m_OpacityFactor; }

    //! does a derived drawer have alpha support or not
    virtual bool HasAlpha() { return false; }

    int ToDeviceLines( std::vector<wxRealPoint>& cpointsDouble, const a2dVertexList* list, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle );

    virtual void DrawShape( const a2dListId& Ids ) const {};
    virtual void DeleteShapeIds() {};
    virtual void DeleteShapeIds( const a2dListId& Ids ) {};

    const a2dListId& GetShapeIds() const { return m_shapeId; }

    void SetPreserveCache( bool cache ) { m_preserveCache = cache; }
    bool GetPreserveCache() { return m_preserveCache; }

public:

    //! Wrapper for DrawCharStroke
    /*!
        ISO C++ seems(?) to forbid directly calling the address of virtual member functions.
        Calling &a2dDrawer2D::DrawCharStroke refers to the actual function in a2dDrawer2D and
        does not refer to the virtual function.
        Calling &(this->DrawCharStroke) should refer to the virtual function and is accepted by
        some compilers (MSVC, gcc <3.3), but according to gcc 3.4 this is not allowed by ISO C++.
        Therefore the virtual function is called through this intermediate function
    */
    void DrawCharStrokeCb( wxChar c ) { DrawCharStroke( c ); }

    //! Wrapper for DrawCharFreetype
    /*! \sa DrawCharStrokeCb */
    void DrawCharFreetypeCb( wxChar c ) { DrawCharFreetype( c ); }

    //! Wrapper for DrawCharDc
    /*! \sa DrawCharStrokeCb */
    void DrawCharDcCb( wxChar c ) { DrawCharDc( c ); }

protected:

    //! sync with actual API graphical context
    /*!
        The stroke is set to the API that is doing the actual drawing.
        Depending on the API the m_drawStyle should be taken into account, if it does not already
        yield for any stroke set by SetDrawStyle()
    */
    virtual void SetActiveStroke( const a2dStroke& stroke );

    //! sync with actual API graphical context
    /*!
        The fill is set to the API that is doing the actual drawing.
        Depending on the API the m_drawStyle should be taken into account, if it does not already
        yield for any stroke set by SetDrawStyle()
    */
    virtual void SetActiveFill( const a2dFill& fill );

    virtual void DoSetDrawStyle( a2dDrawStyle drawstyle ) = 0;

    //! per drawer implementation
    virtual void DoSetActiveStroke() = 0;

    //! per drawer implementation
    virtual void DoSetActiveFill() = 0;

    virtual void DoSetActiveFont( const a2dFont& font ) {};

    //! get active stroke as wxPen
    wxPen ConvertActiveToPen();

    //! get active fill as wxBrush
    wxBrush ConvertActiveToBrush();

#if wxART2D_USE_GRAPHICS_CONTEXT
    wxGraphicsBrush ConvertActiveToGraphicsBrush( wxGraphicsContext* context );
    wxGraphicsPen ConvertActiveToGraphicsPen( wxGraphicsContext* context );
#endif ///wxART2D_USE_GRAPHICS_CONTEXT

    //! return true if the drawstyle in combination with the active stroke and fill,
    //! does not require filling
    inline bool IsStrokeOnly() const
    {
        return ( m_drawstyle == a2dWIREFRAME ||
                 m_drawstyle == a2dWIREFRAME_ZERO_WIDTH ||
                 m_drawstyle == a2dWIREFRAME_INVERT ||
                 m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH ||
                 m_activefill.IsNoFill() ||
                 m_activefill.GetStyle() == a2dFILL_TRANSPARENT
               );
    }

    //! return true if the the drawstyle in combination with the active stroke,
    //! does not require stroking
    inline bool IsStroked() const
    {
        return !m_activestroke.IsNoStroke() && !m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT;
    }


    //! convert vertex array containing line and arc segments in user coordinates to device coordinate lines.
    int ToDeviceLines( a2dVertexArray* points, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle = false );

    //! convert vertex list containing line and arc segments in user coordinates to device coordinate lines.
    int ToDeviceLines( const a2dVertexList* list, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle = false );

    //!Given the rectangle in world coordinate to be displayed, update the mappingmatrix.
    /*!
    To display all of a drawing, set this to the bounding box of the root object
    of the canvas.

    So vx1 and vx2 to the minimum x and y of the bounding box.
    Calculate xpp and ypp in such a manner that it will show the whole drawing.

    The buffer size or SetMappingDeviceRect() are not used when setting the mapping with this function.
    In fact all other mapping functions in a2dDrawer2D use this function to set the mapping for the a2dDrawer2D in the end.

    \remark when a relative world matrix SetTransform() is set, the m_usertodevice
       is recalculated to take the new mapping matrix and the already set
       user matrix into account.

    \param x:   map to device x (non zero if canvas window is smaller than drawer)
    \param y:   map to device y (non zero if canvas window is smaller than drawer)
    \param wx:  map to device of this size in x, normally same as width of buffer bitmap
    \param wy:  map to device of this size in y, normally same as height of buffer bitmap
    \param vx1: minimum x coordinate of display area
    \param vy1: minimum y coordinate of display area
    \param xpp: Number of world units per pixel in x
    \param ypp: Number of world units per pixel in y
    */
    void SetMappingUpp( double x, double y, double wx, double wy, double vx1, double vy1, double xpp, double ypp );

    //! converts internal device points array to spline.  Returns new number of points.
    unsigned int ConvertSplinedPolygon2( unsigned int n );

    //! converts internal device points array to spline. Returns new number of points.
    unsigned int ConvertSplinedPolyline2( unsigned int n );

    //! clip lines
    bool Clipping( double& x1, double& y1, double& x2, double& y2 );

    //! clip code of a point
    int  GetClipCode( double x, double y );

    //!Used for filling with Gradient fill style.
    void FillPolygon( int n, wxRealPoint points[] );

    //!Used for filling with Gradient fill style.
    bool MoveUp( int n, wxRealPoint points[] , double horline, int& index, int direction );

    //!Used for filling with Gradient fill style.
    void DetectCriticalPoints( int n, wxRealPoint points[] );

    // Core text drawing function.
    /*! This function will draw each character separately using the function drawchar.
        It will also advance and kern the characters, by adjusting the affine matrices.
        The character will also be checked against the clipbox.
        This function assumes (0,0) is the lowerleft bbox corner.
        This function will not draw the background fill.
        \param text The text to be drawn.
        \param x x-Position of the text.
        \param y y-Position of the text.
        \param drawchar A function which will draw a given character.
    */
    void DrawTextGeneric( const wxString& text, double x, double y, void ( a2dDrawer2D::*drawchar )( wxChar ) );

    //! Draw a cross instead of a character.
    virtual void DrawCharUnknown( wxChar c );

    //! Draw a cross, indicating an unsupported font type for this drawer.
    /*! Draws text at the given position in user coordinates.
        Adding rotation etc. to that matrix makes it possible to draw rotated text.
        \param text draw unknown text
        \param x x position
        \param y y position
        \param words If false, draw a boxed cross for the whole line.
                If true, draw a piece of line for each word.
    */
    virtual void DrawTextUnknown( const wxString& text, double x, double y, bool words = false );

    //! Draw a stroke character
    /*! This is an internal function, used by DrawTextStroke.
        This function assumes that the affine matrix has been set up by the calling function
        in such a way, that the (0,0) coordinate will match with the lowerleft bbox corner
        of the character. Also mirroring should be performed by the calling function.
    */
    virtual void DrawCharStroke( wxChar c );

    //! Draw text in user coordinates, based on a stroke font.
    /* The text will be drawn with (0,0) being the lowerleft bbox corner.
        The background fill and alignment positioning are handled by DrawText.
    */
    virtual void DrawTextStroke( const wxString& text, double x, double y );

    //! Draw a freetype character
    /*! This is an internal function, used by DrawTextFreetype
        This function assumes that the affine matrix has been set up by the calling function
        in such a way, that the (0,0) coordinate will match with the lowerleft bbox corner
        of the character. Also mirroring should be performed by the calling function.
    */
    virtual void DrawCharFreetype( wxChar c ) { DrawCharUnknown( c ); }

    //! Draw text in world coordinates, based on a freetype font .
    /* The text will be drawn with (0,0) being the lowerleft bbox corner.
        The background fill and alignment positioning are handled by DrawText.
    */
    virtual void DrawTextFreetype( const wxString& text, double x, double y )
    { DrawTextGeneric( text, x, y, &a2dDrawer2D::DrawCharFreetypeCb ); }

    //! Draw a dc character
    /*! This is an internal function, used by DrawTextDc
        This function assumes that the affine matrix has been set up by the calling function
        in such a way, that the (0,0) coordinate will match with the lowerleft bbox corner
        of the character. Also mirroring should be performed by the calling function.
    */
    virtual void DrawCharDc( wxChar c ) { DrawCharUnknown( c ); }

    //! Draw text in user coordinates, based on a dc font .
    /* The text will be drawn with (0,0) being the lowerleft bbox corner.
        The background fill and alignment positioning are handled by DrawText.
    */
    virtual void DrawTextDc( const wxString& text, double x, double y )
    { DrawTextGeneric( text, x, y, &a2dDrawer2D::DrawCharDcCb ); }

    //! Convert double points to integer coords in the point cache and return pointer.
    wxPoint* _convertToIntPointCache( int n, wxRealPoint* pts );

    //!buffer updating activity possible or not
    bool m_frozen;

    //!enable/ disable mouse events handling by canvas
    bool m_mouseevents;

    //!used while rendering
    a2dStroke m_currentstroke;

    //!used while rendering
    a2dFill m_currentfill;

    //!used while rendering
    a2dStroke m_activestroke;

    //!used while rendering
    a2dFill m_activefill;

    //!current font set
    a2dFont m_currentfont;

    //!drawstyle (like invert mode)
    a2dDrawStyle m_drawstyle;

    //! used to restore an overruled fix style
    a2dStroke m_fixStrokeRestore;

    //! used to restore an overruled fix style
    a2dFill m_fixFillRestore;

    //! used to restore an overruled fix style
    a2dDrawStyle m_fixDrawstyle;

    //! up or down
    bool m_yaxis;

    bool m_yaxisDevice;

    //! virtual coordinates box its miminum X
    double m_virt_minX;

    //! virtual coordinates box its miminum Y
    double m_virt_minY;

    //!user units per pixel in x
    double m_xpp;

    //!user units per pixel in y
    double m_ypp;

    //! keep track of this
    bool m_fixedStyledOverRuled;

    //! up or down
    bool m_forceNormalizedFont;

    wxUint16 m_smallTextThreshold;

    //! When display refresh is on hold, this triggers a delayed zoom signal, when released.
    bool m_pendingSig_changedZoom;

    //! used internally in some methods to avoid constructor, new, ...
    a2dVertexList m_tempPoints;

    a2dListId m_shapeId;
    a2dShapeIdVector m_shapeIds;
    bool m_preserveCache;

private:

    void ColourXYLinear( int x1, int x2, int y );
    void ColourXYRadial( int x1, int x2, int y );

    //! gradient start stop
    double m_dx1, m_dy1, m_dx2, m_dy2, m_radiusd, m_length, m_max_x, m_min_x, m_max_y, m_min_y;
    a2dLine m_line;

protected:
    
    virtual void DoSetMappingUpp() {};

    //! draw an internal  polygon in device coordinates
    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );

    //! draw an internal polyline in device coordinates
    virtual void DeviceDrawLines( unsigned int n, bool spline );

    //!draw in pixels
    virtual void DeviceDrawLine( double x1, double y1, double x2, double y2 );

    //! Draw a pixel-width, unstroked horizontal line in device (pixel) coordinates
    /*! This method is useful for implementing low level fill routines.

    \param x1  x start of line
    \param x2  x end of line
    \param y1  y of line
    \param use_stroke_color If true, then use the current stroke color
                            If false, then use the current fill color.
    */
    virtual void DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color );

    //! Draw a pixel-width, unstroked vertical line in device (pixel) coordinates
    /*! This method is useful for implementing low level fill routines.

    \param x1  x of line
    \param y1  y start of line
    \param y2  y end of line
    \param use_stroke_color If true, then use the current stroke color
                            If false, then use the current fill color.
    */
    virtual void DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color );

    //!draw a single, unstroked pixel in device coordinates with the given color
    virtual void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );

    //! active stroke alpha
    wxUint8 m_StrokeOpacityCol1;
    //! active stroke alpha
    wxUint8 m_StrokeOpacityCol2;
    //! active fill alpha
    wxUint8 m_FillOpacityCol1;
    //! active fill alpha
    wxUint8 m_FillOpacityCol2;

    //! active stroke colours
    unsigned char m_colour1redStroke;
    //! active stroke colours
    unsigned char m_colour1greenStroke;
    //! active stroke colours
    unsigned char m_colour1blueStroke;

    //! active fill colours
    unsigned char m_colour1redFill;
    //! active fill colours
    unsigned char m_colour1greenFill;
    //! active fill colours
    unsigned char m_colour1blueFill;

    //! active fill colours
    unsigned char m_colour2redFill;
    //! active fill colours
    unsigned char m_colour2greenFill;
    //! active fill colours
    unsigned char m_colour2blueFill;

    //! current layer
    wxUint16 m_layer;

    //!world to device coordinate mapping
    a2dAffineMatrix m_worldtodevice;

    //!pointer to current user-to-world transform matrix
    a2dAffineMatrix m_usertoworld;

    //!pointer to current user-to-device transform matrix ( so includes mapping matrix )
    a2dAffineMatrix m_usertodevice;

    //!is the virtual area set already (used during startup)
    bool m_virtualarea_set;

    //! see SetRefreshDisplay()
    int m_refreshDisplay;

    //!device size width
    int m_width;

    //!device size height
    int m_height;

    //! X mapping position in device coordinates
    int m_mapX;

    //! Y mapping position in device coordinates
    int m_mapY;

    //! width for mapping in device coordinates
    int m_mapWidth;

    //! height for mapping in device coordinates
    int m_mapHeight;

    //! critical point list for polygon
    a2dCriticalPointList m_CRlist;

    //! scanline list of intersections with polygon
    a2dAETList m_AETlist;

    //!cached array for containing transformed device coordinates (integer)
    std::vector<wxPoint> m_cpointsInt;

    //!cached array for containing transformed device coordinates (double)
    std::vector<wxRealPoint> m_cpointsDouble;

    //! accuracy of spline
    double m_splineaberration;

    //! pushed clipping regions
    a2dClipRegionList   m_clipregionlist;

    //!accuracy of arc segment calculation etc. in device coordinates
    double m_displayaberration;

    //! object smaller than this value will not be rendered
    wxUint16 m_drawingthreshold;

    //! polygon smaller than this value will be rendered non filled.
    wxUint16 m_polygonFillThreshold;

    //! underneath the threshold draw rectangles if true else nothing
    bool m_asrectangle;

    //! current clipping area in world coordinates
    a2dBoundingBox m_clipboxworld;

    //! current clipping area in device coordinates
    wxRect m_clipboxdev;

    //! counter for BeginDraw EndDraw calls
    int m_beginDraw_endDraw;

    //! view its window.
    wxWindow* m_display;

    //! all drawing is disabled when this is true
    bool m_disableDrawing;

    //! opacity will be the one set, derived by this
    wxUint8 m_OpacityFactor;

    //! Affine transform stack.
    /*! The stack actually is a double stack, for both m_relativetransform and m_usertodevice
        This prevents some additional affine matrix multiplications.
        The stack is implemented as a 'fixed' array, preventing (slow) dynamic memory allocation.
    */
    std::vector<a2dAffineMatrix> m_affineStack;

    //! If true use real scale else different scale by x and y
    bool m_realScale;

    //! can be used to modify drawing features when used as context for printing
    bool m_printingMode;

    DECLARE_CLASS( a2dDrawer2D )

public:

    //! id for changed zoom
    //! sent from a2dDrawer2D to a2dEventDistributer when zoom has changed.
    const static a2dSignal sig_changedZoom;

};

//! class draws nothing, still can be used as a drawing context.
/*!
    In cases where a drawing context is not really needed to draw, but
    one still wants to follow the same matrix multiplication as if drawn,
    this class can be used.
    It can simplifies the code a lot if there is a dummy drawer instead of non at all,
    which would require testing for that.
    A dummy drawer can be used in all cases where normally a real drawer is used.
*/
class A2DARTBASEDLLEXP a2dBlindDrawer2D : public a2dDrawer2D
{
public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dBlindDrawer2D( int w = 100, int h = 100 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dBlindDrawer2D( const wxSize& size );

    //!set buffer size to w pixel wide and h pixels heigh
    /*!
        \remark
        SetMappingDeviceRect() is NOT reset.
    */
    void SetBufferSize( int w, int h );

    wxBitmap GetBuffer() const { return wxBitmap( 0, 0 ); }

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    //!destructor
    virtual ~a2dBlindDrawer2D();

    //!copy constructor
    a2dBlindDrawer2D( const a2dBlindDrawer2D& other );

    a2dBlindDrawer2D( const a2dDrawer2D& other );

    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    void ResetStyle();

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw();

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw();

    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    void PopClippingRegion();

    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height );
    void SetClippingRegion( double minx, double miny, double maxx, double maxy );
    void DestroyClippingRegion();

    void DrawPolygon( a2dVertexArray* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    void DrawLines( a2dVertexArray* points, bool spline = false );

    void DrawLines( const a2dVertexList* list, bool spline = false );

    void DrawLine( double x1, double y1, double x2, double y2 );

    void DrawArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord );

    void DrawEllipticArc( double xc, double yc, double width, double height , double sa, double ea, bool chord );

    void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    void DrawCenterRoundedRectangle( double xc, double yc, double width, double height, double radius, bool pixelsize = false );

    void DrawCircle( double x, double y, double radius );

    void DrawEllipse( double x, double y, double width, double height );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    //more speedy then base, by directly converting to device coordinates.
    void DrawVpath( const a2dVpath* path );

    void DrawPoint( double xc, double yc );

    void DeviceDrawAnnotation( const wxString& text, wxCoord x, wxCoord y, const wxFont& font );

protected:

    virtual void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void DoSetActiveStroke();

    void DoSetActiveFill();

private:

    DECLARE_DYNAMIC_CLASS( a2dBlindDrawer2D )
};


#if wxART2D_USE_GRAPHICS_CONTEXT

//!  wxGraphicsContext based drawing context derived from a2dDrawer2D
/*!
    Use wxGraphicsContext from wxWidgets to draw.

    \ingroup drawer
*/
class a2dGcBaseDrawer : public a2dDrawer2D
{
    DECLARE_CLASS( a2dGcBaseDrawer )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dGcBaseDrawer( int width = 0, int height = 0, wxGraphicsRenderer* render = NULL, wxGraphicsContext* context = NULL );

    //! copy constructor
    a2dGcBaseDrawer( const a2dGcBaseDrawer& other );

    a2dGcBaseDrawer( const a2dDrawer2D& other );

    virtual ~a2dGcBaseDrawer();

    virtual void SetTransform( const a2dAffineMatrix& userToWorld );
    virtual void PushTransform();
    virtual void PushIdentityTransform();
    virtual void PushTransform( const a2dAffineMatrix& affine );
    virtual void PopTransform( void );

    virtual void BeginDraw();
    virtual void EndDraw();

    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos );

    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  ) = 0;

    void ResetStyle();

    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    void PopClippingRegion();

    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height );

    void SetClippingRegion( double minx, double miny, double maxx, double maxy );

    void DestroyClippingRegion();

    virtual void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    virtual void DrawCircle( double x, double y, double radius );

    void DrawPoint(  double xc,  double yc );

    virtual void DrawEllipse( double x, double y, double width, double height );

    virtual void DrawLines( const a2dVertexList* list, bool spline );

    virtual void DrawPolygon( a2dVertexArray* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void DrawLines( a2dVertexArray* points, bool spline = false );

    void DrawLine( double x1, double y1, double x2, double y2 );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawText( const wxString& text, double x, double y, int alignment = wxMINX | wxMINY, bool Background = true );

protected:

    virtual void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void DoSetActiveStroke();

    void DoSetActiveFill();

    void DoSetActiveFont( const a2dFont& font );

    wxRegion m_clip;

    wxGraphicsRenderer* m_render;
    wxGraphicsContext* m_context;
};


//!  wxGraphicsContext based drawing context derived from a2dDrawer2D
/*!
    Use wxGraphicsContext from wxWidgets to draw.

    \ingroup drawer
*/
class a2dNativeGcDrawer : public a2dGcBaseDrawer
{
    DECLARE_DYNAMIC_CLASS( a2dNativeGcDrawer )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dNativeGcDrawer( int width = 0, int height = 0, wxGraphicsRenderer* render = NULL, wxGraphicsContext* context = NULL );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dNativeGcDrawer( const wxSize& size );

    //! bitmap is converted to image on which you can draw.
    //! Use GetBuffer() to get a bitmap back.
    a2dNativeGcDrawer( const wxBitmap& bitmap );

    //!return buffer as a bitmap
    wxBitmap GetBuffer() const;

    //! copy constructor
    a2dNativeGcDrawer( const a2dNativeGcDrawer& other );

    a2dNativeGcDrawer( const a2dDrawer2D& other );

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    void CopyIntoBuffer( const wxBitmap& bitm );

    virtual ~a2dNativeGcDrawer();

    void InitContext();

    void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) );

    void ShiftBuffer( int dxy, bool yshift );

    void DrawPoint(  double xc,  double yc );

protected:

    //!the buffer that is used for rendering
    wxBitmap  m_buffer;

    //!Created at BeginDraw, and destoyed at EndDraw, used to actually draw
    wxMemoryDC m_memdc;
};
#endif  // wxART2D_USE_GRAPHICS_CONTEXT

#endif /* __WXDRAWER2D_H__ */

