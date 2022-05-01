/*! \file wx/canvas/candefs.h
    \brief defenitions an no more
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    This is to be included in at first, mostly via canglob.h already

    RCS-ID: $Id: candefs.h,v 1.15 2008/09/05 19:01:10 titato Exp $
*/

#ifndef __WXCANDEFS_H__
#define __WXCANDEFS_H__



#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/geometry.h"

#include <wx/module.h>

#include "wx/general/gen.h"
#include <wx/artbase/drawer2d.h>

#ifdef A2DCANVASMAKINGDLL
#define A2DCANVASDLLEXP WXEXPORT
#define A2DCANVASDLLEXP_DATA(type) WXEXPORT type
#define A2DCANVASDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DCANVASDLLEXP WXIMPORT
#define A2DCANVASDLLEXP_DATA(type) WXIMPORT type
#define A2DCANVASDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DCANVASDLLEXP
#define A2DCANVASDLLEXP_DATA(type) type
#define A2DCANVASDLLEXP_CTORFN
#endif

#ifdef A2DEDITORMAKINGDLL
#define A2DEDITORDLLEXP WXEXPORT
#define A2DEDITORDLLEXP_DATA(type) WXEXPORT type
#define A2DEDITORDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DEDITORDLLEXP WXIMPORT
#define A2DEDITORDLLEXP_DATA(type) WXIMPORT type
#define A2DEDITORDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DEDITORDLLEXP
#define A2DEDITORDLLEXP_DATA(type) type
#define A2DEDITORDLLEXP_CTORFN
#endif

//#define wxLAYER_DEFAULT 0 /*!< default layer for new objects. */
#define wxLAYER_ALL wxINT16_MAX
#define wxMAXLAYER (wxLAYER_ALL-1)
#define wxLAYER_DEFAULT 0 /*!< default layer for new objects. */

//! Define the manner in which a2dDrawingPart draws the document to the device.
/*!
The render style tells how a a2dDrawingPart should draw when it is updating parts from the document.
This might overrule of the style ( a2dFill a2dStroke ) set by a2dCanvasObjects.
More then one style can be set, all those styles will be rendered one after the other.

\ingroup drawer
*/
enum a2dDocumentRenderStyle
{
    /*! normal drawing style, canvasobjects are filled and outlined according to the style ( a2dStroke a2dFill ) set */
    RenderLAYERED = 0x00000001,
    /*! only a wire frame of the drawn canvasobjects is drawn, as is set by the its a2dStroke */
    RenderWIREFRAME = 0x00000002,
    /*! only a zero width wire frame of the drawn canvasobjects is drawn, Color of the canvasobject a2dStroke, is used */
    RenderWIREFRAME_ZERO_WIDTH = 0x00000004,
    /*! as RenderWIREFRAME, but the outline is the inverted of what is on the device/buffer */
    RenderWIREFRAME_INVERT = 0x00000008,
    /*! as RenderINVERT_WIREFRAME but zero width outline */
    RenderWIREFRAME_INVERT_ZERO_WIDTH = 0x00000010,
    /*! fixed stroke and fill are used */
    RenderFIX_STYLE = 0x00000020,
    /*! render rectangles for all direct child objects of ShowObject */
    RenderRectangles = 0x00000040,
    /*! Only a wire frame of the drawn primitive is drawn, with the style of the a2dDrawer::m_selectStroke */
    RenderWIREFRAME_SELECT = 0x00000200,
    /*! Only an inverted wire frame of the drawn canvasobjects is drawn, using the width of the wxLAYER_SELECTED a2dStroke */
    RenderWIREFRAME_SELECT_INVERT = 0x00000400,
    /*! render tool object */
    RenderTOOL_OBJECTS = 0x00001000,
    /*! render tool objects, using style of object itself */
    RenderTOOL_OBJECTS_STYLED = 0x00002000,
    /*! render tool decoration objects, using style of object itself */
    RenderTOOL_DECORATIONS = 0x00004000,
    /*! render rectangles for all direct child objects of ShowObject Which are tool objects. */
    RenderRectangleTOOL_OBJECTS = 0x00008000,
    /*! Only a wire frame of the drawn primitive is drawn, with the style of the RenderWIREFRAME_HighLight specific a2dStroke */
    RenderWIREFRAME_HighLight = 0x00010000,
    /*! Draw using layerseting but all at once and not in layer order. */
    RenderLAYERED_All = 0x00020000,
    /*! Only a wire frame of the drawn primitive is drawn, with the style of the a2dDrawer::m_select2Stroke */
    RenderWIREFRAME_SELECT2 = 0x00040000,

    //! what is normal
    RenderDefault = RenderLAYERED | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2
};

class A2DCANVASDLLEXP a2dExtendedResult;

class A2DCANVASDLLEXP a2dCanvas;

class A2DCANVASDLLEXP a2dCanvasObject;
class A2DCANVASDLLEXP a2dCanvasObjectList;

class A2DCANVASDLLEXP a2dLayers;

class A2DCANVASDLLEXP a2dDrawing;

class A2DCANVASDLLEXP a2dStyleProperty;
class A2DCANVASDLLEXP a2dShadowStyleProperty;
class A2DCANVASDLLEXP a2dCanvasObjectPtrProperty;

class A2DCANVASDLLEXP a2dDrawingPart;
class A2DCANVASDLLEXP a2dBaseTool;
class A2DCANVASDLLEXP a2dToolContr;

class A2DCANVASDLLEXP a2dIterC;
class A2DCANVASDLLEXP a2dBoudingBoxProperty;

class A2DCANVASDLLEXP a2dComEvent;
class A2DCANVASDLLEXP a2dCanvasObjectEvent;
class A2DCANVASDLLEXP a2dCanvasObjectMouseEvent;
class A2DCANVASDLLEXP a2dHandleMouseEvent;

class A2DCANVASDLLEXP a2dPin;
class A2DCANVASDLLEXP a2dText;

class A2DCANVASDLLEXP a2dPolygonL;

//! mask flags for a2dCanvasObject
/*!
 \ingroup canvasobject
*/
typedef wxUint64 a2dCanvasObjectFlagsMask;

//! a2dCanvasObject flags as a structure \ingroup canvasobject
/*!
 \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCanvasOFlags
{

public:

    //! Flags for a2dCanvasObject
    /*!
    Those flags or used in mask for set flag inside a a2dCanvasObject or
    to get the value of a flag.

    \sa  a2dCanvasObject_FlagsS
    \sa  a2dCanvasObjectFlagsMask

       \ingroup canvasobject
    */
    static const a2dCanvasObjectFlagsMask NON                ; /*!< No flags */
    static const a2dCanvasObjectFlagsMask SELECTED           ; /*!< object is selected */
	static const a2dCanvasObjectFlagsMask SELECTED2          ; /*!< object is selected2 */
    
	static const a2dCanvasObjectFlagsMask SELECTPENDING      ; /*!< object is pending for change in select state */

    static const a2dCanvasObjectFlagsMask SELECTABLE         ; /*!< can select object */
    static const a2dCanvasObjectFlagsMask HasSelectedObjectsBelow ; /*!< selected objects detected below */
    static const a2dCanvasObjectFlagsMask SubEdit            ; /*!< allow subedit of children within this object */
    static const a2dCanvasObjectFlagsMask SubEditAsChild     ; /*!< allow subedit on this object when child of other object */
    static const a2dCanvasObjectFlagsMask VISIBLE            ; /*!< is the object visible (overruled by parent object in some cases during rendering */
    static const a2dCanvasObjectFlagsMask DRAGGABLE          ; /*!< can be dragged */
    static const a2dCanvasObjectFlagsMask SHOWSHADOW         ; /*!< show shadow object if true and shadow property is available */
    static const a2dCanvasObjectFlagsMask FILLED             ; /*!< use fill to fill if set */
    static const a2dCanvasObjectFlagsMask A                  ; /*!< group A flag (e.g boolean operations) */
    static const a2dCanvasObjectFlagsMask B                  ; /*!< group B flag (e.g boolean operations) */
    static const a2dCanvasObjectFlagsMask BIN                ; /*!< general flag  ( used for temporarely marking object ) */
    static const a2dCanvasObjectFlagsMask BIN2               ; /*!< general flag  ( used for temporarely marking object ) */
    static const a2dCanvasObjectFlagsMask PENDING            ; /*!< set when a2dCanvasObject needs an update (redraw) */
    static const a2dCanvasObjectFlagsMask SNAP               ; /*!< snapping of editable object or when moving */
    static const a2dCanvasObjectFlagsMask SNAP_TO            ; /*!< snapping of other object to this when in place*/
    static const a2dCanvasObjectFlagsMask PUSHIN             ; /*!< push into allowed */
    static const a2dCanvasObjectFlagsMask PRERENDERASCHILD   ; /*!< as child this object will be rendered before the parent object itself when true (default) */
    static const a2dCanvasObjectFlagsMask VISIBLECHILDS      ; /*!< child objects are visible or not */
    static const a2dCanvasObjectFlagsMask EDITABLE           ; /*!< object can be edited */
    static const a2dCanvasObjectFlagsMask ChildrenOnSameLayer; /*!< child objects are rendered when the object is rendered itself.
                                                   The effect is that the children will appear on the same layer as the object. */
    static const a2dCanvasObjectFlagsMask DoConnect          ; /*!< in case of pins on the object is it currely allowed for other object to connect to it? */
    static const a2dCanvasObjectFlagsMask IsOnCorridorPath   ; /*!< this object is on the corridor path to a captured object */
    static const a2dCanvasObjectFlagsMask HasPins            ; /*!< true if this object does have a2dPin's as children */
    static const a2dCanvasObjectFlagsMask Editing            ; /*!< true if the object is currently being edited */
    static const a2dCanvasObjectFlagsMask EditingCopy        ; /*!< true if the object needs is the editcopy of an object that is edited. */
    static const a2dCanvasObjectFlagsMask IsProperty         ; /*!< True if object is a property show object */
    static const a2dCanvasObjectFlagsMask MouseInObject      ; /*!< true is mouse is currently within object */
    static const a2dCanvasObjectFlagsMask HighLight          ; /*!< true is object is highlighted */
    static const a2dCanvasObjectFlagsMask AlgoSkip           ; /*!< skip object in walker algorithms */
    static const a2dCanvasObjectFlagsMask ignoreSetpending   ; /*!< ignore Setpending calls on a2dCanvasObject */
    static const a2dCanvasObjectFlagsMask HasToolObjectsBelow  ; /*!< special tool object detected below */
    static const a2dCanvasObjectFlagsMask ChildOnlyTranslate   ; /*!< do not tranfrom only translate. */
    static const a2dCanvasObjectFlagsMask ignoreLayer          ; /*!< ignore Layer id, just render always*/
    static const a2dCanvasObjectFlagsMask generatePins         ; /*!< generate Pins when asked for  */
    static const a2dCanvasObjectFlagsMask normalized           ; /*!< normalized coordinates */
    static const a2dCanvasObjectFlagsMask NoRenderCanvasObject ; /*!< do not render if no children */
    static const a2dCanvasObjectFlagsMask FixedStyle           ; /*!< style on object fixed */
    static const a2dCanvasObjectFlagsMask ALL                  ;  /*!< To set all flags at once */

    a2dCanvasOFlags( a2dCanvasObjectFlagsMask newmask = NON );

    void SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which );

    a2dCanvasObjectFlagsMask GetFlags() const;

    void SetFlags( a2dCanvasObjectFlagsMask which );

    bool GetFlag( a2dCanvasObjectFlagsMask which ) const;

    bool CheckMask( a2dCanvasObjectFlagsMask mask ) const;

    //!object is selected
    bool m_selected: 1;

    //!object is selected2
    bool m_selected2: 1;

    //!can select object
    bool m_selectable: 1;

    //!can select pending
    bool m_selectPending: 1;

    //!allow subedit of children within this object
    bool m_subEdit: 1;

    //!allow subedit on this object when child of other object
    bool m_subEditAsChild: 1;

    //!is the object visible (overruled by paranet object in some cases during rendering
    bool m_visible: 1;

    //!can be dragged
    bool m_draggable: 1;

    //!show shadow object if true and shadow property is available
    bool m_showshadow: 1;

    //!use fill to fill if set
    bool m_filled: 1;

    //!group A flag (e.g boolean operations)
    bool m_a: 1;

    //!group B flag (e.g boolean operations)
    bool m_b: 1;

    //! generate pins if true
    bool m_generatePins: 1;

    //!general flag ( used for temporarely marking object )
    bool m_bin: 1;

    //!general flag  ( used for temporarely marking object )
    bool m_bin2: 1;

    //! set when a2dCanvasObject needs an update (redraw).
    bool m_pending: 1;

    //!snapping of editable object or when moving
    bool m_snap: 1;

    //!snapping of other objects to this when in place
    bool m_snap_to: 1;

    //! is push into allowed
    bool m_pushin;

    //!as child this object will be rendered before the parent object itself when true (default)
    bool m_prerenderaschild: 1;

    //! child objects are visible or not
    bool m_visiblechilds: 1;

    //! object can be edited
    bool m_editable: 1;

    //! if set fixed style (stroke and fill )is assumed. Can be used in tools to detect if style should be changed or not.
    bool m_fixedStyle: 1;

    //! true if the object is currently being edited
    bool m_editing: 1;

    //! true if the object needs to be rendered in edit mode.
    bool m_editingCopy: 1;

    //! ignore this object in a2dCanvasObject::SetPending()
    bool m_ignoreSetpending: 1;

    //! child objects are rendered when the object is rendered itself.
    //! The effect is that the children will appear on the same layer as the object.
    bool m_childrenOnSameLayer: 1;

    //! in case of pins on the object is it currely allowed for other object to connect to it?
    bool m_doConnect: 1;

    //! This object is on the corridor path to a captured object or to a corridored object.
    /*!
        When an object is captured, it and its parent, grandparent etc. objects get this flag set.
    */
    bool m_isOnCorridorPath: 1;

    //! true if this object does have a2dPin's as children
    bool m_hasPins: 1;

    bool m_IsProperty: 1;

    bool m_MouseInObject: 1;

    bool m_HighLight: 1;

    bool m_AlgoSkip: 1;

    bool m_ChildOnlyTranslate: 1;

    //! when this is set DoRender() is called even if layer of the object is not the layer to be rendered.
    bool m_ignoreLayer: 1;

    //! GDSII format flag
    bool m_template: 1 ;

    //! GDSII format flag
    bool m_external : 1 ;

    //! GDSII format flag
    bool m_used : 1 ;

    //! special tool object detected below
    bool m_HasToolObjectsBelow : 1 ;

    //! true is the object is a a2dCanvasObject (or derived ) without rendering something if there or now children.
    bool m_NoRenderCanvasObject : 1 ;

    //! selected objects detected below
    bool m_HasSelectedObjectsBelow : 1 ;

    //! when true, the wxEVT_CANVASOBJECT_RESIZE_EVENT is sent to this object, if the child box did change.
    bool m_resizeToChilds : 1 ;

    //! true if the object is defined in normalized coordinates
    bool m_normalized: 1;

};



#endif
