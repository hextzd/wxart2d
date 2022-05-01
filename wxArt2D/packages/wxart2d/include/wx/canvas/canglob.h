/*! \file wx/canvas/canglob.h
    \brief general canvas module declarations and classes

    The event classes for an a2dCanvasObject are here.
    And a2dCanvasGlobal holds settings for the canvas module.
    Storing layers settings as a template for layer settings of new documents is a
    typical use of wxCanvasGlobals.

    a2dTimer, is a wxTimes special for a2dCanvasObject

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    Classes, functions and enumration for use in all othere files, are located here.

    RCS-ID: $Id: canglob.h,v 1.58 2009/09/03 20:09:53 titato Exp $
*/

#ifndef __WXCANGLOB_H__
#define __WXCANGLOB_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <map>

using namespace std;

#include "wx/geometry.h"
#include <wx/config.h>

#include <wx/module.h>

#include <wx/general/gen.h>
#include <wx/artbase/drawer2d.h>

#include "wx/canvas/candefs.h"

#include "wx/canvas/restrict.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/cantext.h"
#include "wx/canvas/objlist.h"

//! OR-ing a2dDocumentRenderStyle is allowed

inline a2dDocumentRenderStyle operator | ( a2dDocumentRenderStyle a, a2dDocumentRenderStyle b )
{
    return ( a2dDocumentRenderStyle ) ( ( wxUint32 ) a | ( wxUint32 ) b );
}

//--------------------------------------------------------------------
// a2dLayerGroup
//--------------------------------------------------------------------

//! holds a list of layers numbers
class a2dLayerGroup: public a2dlist< wxUint16 >
{
public:

    //! constructor
    a2dLayerGroup() {}

    //! return true if layer is part of the group
    //! if group eqauls wxLAYER_ALL it return true always
    bool InGroup( wxUint16 layer );
};

A2DCANVASDLLEXP wxString wxToString  ( const a2dLayerGroup&  group );
A2DCANVASDLLEXP bool wxFromString  ( const wxString &  string, a2dLayerGroup*  group );

class A2DARTBASEDLLEXP a2dAffineMatrix;

//--------------------------------------------------------------------
// a2dCanvasModule
//--------------------------------------------------------------------

#if wxUSE_PRINTING_ARCHITECTURE
#include "wx/paper.h"
#include "wx/print.h"
#endif

//! initiation module for the wxArt2D library
/*!
    One instance of is used to initiate global settings.

    Like a2dCanvasGlobals and a2dCanvasInitializeStockObjects()

    \ingroup global
*/
class A2DCANVASDLLEXP a2dCanvasModule : public wxModule
{
public:

    a2dCanvasModule();

    virtual bool OnInit();
    virtual void OnExit();

    static const a2dFont& GetFontMedBold() { return *m_FontMedBold; }
    static const a2dFont& GetFontMed() { return *m_FontMed; }
    static const a2dFont& GetFontSmall() { return *m_FontSmall; }
    static const a2dFont& GetFontMini() { return *m_FontMini; }

    static void SetFontMedBold( a2dFont& font ) { *m_FontMedBold = font; }
    static void SetFontMed( a2dFont& font ) { *m_FontMed = font; }
    static void SetFontSmall( a2dFont& font ) { *m_FontSmall = font; }
    static void SetFontMini( a2dFont& font ) { *m_FontMini = font; }

private:

    static a2dFont* m_FontMedBold;
    static a2dFont* m_FontMed;
    static a2dFont* m_FontMini;
    static a2dFont* m_FontSmall;

#if wxUSE_PRINTING_ARCHITECTURE
    wxPrintPaperDatabase* m_wxThePrintPaperDatabase;
#endif

    DECLARE_DYNAMIC_CLASS( a2dCanvasModule )

};

class A2DCANVASDLLEXP a2dIterC;
class A2DCANVASDLLEXP a2dCanvasObject;
class A2DCANVASDLLEXP a2dCanvasObjectList;

class A2DARTBASEDLLEXP a2dAffineMatrix;
class A2DGENERALDLLEXP a2dWalkerIOHandler;
class A2DCANVASDLLEXP a2dText;


BEGIN_DECLARE_EVENT_TYPES()
//!see a2dCanvasObjectMouseEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASOBJECT_MOUSE_EVENT, 1 )
//!see a2dCanvasObjectEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASOBJECT_ENTER_EVENT, 1 )
//!see a2dCanvasObjectEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASOBJECT_LEAVE_EVENT, 1 )
//! see a2dHandleMouseEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASHANDLE_MOUSE_EVENT, 1 )
//! see a2dHandleMouseEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASHANDLE_MOUSE_EVENT_ENTER, 1 )
//! see a2dHandleMouseEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASHANDLE_MOUSE_EVENT_LEAVE, 1 )
//!see a2dCanvasObjectEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASOBJECT_RESIZE_EVENT, 1 )
//!see a2dCanvasObjectEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CANVASOBJECT_POPUPMENU_EVENT, 1 )
END_DECLARE_EVENT_TYPES()

//! general event sent from a2dCanvasObject to itself
/*!
    It contains the accumulated matrix to reach the position etc. of the object.
    This is important for editing childobjects of a2dCanvasObject directly.

    Currently the following Id's:

*/
class A2DCANVASDLLEXP a2dCanvasObjectEvent : public wxEvent
{
public:

    //! constructor
    a2dCanvasObjectEvent( a2dIterC* ic, wxEventType type, a2dCanvasObject* object, double x, double y, int id = 0 );

    a2dCanvasObjectEvent( a2dCanvasObject* object, const a2dBoundingBox& box, int id = 0 );

    a2dCanvasObjectEvent( const a2dCanvasObjectEvent& other );

    //! clone the event
    virtual wxEvent* Clone() const { return new a2dCanvasObjectEvent( *this ); }

    a2dCanvasObject* GetCanvasObject() { return ( a2dCanvasObject* ) GetEventObject(); }

    double GetX() { return m_x; }
    double GetY() { return m_y; }

    //! in case of wxEVT_CANVASOBJECT_RESIZE_EVENT event get boundingbox which was sent
    a2dBoundingBox& GetBbox() { return m_box; }

    a2dIterC* GetIterC() { return m_ic; }
    // This is used when sending a2dCanvasObjectEvent's from a simple (handle less) tool
    void SetIterC( a2dIterC* ic ) { wxASSERT( !m_ic ); m_ic = ic; }

private:

    DECLARE_CLASS( a2dCanvasObjectEvent )

    //! x position of event
    double m_x;

    //! y position of event
    double m_y;

    //! the interative context, for getting matrix for conversion to absolute
    a2dIterC* m_ic;

    //! for storing info used in wxEVT_CANVASOBJECT_RESIZE_EVENT
    a2dBoundingBox m_box;
};

//! mouse event sent from a2dCanvasObject to itself
/*!
    It contains the accumulated matrix to reach the position etc. of the object.
    This is important for editing childobjects of a2dCanvasObject directly.
    The original mouse event ( e.g. as sent from the a2dDrawingPart its a2dCanvas ),
    is also stored as part of the event.

    Currently the following Id's:

    - ::wxEVT_CANVASOBJECT_MOUSE_EVENT

    - ::wxEVT_CANVASOBJECT_ENTER_EVENT when entering a a2dCanvasObject (also children)

    - ::wxEVT_CANVASOBJECT_LEAVE_EVENT when leaving a a2dCanvasObject (also children)

    - ::wxEVT_CANVASOBJECT_POPUPMENU_EVENT Right Mouse Down default handling

    \ingroup events
*/
class A2DCANVASDLLEXP a2dCanvasObjectMouseEvent : public a2dCanvasObjectEvent
{
public:

    //! constructor
    a2dCanvasObjectMouseEvent( a2dIterC* ic, a2dCanvasObject* object, a2dHit& how, double x, double y, const wxMouseEvent& event );

    //! constructor
    a2dCanvasObjectMouseEvent( a2dIterC* ic, a2dCanvasObject* object, wxEventType type, double x, double y, const wxMouseEvent& event );

    //! constructor
    a2dCanvasObjectMouseEvent( const a2dCanvasObjectMouseEvent& other );

    //! clone the event
    virtual wxEvent* Clone( bool WXUNUSED( deep ) = true ) const { return new a2dCanvasObjectMouseEvent( *this ); }

    //! return the object hit
    a2dCanvasObject* GetCanvasObject() { return ( a2dCanvasObject* ) GetEventObject(); }

    //! return the original mouse event that was redirected to the a2dCanvasObject
    wxMouseEvent& GetMouseEvent() { return m_mouseevent; }

    //! in what manner the a2dCanvasObject was hit.
    a2dHit& GetHow() { return m_how; }

private:

    DECLARE_CLASS( a2dCanvasObjectMouseEvent )

    //! original mouse event
    wxMouseEvent m_mouseevent;

    //! in what manner the a2dCanvasObject was hit.
    a2dHit m_how;
};

//! general event sent from a2dHandle to its parent a2dCanvasObject
/*!
    This event can be intercepted, to implement an action when a a2dHandle is hit.
    The original mouse event ( e.g. as sent from the a2dDrawingPart its a2dCanvas ),
    is also stored as part of the event.

    Currently the following Id's:

    - ::wxEVT_CANVASHANDLE_MOUSE_EVENT
    - ::wxEVT_CANVASHANDLE_MOUSE_EVENT_ENTER
    - ::wxEVT_CANVASHANDLE_MOUSE_EVENT_LEAVE

  \ingroup events
*/
class A2DCANVASDLLEXP a2dHandleMouseEvent : public a2dCanvasObjectEvent
{
public:

    //! constructor
    a2dHandleMouseEvent( a2dIterC* ic, a2dHandle* handle, double x, double y, const wxMouseEvent& event, wxEventType type = wxEVT_CANVASHANDLE_MOUSE_EVENT );

    //! constructor
    a2dHandleMouseEvent( const a2dHandleMouseEvent& other );

    //! clone the event
    virtual wxEvent* Clone( bool WXUNUSED( deep ) = true ) const { return new a2dHandleMouseEvent( *this ); }

    //! return the handle object hit
    a2dHandle* GetCanvasHandle() { return ( a2dHandle* ) GetEventObject(); }

    //! return the original mouse event that was redirected to the a2dHandle
    wxMouseEvent& GetMouseEvent() { return m_mouseevent; }

private:

    DECLARE_CLASS( a2dHandleMouseEvent )

    //! original mouse event
    wxMouseEvent m_mouseevent;
};

//! event function for  a2dCanvasObjectEvent
typedef void ( wxEvtHandler::*a2dCanvasObjectEventFunction ) ( a2dCanvasObjectEvent& );
//! event function for  a2dHandleMouseEvent
typedef void ( wxEvtHandler::*a2dCanvasHandleMouseEventFunction ) ( a2dHandleMouseEvent& );
//! event function for  a2dCanvasObjectMouseEvent
typedef void ( wxEvtHandler::*a2dCanvasObjectMouseEventFunction ) ( a2dCanvasObjectMouseEvent& );

/*! \addtogroup eventhandlers
*  @{
*/

//! static event table macro for a2dCanvasObject mouse event
#define EVT_CANVASOBJECT_MOUSE_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASOBJECT_MOUSE_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasObjectMouseEventFunction > ( & func ), (wxObject *) NULL ),
//! static event table macro for a2dCanvasObject mouse enter event
#define EVT_CANVASOBJECT_ENTER_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASOBJECT_ENTER_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasObjectMouseEventFunction > ( & func ), (wxObject *) NULL ),
//! static event table macro for a2dCanvasObject mouse leave event
#define EVT_CANVASOBJECT_LEAVE_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASOBJECT_LEAVE_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasObjectMouseEventFunction > ( & func ), (wxObject *) NULL ),
//! static event table macro for a2dHandle mouse event
#define EVT_CANVASHANDLE_MOUSE_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASHANDLE_MOUSE_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasHandleMouseEventFunction > ( & func ), (wxObject *) NULL ),
//! static event table macro for a2dHandle mouse enter event
#define EVT_CANVASHANDLE_MOUSE_EVENT_ENTER(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASHANDLE_MOUSE_EVENT_ENTER, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasHandleMouseEventFunction > ( & func ), (wxObject *) NULL ),
//! static event table macro for a2dHandle mouse leave event
#define EVT_CANVASHANDLE_MOUSE_EVENT_LEAVE(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASHANDLE_MOUSE_EVENT_LEAVE, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasHandleMouseEventFunction > ( & func ), (wxObject *) NULL ),
//! static event table macro for a2dCanvasObject mouse Right Down event
#define EVT_CANVASOBJECT_POPUPMENU_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASOBJECT_POPUPMENU_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasObjectMouseEventFunction > ( & func ), (wxObject *) NULL ),

//! static event table macro for a2dCanvasObject resize event
#define EVT_CANVASOBJECT_RESIZE_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANVASOBJECT_RESIZE_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dCanvasObjectEventFunction > ( & func ), (wxObject *) NULL ),

//!  @} eventhandlers

typedef unsigned int a2dCursorId;
typedef unsigned int a2dImageId;

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_NONE;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_ARROW;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_RIGHT_ARROW;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_BULLSEYE;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CHAR;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CROSS;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HAND;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_IBEAM;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_LEFT_BUTTON;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_MAGNIFIER;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_MIDDLE_BUTTON;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_NO_ENTRY;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_PAINT_BRUSH;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_PENCIL;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_POINT_LEFT;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_POINT_RIGHT;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_QUESTION_ARROW;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_RIGHT_BUTTON;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZENESW;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZENS;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZENWSE;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZEWE;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZING;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SPRAYCAN;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_WAIT;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_WATCH;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_BLANK;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_ARROWWAIT;

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Select;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Select2;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Zoom;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Drag;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Move;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Copy;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_DrawWire;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWire;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireVertex;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireDeleteVertex;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireSegment;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireSegmentHorizontal;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireSegmentVertical;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_WireSegmentInsert;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Edit;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditVertex;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditDeleteVertex;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditSegment;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditSegmentHorizontal;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditSegmentVertical;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SegmentInsert;

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleLeftDown;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleMove;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleDrag;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleEnter;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleLeave;

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CanvasObjectHit;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CanvasObjectEnter;
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CanvasObjectLeave;



A2DCANVASDLLEXP extern const a2dImageId a2dBitmap_EditRectangleFill;

//! provide cursors etc.
class A2DCANVASDLLEXP a2dArtProvider : public wxObject
{
public:

    a2dArtProvider();

    ~a2dArtProvider();

    virtual wxCursor& GetCursor( a2dCursorId id );

    virtual wxBitmap& GetImage( a2dImageId id );

    // when a cursor is not in here it will be loaded and stored in here for caching.
    map<a2dCursorId, wxCursor> m_cursorsLoaded;

    // when a bitmap is not in here it will be loaded and stored in here for caching.
    map<a2dImageId, wxBitmap> m_imageLoaded;

};

//forward decl
class A2DCANVASDLLEXP a2dDrawing;


enum a2dNextSeg
{
	WIREEND_STRAIGHT,
	WIREEND_MANHATTAN_X,
	WIREEND_MANHATTAN_Y
};

//! defines common settinsg for a habitat for a set of a2dCameleons.
/*!
    To centralize common settings to be used by all a2dCameleons and nested objects, 
    which are contained in the same habitat. This last is at least the a2dDrawing in
    which they are placed.

    \ingroup global
*/
class A2DCANVASDLLEXP a2dHabitat: public a2dObject
{

public:
    a2dHabitat();
    virtual ~a2dHabitat();

	void Init( a2dLayers* layers = NULL );

#if wxUSE_CONFIG
    void Load( wxConfigBase& config );

    void Save( wxConfigBase& config );
#endif // wxUSE_CONFIG

    void SetDefaultUnitsScale( double scale ) { m_defaultUnitsScale = scale; }
    double GetDefaultUnitsScale() { return m_defaultUnitsScale; }

    //!this is the number that defines the number of decimal places in the fraction
    /*!when writing numbers to a file.
       When reading a file, this number is set to the accuracy information available in the file.
       1/GetUnitsAccuracy() == 100 means 2 decimal places will be written, or the data just
       read is accurate to this number of decimal places.

       \remark GetUnitsAccuracy() can also be > 1
    */
    double GetUnitsAccuracy() const { return m_units_accuracy; }

    //!this is the number that defines the number of decimal places in the fraction
    /*!when writing numbers to a file.
       When reading a file, this number is set to the accuracy information available in the file.
       1/GetAccuracyUserUnits() == 100 means 2 decimal places will be written, or the data just
       read is accurate to this number of decimal places.
       \remark SetUnitsAccuracy() can also be > 1
    */
    void   SetUnitsAccuracy( double accuracy ) { m_units_accuracy = accuracy; }

    //! Normalize objects ( defined in coordinates -1 to 1 ranges ) will be multiplied by this factor.
    /*!
        Libraries containing object which are defined in normalized coordinates,
        need to be multiplied by a certain factor before they can be added to a document.
        This factor defines by how much that should be done.
        As an example a libary containing arrow objects for placing at the end of a2dEndsLine
        objects, will use this factor.
    */
    double GetNormalizeScale() const { return m_normalize_scale; }

    //! see GetNormalizeScale()
    void SetNormalizeScale( double scale ) { m_normalize_scale = scale; }

    //!this string defines the unit e.g. meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()

        \remark the value may also be "trees" "pixels" or "people"  etc.

        \remark conversion to other units is not just simply changing this string.
        You must Change the SetUnitsScale(double) or really scale the data itself.
    */
    wxString GetUnits() const { return m_units; }

    //!this string defines the unit e.g. meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()

        \remark the value may also be "trees" "pixels" or "people" etc.

        \remark conversion to other units is not just simply changing this string.
        You must Change the SetUnitsScale(double) or really scale the data itself.
    */
    void SetUnits( const wxString& unitString ) { m_units = unitString; }

    //! Get measuring unit (e.g. Meters Inch Microns etc.)
    /*!
        Can be used to measure distances in a different unit than m_units.
    */
    wxString GetUnitsMeasure() { return m_units_measure; }

    //! Set measuring unit (e.g. Meters Inch Microns etc.)
    /*!
        Can be used to measure distances in a different unit than m_units.
    */
    void SetUnitsMeasure( const wxString& unitMeasure ) { m_units_measure = unitMeasure; }

    double ACCUR();

    //!Get the central layersettings for the canvas library.
    /*!
        New a2dCanvasDocument's are initialized with this layer settings.
        A default Layer Setting is created in the constructor,
        but this one can be changed.
    */
    a2dLayers* GetLayerSetup() { return m_layersetup; }

    //!set the layersettings for the canvas library.
    /*! old layer setting will be deleted, and new one is owned.
    */
    void SetLayerSetup( a2dLayers* layersetup );

    void SetSymbolLayer( wxUint16 layerId ) { m_symbolLayer = layerId; }
    wxUint16 GetSymbolLayer() { return m_symbolLayer; }

    //! load a layer set in CVG format from a file, which will be used as a template
    //! for a2dCanvasDocument's its layer settings
    bool LoadLayers( const wxString& filename );

    void SetPinSize( double size ) { m_defPinSize = size; }

    double GetPinSize() { return m_defPinSize; }

    void SetHandleSize( double size ) { m_defHandleSize = size; }

    double GetHandleSize() { return m_defHandleSize; }

    //! set default pin for graph like structure
    void SetPin( a2dPin* newpin );

    //! get default pin for graph like structure
    a2dPin* GetPin();

    //! \see  GetPinCanConnect()
    void SetPinCanConnect( a2dPin* newpin );

    //! represents the pin styles for pins that can connect right now
    /*! This pin is usually bigger than the default pin and green */
    a2dPin* GetPinCanConnect();

    //! \see  GetPinCannotConnect()
    void SetPinCannotConnect( a2dPin* newpin );

    //! represnts the pin styles for pins that cannot connect right now
    /*! This pin is usually smaller than the default pin and red */
    a2dPin* GetPinCannotConnect();

    //! set default handle for editing
    void SetHandle( a2dHandle* newhandle );

    //! get default handle for editing
    a2dHandle* GetHandle();

    //! default text for displaying properties.
    a2dText* GetPropertyText();

    //! Get Stroke to use for Highlighted a2dCanvasObject's
    a2dStroke& GetHighLightStroke();

    //! Get Fill to use for Highlighted a2dCanvasObject's
    a2dFill& GetHighLightFill();

    //! Set Stroke to use for Highlighted a2dCanvasObject's
    void SetHighLightStroke( const a2dStroke& stroke );

    //! Set Fill to use for Highlighted a2dCanvasObject's
    void SetHighLightFill( const a2dFill& fill );

    //! Get Stroke to use for Selected a2dCanvasObject's
    a2dStroke& GetSelectStroke();

    //! Get Fill to use for Selected a2dCanvasObject's
    a2dFill& GetSelectFill();

    //! Get Stroke to use for Selected2 a2dCanvasObject's
    a2dStroke& GetSelect2Stroke();

    //! Get Fill to use for Selected2 a2dCanvasObject's
    a2dFill& GetSelect2Fill();

    //! Set Stroke to use for Selected a2dCanvasObject's
    void SetSelectStroke( const a2dStroke& stroke );

    //! Set Fill to use for Selected a2dCanvasObject's
    void SetSelectFill( const a2dFill& fill );

    //! Set Stroke to use for Selected2 a2dCanvasObject's
    void SetSelect2Stroke( const a2dStroke& stroke );

    //! Set Fill to use for Selected2 a2dCanvasObject's
    void SetSelect2Fill( const a2dFill& fill );

    //! sets draw style to be used for selected object
    void SetSelectDrawStyle( a2dDocumentRenderStyle drawstyle ) { m_SelectDrawStyle = drawstyle; }

    //! returns draw style to be used for selected object
    a2dDocumentRenderStyle GetSelectDrawStyle() const { return m_SelectDrawStyle; }

    void SetDrawStyle( wxUint32 drawstyle );

    //!get drawstyles used for drawing the document
    wxUint32 GetDrawStyle() { return m_documentDrawStyle; }

    //! Get Stroke to use for non layered a2dCanvasObject's
    a2dStroke& GetDefaultStroke();

    //! Get Fill to use for non layered a2dCanvasObject's
    a2dFill& GetDefaultFill();

    //! Set Stroke to use for non layered a2dCanvasObject's
    void SetDefaultStroke( const a2dStroke& stroke );

    //! Set Fill to use for non layered a2dCanvasObject's
    void SetDefaultFill( const a2dFill& fill );

    //! Set restriction engine (grid snapping) used normally
    void SetRestrictionEngine( a2dRestrictionEngine* restrict ) { m_restrictionEngine = restrict; }

    //! Get restriction engine (grid snapping)
    a2dRestrictionEngine* GetRestrictionEngine();

    //! Set class for generating new connection objects between object and pins
    void SetConnectionGenerator( a2dConnectionGenerator* connectionGenerator ) { m_connectionGenerator = connectionGenerator; };

    //! Get class for generating new connection objects between object and pins 
    a2dConnectionGenerator* GetConnectionGenerator() const { return m_connectionGenerator; }

    //! Set the display aberration of curved shapes
    /*!
        The drawing of curved shapes will not deviate more than this from the ideal curve.
        \remark the smaller the number the longer the drawing takes.

        \param aber maximum deviation in device coordinates
    */
    void SetDisplayAberration( double aber );

    //! Returns the display aberration of curved shapes
    /*!,
        The drawing of curved shapes will not deviate more than this from the ideal curve.
    */
    double GetDisplayAberration() const;

    //! hit marging in pixels.
    /*! This is used in editing, where a drawer is known and conversion from
        pixels to world units is possible.
        a2dDrawingPart copies this value in its constructor and it can be changed
        in a2dDrawingPart as well
    */
    wxUint16 GetHitMarginDevice() const { return m_hitmargin_device; }

    //! hit marging in pixels.
    /*! \see GetHitMarginDevice */
    void SetHitMarginDevice( wxUint16 val ) { m_hitmargin_device = val; }

    //! hit marging in world units.
    /*! This is used during command processing, where a drawer is not known.
    */
    double GetHitMarginWorld() const { return m_hitmargin_world; }

    //! hit marging in world units.
    /*! \see GetHitMarginWorld */
    void SetHitMarginWorld( double val ) { m_hitmargin_world = val; }

    //! get minimum distance of copied object from original
    double GetCopyMinDistance() const { return m_copymindistance; }

    //! set minimum distance of copied object from original
    void SetCopyMinDistance( double val ) { m_copymindistance = val; }

    //! coordinates below this distance will be treated as equal
    /*! This is used e.g. to check if two pins have the same location.
        This should be the machine presition with the used coordinate resolution
    */
    double GetCoordinateEpsilon() const { return m_coordinateepsilon; }

    //! \see GetCoordinateEpsilon
    void SetCoordinateEpsilon( double val ) { m_coordinateepsilon = val; }

    //!Set to draw layers in reverse order
    void SetReverseOrder( bool reverse_order ) { m_reverse_order = reverse_order; }

    //!Get Setting for draw layers in reverse order
    bool GetReverseOrder() const  { return m_reverse_order; }

    //! set true result in having straight lines rerouted between begin and end pin
    void SetRouteOneLine( bool oneLine ) { m_oneLine = oneLine; }

    //! see SetRouteOneLine()
    bool GetRouteOneLine() const { return m_oneLine; }

    //! sets operand A in layer operation
    void SetGroupA( const a2dLayerGroup& groupA ) { m_groupA = groupA; }

    //! sets operand B in layer operation
    void SetGroupB( const a2dLayerGroup& groupB ) { m_groupB = groupB; }

    //! return group A layers
    a2dLayerGroup& GetGroupA() { return m_groupA; }

    //! return group B layers
    a2dLayerGroup& GetGroupB() { return m_groupB; }

    //! add layer to group A layers
    void AddGroupA( wxUint16 layer ) { m_groupA.push_back( layer ); }

    //! add layer to group B layers
    void AddGroupB( wxUint16 layer ) { m_groupB.push_back( layer ); }

    //! set target layer in operation
    void SetTarget( wxUint16 target );

    //! get target layer in operation
    wxUint16 GetTarget() { return m_target; }

    //! set selectedOnlyA in group operations
    void SetSelectedOnlyA( bool selectedOnlyA ) { m_selectedOnlyA = selectedOnlyA; }

    //! get selectedOnlyA in group operations
    bool GetSelectedOnlyA() const { return m_selectedOnlyA; }

    //! set selectedOnlyB in group operations
    void SetSelectedOnlyB( bool selectedOnlyB ) { m_selectedOnlyB = selectedOnlyB; }

    //! get selectedOnlyB in group operations
    bool GetSelectedOnlyB() const { return m_selectedOnlyB; }

    //! boolean engine setting
    void SetBooleanEngineMarge( const a2dDoMu& marge ) { m_MARGE = marge; }
    //! boolean engine setting
    void SetBooleanEngineGrid( long grid ) { m_GRID = grid; }
    //! boolean engine setting
    void SetBooleanEngineDGrid( double dgrid ) { m_DGRID = dgrid; }
    //! boolean engine setting
    void SetBooleanEngineCorrectionAber( const a2dDoMu& aber ) { m_CORRECTIONABER = aber; }
    //! boolean engine setting
    void SetBooleanEngineCorrectionFactor( const a2dDoMu& aber ) { m_CORRECTIONFACTOR = aber; }
    //! boolean engine setting
    void SetBooleanEngineMaxlinemerge( const a2dDoMu& maxline ) { m_MAXLINEMERGE = maxline; }
    //! boolean engine setting
    void SetBooleanEngineWindingRule( bool rule ) { m_WINDINGRULE = rule; }
    //! boolean engine setting
    void SetBooleanEngineRoundfactor( double roundfac ) { m_ROUNDFACTOR = roundfac; }
    void SetBooleanEngineSmoothAber( const a2dDoMu& aber ) { m_SmoothAber = aber; }
    void SetBooleanEngineLinkHoles( bool doLinkHoles ) { m_LinkHoles = doLinkHoles; }
    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( const a2dDoMu& radiusMin ) { m_radiusMin = radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( const a2dDoMu& radiusMax ) { m_radiusMax = radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( const a2dDoMu& aber );
    //! Arc to Polygon/polyline Maximum abberation
    void SetAberArcToPoly( const a2dDoMu& aber );
    //! detection of small object, smaller than this
    void SetSmall( const a2dDoMu& smallest );

    //! for operation ConvertPointsAtDistance, this will be the distance the vertexes will have
    void SetDistancePoints( const a2dDoMu& distancePoints ) { m_distancePoints = distancePoints; }
    //! for operation ConvertPointsAtDistance, this will creates vertexes at the original vertexes also if set true.
    void SetAtVertexPoints( bool atVertexPoints ) { m_atVertexPoints = atVertexPoints; }

    bool GdsIoSaveTextAsPolygon( bool onOff );
    bool KeyIoSaveTextAsPolygon( bool onOff );

    bool GetGdsIoSaveTextAsPolygon() const;
    bool GetKeyIoSaveTextAsPolygon() const;

    //! Polygon/polyline to Arc Minimum radius to test
    const a2dDoMu&  GetRadiusMin() const { return m_radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    const a2dDoMu& GetRadiusMax() const { return m_radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    const a2dDoMu& GetAberPolyToArc() const { return m_AberPolyToArc; }
    //! Arc to Polygon/polyline Maximum abberation
    const a2dDoMu& GetAberArcToPoly() const { return m_AberArcToPoly; }
    //! detection of small object, smaller than this
    const a2dDoMu& GetSmall() const { return m_small; }

    //! for operation ConvertPointsAtDistance, this will be the distance the vertexes will have
    const a2dDoMu& GetDistancePoints() { return m_distancePoints; }
    //! for operation ConvertPointsAtDistance, this will creates vertexes at the original vertexes also if set true.
    bool GetAtVertexPoints() { return m_atVertexPoints; }

    //! set default grid setting X in world coordinates ( vertexes/coordinates as used in a2dDrawing )
    void SetGridX( double gridX );
    //! get default grid setting Y in world coordinates ( vertexes/coordinates as used in a2dDrawing )
    void SetGridY( double gridY );

    //! get default setting X  in world coordinates ( vertexes/coordinates as used in a2dDrawing )
    double GetGridX() const { return m_gridX; }

    //! get default grid setting Y  in world coordinates ( vertexes/coordinates as used in a2dDrawing )
    double GetGridY() const { return m_gridY; }

    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineMarge() const { return m_MARGE; }
    //! boolean engine setting
    long GetBooleanEngineGrid() const { return m_GRID; }
    //! boolean engine setting
    double GetBooleanEngineDGrid() const { return m_DGRID; }
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineCorrectionAber() const { return m_CORRECTIONABER; }
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineCorrectionFactor() const { return m_CORRECTIONFACTOR; }
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineMaxlinemerge() const  { return m_MAXLINEMERGE; }
    //! boolean engine setting
    bool GetBooleanEngineWindingRule() const  { return m_WINDINGRULE; }
    //! boolean engine setting
    double GetBooleanEngineRoundfactor() const  { return m_ROUNDFACTOR; }
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineSmoothAber() const  { return m_SmoothAber; }
    //! boolean engine setting
    bool GetBooleanEngineLinkHoles() const  { return m_LinkHoles; }

    //! set clear of target in group operations
    /*!
        if set, in boolean operation the target layer will be cleared before adding results ot it.
    */
    void SetClearTarget( bool clearTarget ) { m_clearTarget = clearTarget; }

    //! get clear of target in group operations
    bool GetClearTarget() const { return m_clearTarget; }

    //! set last mouse X position in world coordinates, as set by tools.
    /*!
        This position is used to set cursor position relative to the last entered position.
        The position can be set by drawing tools, when it makes sence to draw a next coordinates
        relative to the current/last entered coordinate.
    */
    void SetLastXyEntry( double x, double y ) { m_lastX = x; m_lastY = y; }

    //! holds last mouse X position in world coordinates, as set by tools.
    double GetLastXEntry() const { return m_lastX; }
    //! holds last mouse Y position in world coordinates, as set by tools.
    double GetLastYEntry() const { return m_lastY; }

    //! set RelativeStart measurement X and Y position in world coordinates
    /*!
        The RelativeStart start is used when mesuring using a tool, or from the commandline.
    */
    void SetRelativeStart( double x, double y ) { m_RelativeStartX = x; m_RelativeStartY = y; }

    //! holds last RelativeStart measurement X position in world coordinates
    double GetRelativeStartX() const { return m_RelativeStartX; }
    //! holds last RelativeStart measurement Y position in world coordinates
    double GetRelativeStartY() const { return m_RelativeStartY; }

    //!get the current stroke
    /*!
        Return the current stroke of the current document,
        if there is no document, return a2dBLACK_STROKE
    */
    const a2dStroke& GetStroke() const { return m_currentstroke; }

    //!get the current fill
    /*!
        Return the current fill of the current document,
        if there is no document, return a2dBLACK_FILL
    */
    const a2dFill& GetFill() const { return m_currentfill; }

    //!used for new objects etc. to set the stroke
    void SetStroke( const a2dStroke& stroke );

    //!used for new objects etc. to set the stroke
    void SetStroke( const unsigned char red, const unsigned char green, const unsigned char blue );

    //!used for new objects etc. to set the fill
    /*!
        \param fill to set, if NULL a2dNullFILL will be used
    */
    void SetFill( const a2dFill& fill );

    //!used for new objects etc. to set the fill
    void SetFill( const unsigned char red, const unsigned char green, const unsigned char blue );

    //!certain drawing tools like polygon drawing use this setting
    bool GetSpline() const {return m_spline;}

    void SetSpline( bool spline ) { m_spline = spline; }

    //!get current line begin object for line and polyline.
    /*!\return NULL if not set. */
    a2dCanvasObject* GetLineBegin() const { return m_currentBegin;}

    //!get current line end object for line and polyline.
    /*!\return NULL if not set. */
    a2dCanvasObject* GetLineEnd() const { return m_currentEnd;}

    //!set current line begin object for line and polyline.
    void SetLineBegin( a2dCanvasObject* begin ) { m_currentBegin = begin; }

    //!set current line end object for line and polyline.
    void SetLineEnd( a2dCanvasObject* end ) { m_currentEnd = end; }

    //!scaling in X for begin and end objects of lines and polylines
    void SetEndScaleX( double xs ) { m_currentXscale = xs; }

    //!scaling in Y for begin and end objects of lines and polylines
    void SetEndScaleY( double ys ) { m_currentYscale = ys; }

    //!get scaling in X for begin and end objects of lines and polylines
    double GetEndScaleX() const {return m_currentXscale;}

    //!get scaling in Y for begin and end objects of lines and polylines
    double GetEndScaleY() const {return m_currentYscale;}

    //!set the Contour width of shapes
    /*
    */
    void SetContourWidth( const a2dDoMu& currentContourWidth );

    //!get the Contour width of shapes in meters
    const a2dDoMu& GetContourWidth() const  { return m_currentContourWidth; }

    //! Set when m_contourwidth != 0 what is the end of the line should be.
    void SetPathType( a2dPATH_END_TYPE pathtype );

    //! get when m_contourwidth != 0 what is the end of the line looks like.
    a2dPATH_END_TYPE GetPathType() { return m_pathtype; }

    //!used for to set the font for non Text objects
    void SetFont( const a2dFont& font );

    //! get current font active font.
    const a2dFont& GetFont() const { return m_font; }

    //!If set, the text objects will use their own style ( GetTextStroke() etc. )
    void SetTextOwnStyle( bool ownStyle ) { m_textOwnStyle = ownStyle; }

    //!get the current text stroke
    bool GetTextOwnStyle() const { return m_textOwnStyle; }

    //!used for new text objects etc. to set the stroke
    void SetTextStroke( const a2dStroke& stroke );

    //!get the current text stroke
    /*!
        Return the current text stroke.
    */
    a2dStroke GetTextStroke() const;

    //!used for new text objects etc. to set the fill
    /*!
        \param fill to set, if NULL a2dNullFILL will be used
    */
    void SetTextFill( const a2dFill& fill );

    //!get the current text fill
    /*!
        Return the current text fill.
    */
    a2dFill GetTextFill() const;

    //!used for new text objects etc. to set the font
    void SetTextFont( const a2dFont& font );

    //! get current font active text font ( like used in text tool ).
    a2dFont GetTextFont() const;

    //! set template for line object
    a2dText* GetTextTemplateObject() { return m_textTemplateObject; }

    //! get template for text objects
    void SetTextTemplateObject( a2dText* textTemplateobject ) { m_textTemplateObject = textTemplateobject; }

    //!layer to be set for new objects.
    //!Default is layer wxLAYER_DEFAULT (colours etc taken from layer list)
    void SetLayer( wxUint16 layer, bool setStyleOfLayer = false );

    //!layer set for new objects.
    inline wxUint16 GetLayer() const { return m_layer; }

    //! used for objects which depend in size on this grid
    /*!
        The size of such object can be modified
    */
    void SetObjectGridSize( double objectGridSize ) { m_objectGridSize = objectGridSize; }

    //! used for objects which depend in size on this grid
    double GetObjectGridSize() { return m_objectGridSize; }

    //! When auto generating symbols, ports will be placed at this distance
    void SetPortDistance( double distance ) { m_portDistance = distance; }

    //! \see SetPortDistance()
    double GetPortDistance() { return m_portDistance; }

    //! When auto generating symbols, width of symbol will be this
    void SetSymbolSize( double size ) { m_symbolSize = size; }

    //! \see SetSymbolSize()
    double GetSymbolSize() { return m_symbolSize; }

    void SetDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetDrawingTemplate() const;

    void SetSymbolDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetSymbolDrawingTemplate() const;

    void SetBuildInDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetBuildInDrawingTemplate() const;

    void SetDiagramDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetDiagramDrawingTemplate() const;

    void SetGuiDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetGuiDrawingTemplate() const;

	//! a2dDrawWirePolylineLTool segment mode initilization 
	a2dNextSeg GetEndSegmentMode() { return m_endSegmentMode; }

	//! a2dDrawWirePolylineLTool segment mode initilization
	void SetEndSegmentMode( a2dNextSeg mode ) { m_endSegmentMode = mode; }

	virtual bool EditCanvasObject( a2dCanvasObject* original, a2dCanvasObject* copy );

protected:

    //!default setup for layers in a2dCanvasDocument's
    a2dSmrtPtr<a2dLayers>  m_layersetup;

    a2dSmrtPtr<a2dDrawing> m_drawingTemplate;

    a2dSmrtPtr<a2dDrawing> m_drawingTemplateSymbol;

    a2dSmrtPtr<a2dDrawing> m_drawingTemplateDiagram;

    a2dSmrtPtr<a2dDrawing> m_drawingTemplateBuildIn;

    a2dSmrtPtr<a2dDrawing> m_drawingTemplateGui;

	a2dNextSeg m_endSegmentMode;

    //! object to clone to use as start
    a2dSmrtPtr<a2dText> m_textTemplateObject;

    bool m_textOwnStyle;

    double m_defPinSize;
    double m_defHandleSize;

    a2dPinPtr m_defPin;
    a2dPinPtr m_defCanConnectPin;
    a2dPinPtr m_defCannotConnectPin;

    a2dHandlePtr m_defHandle;

    a2dSmrtPtr<a2dText> m_propertytext;

    //! Stroke to use for Highlighted a2dCanvasObject's
    a2dStroke m_HighLightStroke;

    //! Fill to use for Highlighted a2dCanvasObject's
    a2dFill   m_HighLightFill;

    //! Stroke to use for selected a2dCanvasObject's
    a2dStroke m_SelectStroke;

    //! Fill to use for selected a2dCanvasObject's
    a2dFill   m_SelectFill;

    //! Stroke to use for selected2 a2dCanvasObject's
    a2dStroke m_Select2Stroke;

    //! Fill to use for selected2 a2dCanvasObject's
    a2dFill   m_Select2Fill;

    //! Stroke to use for non layered a2dCanvasObject's
    a2dStroke m_DefaultStroke;

    //! Fill to use for non layered a2dCanvasObject's
    a2dFill   m_DefaultFill;
    
    //! draw style to be used for selected object
    a2dDocumentRenderStyle m_SelectDrawStyle;

    //! draw style to be used for drawing with e.g. a2dDrawingPart.
    a2dDocumentRenderStyle m_documentDrawStyle;

    //! restriction engine to be used usually (if zero don't restrict)
    a2dRestrictionEnginePtr m_restrictionEngine;

    //! generator for new connections
    a2dConnectionGeneratorPtr m_connectionGenerator;

    //! hit marging in pixels.
    /*! This is used in editing, where a drawer is known and conversion from
        pixels to world units is possible.
        a2dDrawingPart copies this value in its constructor and it can be changes
        in a2dDrawingPart as well
    */
    wxUint16 m_hitmargin_device;

    //! hit marging in world units.
    /*! This is used during command processing, where a drawer is not known.
    */
    double m_hitmargin_world;

    //! the minimum distance in world units of a copied object from the original
    double m_copymindistance;

    //! \see GetCoordinateEpsilon
    double m_coordinateepsilon;

    //!draw document in reverse order if set, used as default value for a2dCanvasDocument.
    bool m_reverse_order;

    //! used in routing wires to be straight or manhatten
    bool m_oneLine;

    wxUint16 m_symbolLayer;

    //! used for objects which depend in size on this grid
    double m_objectGridSize;

    //! \see SetPortDistance()
    double m_portDistance;

    //! \see SetSymbolSize()
    double m_symbolSize;

    double m_default_acc;
    double m_defaultUnitsScale;
   //! how many decimal places in fraction are valid
    double m_units_accuracy;
    //! unit (e.g. Meters Inch Microns etc.)
    wxString m_units;
    //! normalization factor to scale normalized objects.
    double m_normalize_scale;
    //! measuring unit (e.g. Meters Inch Microns etc.)
    wxString m_units_measure;

    a2dDoMu m_MARGE;
    //! boolean operation setting
    long   m_GRID;
    //! boolean operation setting
    double m_DGRID;
    //! boolean operation setting
    a2dDoMu m_CORRECTIONABER;
    //! boolean operation setting
    a2dDoMu m_CORRECTIONFACTOR;
    //! boolean operation setting
    a2dDoMu m_SMOOTHABER;
    //! boolean operation setting
    a2dDoMu m_MAXLINEMERGE;
    //! boolean operation setting
    bool   m_WINDINGRULE;
    //! boolean operation setting
    double m_ROUNDFACTOR;
    //! boolean operation setting
    a2dDoMu m_SmoothAber;
    //! boolean operation setting
    bool m_LinkHoles;
    //! Polygon/polyline to Arc Minimum radius to test
    a2dDoMu m_radiusMin;
    //! Polygon/polyline to Arc Maximum radius to test
    a2dDoMu m_radiusMax;
    //! Polygon/polyline to Arc Maximum abberation
    a2dDoMu m_AberPolyToArc;
    //! Arc to Polygon/polyline Maximum abberation
    a2dDoMu m_AberArcToPoly;
    //! for operation ConvertPointsAtDistance, this will be the distance the vertexes will have
    a2dDoMu m_distancePoints;
    //! for operation ConvertPointsAtDistance, this will creates vertexes at the original vertexes also if set true.
    bool m_atVertexPoints;


public:

    //! for operation on groups of layers
    a2dLayerGroup m_groupA;

    //! for operation on groups of layers
    a2dLayerGroup m_groupB;

    //! target layer in operations
    wxUint16 m_target;

    //! operations on m_groupA but only selected objects
    bool m_selectedOnlyA;
    //! operations on m_groupB but only selected objects
    bool m_selectedOnlyB;
    //! clear target layer before adding results of an operation
    bool m_clearTarget;
    //! boolean operation setting

    //! detect samll objects
    a2dDoMu m_small;

    //! canvas default grid in X  in world coordinates ( vertexes/coordinates as used in a2dDrawing )
    double m_gridX;
    //! canvas default grid in Y  in world coordinates ( vertexes/coordinates as used in a2dDrawing )
    double m_gridY;

    //! spline polygons
    bool m_spline;

    //!begin point scaling
    double  m_currentXscale;

    //!end point scaling
    double  m_currentYscale;

    //! current contour width e.g for polyline with width.
    a2dDoMu m_currentContourWidth;

    //! when m_contourwidth != 0 what is the end of the line/polyline.
    a2dPATH_END_TYPE m_pathtype;

    //! layer to be set for new objects
    wxUint16 m_layer;

    //!current font set
    a2dFont m_font;

    //! holds last mouse X position in world coordinates, as set by tools.
    double m_lastX;
    //! holds last mouse Y position in world coordinates, as set by tools.
    double m_lastY;

    //! The RelativeStart start is used when mesuring using a tool, or from the commandline.
    double m_RelativeStartX;

    //! The RelativeStart start is used when mesuring using a tool, or from the commandline.
    double m_RelativeStartY;

    bool m_keySaveTextAsPolygon;
    bool m_gdsiiSaveTextAsPolygon;
    bool m_keySaveFromView;
    bool m_gdsiiSaveFromView;

    //! fill to be set for new objects
    a2dFill m_currentfill;

    //! stroke to be set for new objects
    a2dStroke m_currentstroke;

    //! current beginobject for lines and polylines
    a2dCanvasObjectPtr m_currentBegin;

    //! current endobject for lines and polylines
    a2dCanvasObjectPtr m_currentEnd;

public:

    //! when active drawing is set, this signal is send to the drawing.
    static const a2dSignal sig_changedFill; 
    static const a2dSignal sig_changedStroke; 
    static const a2dSignal sig_changedFont; 
    static const a2dSignal sig_changedLayer; 
    static const a2dSignal sig_changedTarget; 
    static const a2dSignal sig_SetPathType; 
    static const a2dSignal sig_SetContourWidth; 

    static const a2dSignal sig_changedTextFill; 
    static const a2dSignal sig_changedTextStroke; 
    static const a2dSignal sig_changedTextFont; 
	
protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return NULL;
    }
};

typedef a2dSmrtPtr<a2dHabitat> a2dHabitatPtr;

#include <map>
class A2DGENERALDLLEXP a2dDrawingId;
typedef std::map< wxString, a2dDrawingId* > a2dDrawingIdMap;

class a2dDrawingId
{

public:

	a2dDrawingId();

	a2dDrawingId( const wxString& name );

	//! are they eqaul
    bool operator== ( const a2dDrawingId& a )
	{
		return m_id == a.m_id;
	}

    //! are they NOT eqaul
    bool operator!= ( const a2dDrawingId& a )
	{
		return m_id != a.m_id;
	}

    friend inline bool operator==( const a2dDrawingId& a, const a2dDrawingId& b )
    {
        return a.m_id == b.m_id;
    }

    friend inline bool operator!=( const a2dDrawingId& a, const a2dDrawingId& b )
    {
        return a.m_id != b.m_id;
    }

	static const a2dDrawingId& GetItemByName( const wxString& a2dDrawingIdName );


	wxUint16 m_id;
	wxString m_name;

    //! stored map of names to get id
    static a2dDrawingIdMap& GetHashMap();

};

#define DECLARE_DRAWING_ID( drawingName ) extern const a2dDrawingId& drawingName() ; 

#define DEFINE_DRAWING_ID( drawingName ) const a2dDrawingId& drawingName() \
{ \
    static a2dDrawingId item( wxT(#drawingName) ); \
    return item; \
}

DECLARE_DRAWING_ID( a2dDrawingId_noId )
DECLARE_DRAWING_ID( a2dDrawingId_cameleonrefs )
DECLARE_DRAWING_ID( a2dDrawingId_normal )
DECLARE_DRAWING_ID( a2dDrawingId_physical )
DECLARE_DRAWING_ID( a2dDrawingId_programming )
DECLARE_DRAWING_ID( a2dDrawingId_appearances )

//! contains global settings for the a2dCanvas library
/*!
    Settings and function using those settings or stored here.
    There is only ONE a2dCanvasGlobals instance for the whole a2dCanvas Library.

    \ingroup global

*/
class A2DCANVASDLLEXP a2dCanvasGlobal: public a2dGeneralGlobal
{

    DECLARE_CLASS( a2dCanvasGlobal )

public:
    a2dCanvasGlobal();
    virtual ~a2dCanvasGlobal();

    wxPoint m_initialDocChildFramePos;
    wxSize m_initialDocChildFrameSize;
    long m_initialDocChildFrameStyle;
    long m_initialWorldCanvasStyle;

    bool WINDINGRULE;

    //! Path for Icons and small bitmaps
    a2dPathList& GetLayersPathList() const { return m_layersPath; }

    a2dArtProvider* GetArtProvider() { return m_artProvider; }

    void SetArtProvider( a2dArtProvider* artProvider );

    wxCursor& GetCursor( a2dCursorId id ) { return m_artProvider->GetCursor( id ); }

    void SetPopBeforePush( bool popBeforePush ) { m_popBeforePush = popBeforePush; }

    bool GetPopBeforePush() { return m_popBeforePush; }

    //! when set, tools are pushed in one shotmode in PushTool()
    void SetOneShotTools( bool oneShotTools ) { m_oneShotTools = oneShotTools; }

    //! see SetOneShotTools()
    bool GetOneShotTools() { return m_oneShotTools; }

    //! when set, drawing tools are going into edit mode after drawing
    void SetEditAtEndTools( bool editAtEndTools ) { m_editAtEndTools = editAtEndTools; }

    //! see SetEditAtEndTools
    bool GetEditAtEndTools() { return m_editAtEndTools; }

    //! set the drawing part that has the focus/is active in a window.
    //! if force is true, an event sig_changedActiveDrawing is emmitted,
    //! else only if active drawing part is indeed different.
    void SetActiveDrawingPart( a2dDrawingPart* part, bool force = false );

    //! return the currently/last active drawing part.
    a2dDrawingPart* GetActiveDrawingPart() { return m_activeDrawingPart; }

    a2dHabitat* GetHabitat() const { return m_habitat; }

    void SetHabitat( a2dHabitat* habitat ) { m_habitat = habitat; }

	virtual a2dHabitat* GetHabitatByDrawingId( const a2dDrawingId& drawingId ) const { return m_habitat; }

protected:

    a2dHabitatPtr m_habitat;

    a2dDrawingPart* m_activeDrawingPart;

    a2dArtProvider* m_artProvider;

    //! pop a tool before a new one is pushed
    bool m_popBeforePush;

    //! tools only do one action then they are poped
    bool m_oneShotTools;

    //! drawing tools and with directly pushing the edit tool.
    bool m_editAtEndTools; 

    static a2dPathList m_layersPath; 

public:

    static const a2dPropertyIdPathList PROPID_layerPathList;

    //! when active drawing is set, this signal is send to the drawing.
    static const a2dSignal sig_changedActiveDrawing; 
};

//! global a2dCanvasGlobal to have easy access to global settings
A2DCANVASDLLEXP_DATA( extern a2dCanvasGlobal* ) a2dCanvasGlobals;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------


//! a2dEvtHandler event function for wxTimerEvent
typedef void ( wxEvtHandler::*wxCanvasObjectTimerEventFunction )( wxTimerEvent& );

/*! \addtogroup eventhandlers
*  @{
*/

//! timer event for a2dCanvasObject from a2dTimer
#define EVT_CANVASOBJECT_TIMER(timerid, func) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_TIMER, timerid, -1, (wxObjectEventFunction) wxStaticCastEvent( wxCanvasObjectTimerEventFunction, & func ), NULL),

//!  @} eventhandlers

//! wxArt2D specific a2dTimer which notifies a2dCanvasObject's
/*!
    \ingroup events
*/
class a2dTimer : public wxTimer
{
public:

    a2dTimer()
    {
        Init();
        m_canvasObjectOwner = NULL; m_id = 0;
    }

    //! constructor
    /*!
        \param owner to which a2dCanvasObject/a2dEvtHandler should the event be sent.
        \param id event id
    */
    a2dTimer( wxEvtHandler* owner, int id = -1 )
    {
        m_canvasObjectOwner = owner;
        m_id = id;
    }

    //! set object to recieve timer event.
    /*!
        \param owner to which a2dCanvasObject/a2dEvtHandler should the event be sent.
        \param id event id
    */
    void SetOwner( wxEvtHandler* owner, int id = -1 )
    {
        m_canvasObjectOwner = owner;
        m_id = id;
    }

    //! called by a2dTimer when time event is issued.
    //! This sends the event to the m_owner
    virtual void Notify()
    {
        wxCHECK_RET( m_canvasObjectOwner, _T( "a2dTimer::Notify() should be overridden." ) );

#if !wxCHECK_VERSION(2,9,0)
        wxTimerEvent event( m_id, m_milli );
#else
        wxTimerEvent event( *this );
#endif
        m_canvasObjectOwner->ProcessEvent( event );
    }

protected:

    unsigned long m_id;
    wxEvtHandler* m_canvasObjectOwner;
};


#endif
