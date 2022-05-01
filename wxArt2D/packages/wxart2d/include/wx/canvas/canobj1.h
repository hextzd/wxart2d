/*! \file wx/canvas/canobj1.h
    \brief The base class for all drawable objects in a a2dDrawing

    \author Klaas Holwerda 

    Copyright: 2000-2004 (c) Klaas Holwerda 

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj.h,v 1.85 2009/07/10 19:23:13 titato Exp $
*/

#ifndef __WXCANOBJ1_H__
#define __WXCANOBJ1_H__

//----------------------------------------------------------------------------
// decls
//----------------------------------------------------------------------------

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dCanvasObject>;
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dDrawingPart>;
#endif
#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dPropertyIdTyped<a2dCanvasObjectPtr, a2dCanvasObjectPtrProperty>;
template class A2DCANVASDLLEXP a2dPropertyIdProp<a2dStyleProperty>;
template class A2DCANVASDLLEXP a2dPropertyIdProp<a2dShadowStyleProperty>;
template class A2DCANVASDLLEXP a2dPropertyIdTyped<a2dBoundingBox, a2dBoudingBoxProperty>;
#endif
typedef a2dPropertyIdTyped<a2dCanvasObjectPtr, class a2dCanvasObjectPtrProperty> a2dPropertyIdCanvasObject;
typedef a2dPropertyIdProp<class a2dStyleProperty> a2dPropertyIdCanvasStyle;
typedef a2dPropertyIdProp<class a2dShadowStyleProperty> a2dPropertyIdCanvasShadowStyle;
typedef a2dPropertyIdTyped<a2dBoundingBox, a2dBoudingBoxProperty> a2dPropertyIdBoundingBox;
typedef wxUint8 a2dTag;
typedef vector<a2dTag> a2dTagVec;
typedef a2dPropertyIdTyped< a2dTagVec, class a2dTagVecProperty > a2dPropertyIdTagVec;

class A2DCANVASDLLEXP a2dPinClass;
class A2DCANVASDLLEXP a2dHandle;

//! Enum for hit test options

enum a2dHitOption
{
    //! no special options
    a2dCANOBJHITOPTION_NONE = 0x0000,

    //! if set, respect layer order, hit testing is done per layer from the top.
    a2dCANOBJHITOPTION_LAYERS = 0x0001,
    //! if set, don't hit test the root object or object for which IsHitWorld is called
    /*!
        This option hit tests the child objects as seen from the parent object.
    */
    a2dCANOBJHITOPTION_NOROOT = 0x0002,

    //! object hit should not have select flag set
    a2dCANOBJHITOPTION_NOTSELECTED = 0x0004,

    //! if set, don't stop at the first hit, but test child/sibling objects anyway
    /*!
        For accurate hitting of close parallel lines. If object earlier on layer, is within tolerance, it would be hit,
        even if other lines are more close to the cursor, or cursor is on top of a later.
    */
    a2dCANOBJHITOPTION_ALL    = 0x0008,
};

//! OR-ing a2dHitOption is allowed

inline a2dHitOption operator | ( a2dHitOption a, a2dHitOption b )
{
    return ( a2dHitOption ) ( ( int ) a | ( int ) b );
}



enum wxEditStyle
{
    //! no special flags set
    wxEDITSTYLE_NONE     = 0x0000,

    //! edit a copy of the original object
    /*! There are two reasons, why you might want to edit a copy:

        - In layered drawings, one might want to see the effect of the layering
          during editing (e.g. that the edited object ist partially obscured),
          while at the same time the outline of the object and the editing handles
          should be rendered on top. This behaviour is best implemented by having
          two objects, an editing object with handles on top and the usual object.

        - in wxEDIT_COPY mode the editing information is transfered from the editing
          object to the original object using the command processor. This way the command
          processor is not only used for undo and redo but for all editing. This results
          in a more robust architecture (especially concerning undo/redo).

        If this style is not set, the original object is edited directly. Commands
        are still generated for undo/redo but the original object is not changed
        by these commands, so undo/redo is less robust.
    */
    wxEDITSTYLE_COPY      = 0x0001,

    //! editing is completely controlled by tools. No handles are added
    /*! If a master tool is used, handle editing is not realy appropriate, because
        the editing tool is not active and handles are not visible before the user
        clicks somewhere. Also master tools  need fine control over editing and use
        tools that perform a very specific task. This can be done without handles.
    */
    wxEDITSTYLE_NOHANDLES = 0x0002,
};

inline wxEditStyle operator | ( wxEditStyle a, wxEditStyle b )
{
    return ( wxEditStyle ) ( ( int ) a | ( int ) b );
}

//----------------------------------------------------------------------------
// a2dCanvasObject
//----------------------------------------------------------------------------

//! event mask flags for a2dCanvasObject hit
/*! a2dCanvasObjectHitFlags used in a mask for defining how an object should be hit

 \ingroup canvasobject

*/
enum a2dCanvasObjectHitFlags
{
    a2dCANOBJ_EVENT_NON = 0x0001, //!< no hit will be reported
    a2dCANOBJ_EVENT_FILL = 0x0002, //!< hit if filled
    a2dCANOBJ_EVENT_STROKE = 0x0004, //!< hit if stroked
    a2dCANOBJ_EVENT_FILL_NON_TRANSPARENT = 0x0008, //!< hit if filling is not transparent
    a2dCANOBJ_EVENT_STROKE_NON_TRANSPARENT = 0x0010, //!< hit if stroke is not transparent
    a2dCANOBJ_EVENT_VISIBLE = 0x0020, //!< hit if visible
    a2dCANOBJ_EVENT_ALL = 0x0040 //!< always hit
};

//! a2dCanvasObject hit flags as a structure \ingroup canvasobject
/*! a2dCanvasOHitFlags is used as mask for defining how an object should be hit
*/
struct a2dCanvasOHitFlags
{
    //!no events
    bool m_non: 1;
    //! only when over filled area of object
    bool m_fill: 1;
    //! only when over stroked area of object
    bool m_stroke: 1;
    //! only when non transparent fill
    bool m_fill_non_transparent: 1;
    //! only when non transparent stroke
    bool m_stroke_non_transparent: 1;
    //! only when visible
    bool m_visible: 1;
    //!all events
    bool m_all: 1;

};

//! This is one hit result from a2dExtendedResult
class A2DCANVASDLLEXP a2dExtendedResultItem
{
public:
    //! Default Constructor
    a2dExtendedResultItem() {}
    //! Standard Constructor
    a2dExtendedResultItem( a2dCanvasObject* object, a2dCanvasObject* parent, a2dHit type, int level, int typeex )
    {
        m_object = object;
        m_parent = parent;
        m_type = type;
        m_level = level;
        m_typeex = typeex;
    }
    //! the hit object
    class a2dCanvasObject* GetObject() const { return m_object; };
    //! the parent object of the hit object
    class a2dCanvasObject* GetParent() const { return m_parent; };
    //! type of the hit
    const a2dHit& GetHitType() const { return m_type; }
    //! level of the hit object below the start (root) object
    int GetLevel() const { return m_level; }

protected:
    friend class a2dExtendedResult;
    //! the hit object
    a2dCanvasObject* m_object;
    //! the parent object of the hit object
    a2dCanvasObject* m_parent;
    //! type of the hit
    a2dHit m_type;
    //! level of the hit object below the start (root) object
    /*! note: the extended result can contain multiple hits on the same level */
    int m_level;
    //! Extended hit type
    /*! This is generally class specific.
        Derived classes must include the IDs for the base class.
        a2dCanvasObject does not define extended hit IDs.
    */
    int m_typeex;
};

//! An object of this class represents the context of a query like a hit test
/*! This is similar to a2dIterC, but it is not used for iteration, but for
    return values.
*/
class A2DCANVASDLLEXP a2dExtendedResult: public std::vector<a2dExtendedResultItem>
{
public:
    //! Default constructor
    a2dExtendedResult();
    //! Destructor
    ~a2dExtendedResult();
};

//! structure to give as parameter to member functions of a2dCanvasObject
/*!
    During event processing and hittesting in a a2dDrawingPart and a2dDrawing, this
    structure is used to give and assemble information while traversing the document.
*/
class a2dHitEvent
{

public:

    a2dHitEvent( double absx = 0, double absy = 0, bool continues = true,
                 a2dHitOption option = a2dCANOBJHITOPTION_NONE, bool wantExtended = false )
    {
        m_option = option;
        m_maxlevel = INT_MAX;
        m_relx = 0;
        m_rely = 0;
        m_x = absx;
        m_y = absy;
        m_processed = false;
        m_continue = continues;
        m_isHit = false;
        m_how = a2dHit::stock_nohit;
        m_id = 0;
        m_extendedWanted = wantExtended;
        m_xyRelToChildren = false;
        m_event = NULL;
    }

    void SetProcessed( bool val )
    {
#ifdef _DEBUG
        if ( val )
            m_processed = true;
        else
            m_processed = false;
#else
        m_processed = val;
#endif
    }

    //! (world coordinates) hit point x relative to the canvas object its parent object(s)
    double m_relx;
    //! (world coordinates) hit point y relative to the canvas object its parent object(s)
    double m_rely;
    //! (world coordinates) hit point x as in a2dDrawingPart or any other top level
    double m_x;
    //! (world coordinates) hit point y as in a2dDrawingPart or any other top level
    double m_y;
    //! is set, m_x and m_y are supplied relative to the child objects ( inverse object matrix m_lworld will be applied first )
    bool m_xyRelToChildren;
    //! the way to hit/traverse the document.
    wxUint32 m_option;
    //! return in which way the object was hit (stroke, fill, ...)
    a2dHit m_how;
    //! extended result information with e.g path to lead to the nested object hit
    a2dExtendedResult m_extended;
    //! fill m_extended or not
    bool m_extendedWanted;
    //! maximum level of hit object below this (this = level 0)
    int m_maxlevel;
    //! event to process in case of event processing call
    wxEvent* m_event;
    //! set if event was processed sofar
    bool m_processed;
    //! set if the event processing or hittest needs to continue after first hit
    bool m_continue;
    //! in the end if there was a hit (even if not processed event)
    bool m_isHit;
    //! application specific use
    int m_id;
};

//! if pointToSnap is closer to pointToSnapTo than bestPointSofar, make bestPointSofar that point. 
//! But only if it is within thresHoldWorld distance.
extern bool SetPointIfCloser( const a2dPoint2D& pointToSnapTo, const a2dPoint2D& pointToSnap, a2dPoint2D& bestPointSofar, double thresHoldWorld );

//!a2dCanvasObject is the base class for Canvas Objects.
/*!
    All objects for drawing on the a2dCanvas are derived from this class.
    A a2dDrawing is filled with instances of this object.
    a2dCanvasObject can have children, which are also a2dCanvasObjects.
    A a2dCanvasObject itself can be a child of more than one parent a2dCanvasObject.
    The reference counter takes care of deleting the object when all references are released.

    Child objects are stored in a2dCanvasObjectList m_childobjects, which is only created
    when the first child is added to the a2dCanvasObject. Else it points to wxNullCanvasObjectList.

    Each a2dCanvasObject can also have a list of a2dObject objects.
    They are stored in the a2dObjectList m_properties.

    Asking for the GetCount() of the above two lists, is save, since wxNullCanvasObjectList
    will return 0.

    Traversing a a2dDrawing is mainly by traversing of the child list of each a2dCanvasObject in the
    document in a recursive manner.
    Many functions in a2dCanvasObject need to traverse the document, therefore there is often a DoSomething
    function called from the base a2dCanvasObject, in order to do the object specific job while
    traversing the document.

    Rendering of a2dCanvasObject's is via the base Render function, which takes care
    of rendering child objects. It also does the clipping of objects to the area to be drawn.
    First it searches for style properties in the property list, if found they will be used
    to set the drawing style of the active a2dDrawingPart which is used to draw on a device.
    If no style properties are available, the layer settings are used to set the style of the a2dDrawingPart.
    Next to style properties, there can be other properties which influence the rendering of the object
    in general; e.g. the  a2dClipPathProperty, is pushed into the a2dDrawingPart to clip the child objects.
    After setting the style the rendering can start. First the children which have the flag
    m_prerenderaschild set are rendered. Next the a2dCanvasObject derived object itself is rendered
    via the virtual DoRender() method. After that the rest of the child objects are rendered.
    In the end visible properties are rendered, via their a2dCanvasObject, which they may use for displaying
    themselfs.

    It is possible to intercept mouse event on a a2dCanvasObject.
    a2dCanvasObject's receive there events after a hit of the mouse pointer was detected
    from within a a2dDrawingPart or a2dCanvas object.
    The lowest object seen from the a2dDrawingPart::ShowObject() receives the events first, and if skipped there
    and no othere child proesses the event, its parent will get the event etc.

    \remark When a style (fill or stroke) is set it will be used instead of the layer fill and stroke
    \remark fill style is inherited by children

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCanvasObject: public a2dObject
{

public:

    friend class a2dCanvasObjectFilter;
    friend class a2dCanvasObjectFilterLayerMask;
    friend class a2dCanvasObjectFilterLayerMaskNoToolNoEdit;

    //! used to tell which child object to render and to detect the need for it.
    struct RenderChild
    {
        bool m_prerender: 1; /*!< object marked for pre rendering ( before the parent ) */
        bool m_postrender: 1; /*!< object marked for post rendering ( after the parent ) */
        bool m_property: 1; /*!< object marked for property rendering */
    };


    DECLARE_EVENT_TABLE()

    //****************** CONSTRUCTION AND REF COUNTING ******************/
    /*! \name Construction and reference counting
    */
    //\{

    DECLARE_DYNAMIC_CLASS( a2dCanvasObject )

    //!constructor called by derived objects
    a2dCanvasObject( double x = 0 , double y = 0 );

    //!constructor using reference to existing canvas object
    a2dCanvasObject( const a2dCanvasObject& other, CloneOptions options, a2dRefMap* refs );

    inline a2dCanvasObject* TClone( CloneOptions options, a2dRefMap* refs = NULL ) { return ( a2dCanvasObject* ) Clone( options, refs ); }

    //!destructor called by derived objects
    virtual ~a2dCanvasObject();

    a2dHabitat* GetHabitat() const;

protected:

    //!Clone this object and return a pointer to the new object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

public:

    //! After constructing a derived instance from a string, using CreateObject(), the object may need to be filled
    //! with child objects. This is often by reading more from a file, or by calling this.
    virtual void Initialize() {}

    //! returns if this object does have nested objects
    /*!
        If the object has children it is nested, but if not it can still have nested objects. This is when
        a derived a2dCanvasObject does have a2dCanvasObject's members itself.
        This function should return true if an object does have nested children or members.
    */
    virtual bool IsRecursive();

    //! All direct a2dCanvasObject which are part of this one are made unique
    /*!
        The base implementation check all child objects in m_childobjects for being more then single referenced.
        If not a clone is created in put in place of the one there, leving the origenal to the other objects
        which owned that child too.
        Derived a2dCanvasObject must implement this, if it has members which can be multiple referenced.
    */
    virtual void MakeReferencesUnique();


    //\}
    //**************** END CONSTRUCTION AND REF COUNTING ****************/

    //****************** AFFINE TRANSFORMATION ******************/
    /*! \name Affine Transformation
        Each a2dCanvasObject has an affine Transformation to translate, rotate, scale,
        and skew it. The Transformations are mutiplied with the transformations of
        all parent objects up the show object of teh current view.
    */
    //\{
public:

    //!Rotates this object clockwise
    /*!
        Rotates this object clockwise: If you call <code>Rotate(20); Rotate(10);</code> the
        absolute rotation will be 30 degrees (if initially rotation was 0 degrees)
        \see a2dAffineMatrix::Rotate
        \see SetRotation

        \param rotation rotate by this angle in degrees
    */
    void Rotate( double rotation );

    //!Sets a rotation of this object
    /*!
        Sets in opposite to Rotate the absolute rotation of this object.
        \see a2dAffineMatrix::SetRotation
        \see Rotate

        \param rotation set rotation angle in degrees
    */
    void SetRotation( double rotation );

    //! Scale in x and y ( > zero)
    /*!
        \param scalex scalling in X
        \param scaley scalling in Y
    */
    void Scale( double scalex, double scaley );

    //!Mirrors this object in x or y orientation
    /*!
    \param x mirror at X-axis (horizontally orientation)
    \param y mirror at Y-axis (vertically orientation)
    */
    void Mirror( bool x = true, bool y = false );

    //! Skew in X
    /*!
        \param angle angle to skew in X
    */
    void SkewX( double angle );

    //! Skew in Y
    /*!
        \param angle angle to skew in Y
    */
    void SkewY( double angle );

    //!get the matrix used to position the object
    const a2dAffineMatrix& GetTransformMatrix() const { return m_lworld; }

    a2dAffineMatrix GetTransform() const { return m_lworld; }
    void SetTransform( a2dAffineMatrix mat = a2dIDENTITY_MATRIX ) { m_lworld = mat; SetPending( true ); }

    //! Returns the matrix used to position the object
    /*!
        \param mat matrix set for transforming the object
    */
    void SetTransformMatrix( const a2dAffineMatrix& mat = a2dIDENTITY_MATRIX ) { m_lworld = mat; SetPending( true ); }

    //!Sets the matrix used to position the object
    /*!
        A new matrix will be constructed for transforming this object.

        \see SetTransformMatrix
        \see a2dAffineMatrix::a2dAffineMatrix

        \param xt      x translation
        \param yt      y translation
        \param scalex  x scale factor
        \param scaley  y scale factor
        \param degrees rotation in degrees
     */
    void SetTransformMatrix( double xt, double yt, double scalex = 1, double scaley = 1, double degrees = 0 );

    //!get x position from affine matrix
    double GetPosX() const { return m_lworld.GetValue( 2, 0 ); }

    //!get y position from affine matrix
    double GetPosY() const { return m_lworld.GetValue( 2, 1 ); }

    //!get position of object
    a2dPoint2D GetPosXY() const { return a2dPoint2D( m_lworld.GetValue( 2, 0 ), m_lworld.GetValue( 2, 1 ) ); }

    //!set position to x,y
    /*! translation is set to x,y, rest of the affine matrix is preserved.
        \param x x position
        \param y y position
        \param restrict use a2dCanvasGlobals->GetRestrictionEngine() to snap position
    */
    void SetPosXY( double x, double y, bool restrict = false );

    //!set position to x,y
    /*! translation is set to x,y, rest of the affine matrix is preserved.
        \param pos position
    */
    void SetPosXyPoint( const a2dPoint2D& pos )
    {
        SetPosXY( pos.m_x, pos.m_y, false );
    }

    //!set position to x,y but restricted, returns new values
    /*! translation is set to x,y, rest of the affine matrix is preserved.

        Same as SetPoXY(), but returneing restricted values

        \param x x position
        \param y y position
        \return true of point was restricted/changed
    */
    bool SetPosXYRestrict( double& x, double& y );

    //!relative translate the object to position x,y in world coordinates
    /*! this function may be defined for each derived object, to move other object with this object.
        it is used internally for dragging and moving objects.
        \param x delta x for translation
        \param y delta y for translation
    */
    void Translate( double x, double y ) { m_lworld.Translate( x, y ); SetPending( true ); }

    //!transform the object using the given matrix
    /*!If the object can not rotate or scale those will be ignored
    and only translation will take place.
    If totally transformed it will return true else false.
    \param tworld matrix for transform
    */
    void Transform( const a2dAffineMatrix& tworld ) { m_lworld = tworld * m_lworld; SetPending( true ); }

    //! reduce matrix to identity
    /*! Override if possible to eliminate matrix of the object
        For a non Derived a2dCanvasObject this function multiplies its children
        with this object matrix, and reduces its own matrix to identity.
    */
    virtual bool EliminateMatrix();

    //\}
    //****************** END AFFINE TRANSFORMATION ******************/


    //****************** SNAP AND ALIGMENT ******************/
    /*! \name Snap and Alignment
        - Functions to deal with snapping the objects to the grid or other objects.
		- Alignment of objects using a calculated boundingbox. 
    */
    //\{

    //! called from an a2dRestrictionEngine, to restrict vertexes, lines, object to this object.
    /*!
        When other object, vertexes or lines are moved (e.g. by a tool), the restriction engine can be set to snap/restrict
        to neighbouring objects. Here you can define to which parts of those neighbour objects something can be snapped to.
        For a simple polygon this will be its vertexes, for an ellipse its extremes. For complex objects, it can be anything.
        For objects with pins, it will be the pins if that is asked for.
        For the various types of snap, you only implement what is useful for the object.

        The result of a possible snap point, is stored in a2dRestrictionEngine.

        The default can snap to:
            - pins snapToWhat == snapToWhat & a2dRestrictionEngine::snapToPins or a2dRestrictionEngine::snapToPinsUnconnected
            - the object its position snapToWhat == a2dRestrictionEngine::snapToObjectPos
            - the vector path conversion with GetAsCanvasVpaths() snapToWhat == a2dRestrictionEngine::snapToObjectVertexes
    */
    virtual bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld );

    //! return a vectorpath indicating on which point/segments the object likes to be snapped.
    /*!
        If a drag of this object is on going, one may want to snap itself to other objects, while dragging.
        This member functions will tell the a2dRestrictionEngine on which points this object likes to snap to others.

        The default implementation for a2dRestrictionEngine::snapToObjectVertexes tries to convert to a Vpath,
        and if so, uses that as return, if not, it takes the boundingbox its points.
        For a2dRestrictionEngine::snapToObjectPos its is the position of the object that is added.
        For a2dRestrictionEngine::snapToBoundingBox its is the BoundingBox points of the object that are added.
    */
    virtual a2dCanvasObjectList* GetSnapVpath( a2dSnapToWhatMask snapToWhat );

	//! Return a list of points to align object with others.
	/*!
		First 4 point are the boundingbox (without pins like objects).
		Next are the middle points along the boundingbox ( can be made object specific if wanted).
		Next the baseline for e.g. text.
		Application specific poinst after that.

		\remark return value must be deleted.
	*/
	virtual a2dVertexArray* GetAlignmentPoints();

	//! calls GetAlignmentPoints()
	a2dBoundingBox GetAlignmentBbox();

    //\}
    //****************** SNAP AND ALIGMENT ******************/


    //****************** BOUNDING BOX ******************/
    /*! \name Bounding box
        Each a2dCanvasObject has a Bounding box for efficient drawing and hit testing.
        The bounding box is always the bounding box of the vertices. It is extended
        in pixels or would units to cope for stroke width and decorations like handles.
    */
    //\{
public:

    //! flags for calculating boundingbox of derived object
    /*!
        Those flags are used in a a2dBboxFlags to indicate different ways of bounding boxes to be calculated.

        \sa DoGetUnTransformedBbox()
        \sa GetUnTransformedBbox()

        \ingroup canvasobject

    */
    enum a2dBboxFlag
    {
        a2dCANOBJ_BBOX_NON  = 0x0000,    //!< return full boundingbox of derived object
        a2dCANOBJ_BBOX_EDIT = 0x0001,     //!< return to contain edit bbox, suitable for editing matrix of object
        a2dCANOBJ_BBOX_CHILDREN = 0x0002 //!< return to contain children bbox
    };

    typedef unsigned int a2dBboxFlags;

    //!get minimum X of the boundingbox in world coordinates relative to its parents
    /*!
        \remark stroke width is not included.
    */
    double  GetBboxMinX()     { return GetBbox().GetMinX(); }

    //!get minimum Y of the boundingbox in world coordinates relative to its parents
    /*!
        \remark stroke width is not included.
    */
    double  GetBboxMinY()     { return GetBbox().GetMinY(); }

    //!get maximum X of the boundingbox in world coordinates relative to its parents
    /*!
        \remark stroke width is not included.
    */
    double  GetBboxMaxX()     { return GetBbox().GetMaxX(); }

    //!get maximum Y of the boundingbox in world coordinates relative to its parents
    /*!
        \remark stroke width is not included.
    */
    double  GetBboxMaxY()     { return GetBbox().GetMaxY(); }

    //!get width  of the boundingbox in world coordinates relative to its parents
    /*!
        \remark stroke width is not included.
    */
    double  GetBboxWidth()     { return GetBbox().GetWidth(); }

    //!get height of the boundingbox in world coordinates relative to its parents
    /*!
        \remark stroke width is not included.
    */
    double  GetBboxHeight()     { return GetBbox().GetHeight(); }

    //!get boundingbox in world coordinates exclusive stroke width relative to its parent
    /*!
        \remark stroke width is not included.

        \remark if the boundingbox is not valid, it will be calculated right now,
        and to make sure parent object will have the right boundingbox, the object is
        set pending. That will in the update of a2dDrawing, lead to redrawing and
        recalculation of this object and parent boundingbox will be recalculated.
    */
    a2dBoundingBox& GetBbox();

    //!Get boundingbox without the affine matrix transform included.
    /*!
        Extends as a result of stroke etc. is not included.

        \param flags default a2dCANOBJ_BBOX_CHILDREN to include child objects in the boundingbox
    */
    virtual a2dBoundingBox GetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_CHILDREN ) const;

    //!Like GetBbox, but it always calculcates the bounding box from scratch
    /*!
       This is usefull, when you need the bounding box, but the stored bounding box
       is not up to date. The rendering system relies on doing this update on its own,
       because it needs the old bounding box for update area calculation.

        \param nChildLevels number of child levels to include (0=no childs, 1=direct childs, ...)
    */
    a2dBoundingBox GetCalculatedBoundingBox( int nChildLevels );

    //!first translate boundingbox with cworld and recalculate at new position
    /*!
        \remark a2dDrawingPart for the a2dDrawing must be set.
        \param ic iterative context contains matrix which is applied first to get to absolute position
        \param withExtend if true boundingbox includes extend in world and pixels ( a2dDrawingPart must be set for m_root )
    */
    a2dBoundingBox GetMappedBbox( a2dIterC& ic, bool withExtend  = true );

    //!first translate boundingbox with cworld and recalculate at new position
    /*!
        \param cworld matrix applied first to get to absolute position
    */
    a2dBoundingBox GetMappedBbox( const a2dAffineMatrix& cworld );


    //!Get absolute occupied area in the device coordinates.
    /*!
        The object its boundingbox in world coordinates is first translated using the matrix cworld.
        Cworld will normally contain the accumulated matrix to the object with in the
        object Tree structure.

        \remark pixel and world extend are included.

        \param ic iterative context to get matrix which is applied first to get to absolute position
        \param inflate extra pxiels added on each size, to get rid of double to int rounding problems.
    */
    wxRect GetAbsoluteArea( a2dIterC& ic, int inflate = 2 );

    //!get world extend
    /*!
         world extend is the amount that the boundingbox is enlarged because
         of the stroke its size in case of a stroke with width defined in world coordinates.
         Other things may be included to get the total extend to the boundingbox of the object.

         The complete area occupied by a a2dCanvasObject in general is the boundingbox enlarged with
         the worldExtend + the pixelExtend ( expressed in worldcoordinates ).
         But if needed one can add to the both extends whatever is needed.

         \remark  worldExtend is calculated while Updating document/object
    */
    float GetWorldExtend() const { return m_worldExtend; }

    //!get pixel extend
    /*!
         pixel extend is the amount that the boundingbox is enlarged because
         of the stroke its size in case of a storke with width defined in pixel coordinates.
         Other things may be included to get the total extend to the boundingbox of the object.

         The complete area occupied by a a2dCanvasObject in general is the boundingbox enlarged with
         the worldExtend + the pixelExtend ( expressed in worldcoordinates ).
         But if needed one can add to the both extends whatever is needed.

         \remark  pixelExtend is calculated while Updating document/object

         \remark a trick to be able to use objects complete in pixel coordinates,
         the boundingbox can be made zero while the pixel extend account for the rest.
         a2dHandle uses this approach. It will result in a total area that is square.
    */
    int GetPixelExtend() const { return m_pixelExtend; }

    //! used for deciding if the object needs to be rendered against the current clipping area of the active drawer.
    /*!
        \param ic iterative context contains matrix which is applied first to get to absolute position
        \param clipparent clipping status of parent object ( to optimize )
    */
    OVERLAP GetClipStatus( a2dIterC& ic, OVERLAP clipparent );

    //! returns boundingbox clipping object if clipping property is set
    /*! \sa a2dClipPathProperty
        \param ic iterative context contains matrix which is applied first to get to absolute position
    */
    a2dBoundingBox GetClipBox( a2dIterC& ic );

    //\}
    //****************** END BOUNDING BOX ******************/

    //****************** EVENT PROCESSING AND HIT TESTING ******************/
    /*! \name Event processing
        Windows events are routed to the canvas objects under the mouse
    */
    //\{
public:

    //!If the position (x,y) is within the object return this
    /*!
        First a simple bounding box test is done, if that hit is positive, a recursive call on child objects
        and properties which are normally rendered is done.
        The hit test is in reverse order of the rendering order of the object and its nested child
        objects.

        When hitEvent.m_option & a2dCANOBJHITOPTION_LAYERS is true, iteration on layers is performed,
        by setting ic.SetPerLayerMode( true ).
        When hitEvent.m_xyRelToChildren is true, hit test is done relative to child objects, realized by
        adding inverse matrix of object to the a2dIterC first.
        When iterating layers, first normal objects are tested for a hit in reverse order of drawing the layers.
        So last drawn layer displayed on top, is tested first.


        For an accurate hittest one needs to implement DoIsHitWorld() for the derived a2dCanvasObject,
        in that function one should test the object accurate, and also additional object which are
        not in the childslist but real members of the derived object.
        DoIsHitWorld() should return the way the object is hit by filling a2dHitEvent::m_how with the correct info.

        \param ic contains iteration context
        \param hitEvent stores hit information

        \return  if there is a hit on this object or its children or its properties return pointer to it.
    */
    a2dCanvasObject* IsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    //!set hit flags
    /*!
        Based on these flags the object will generate a hit true.
        \param mask set hit flags in object according to given mask.
    */
    void SetHitFlags( a2dCanvasObjectHitFlags mask );

    //! return hitflags as mask
    a2dCanvasObjectFlagsMask GetHitFlags() const;

    //! Hit objects will receive the event
    /*!
        The event is first sent to the child objects, and if not processes there,
        testing for a hit on the object itself is done, and if true a2dEvtHandler::ProcessEvent is called.
        The function goes through the whole hierarchy, even if event is already processed.
        It maybe be that other events are generated while iterating over the document.

        \param ic iteration context
        \param hitEvent stores hit information

        \return true if Object (or a child ) did process the event and did not call event.Skip()
    */
    virtual bool ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent );

    //! default handler for mouse events, sent to the object from the a2dDrawingPart.
    /*!
        Mouse events are sent to the object when the mouse pointer is hiting the object.
        The default is used when the object is in edit mode. Else it will detect
        if the special object tip property named __OBJECTTIP__ is available, and switch
        it on or off when the mouse eneter or leaves the object.
    */
    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    //! default handler for character events
    void OnChar( wxKeyEvent& event );

    //! called on Right Down by default.
    void OnPopUpEvent( a2dCanvasObjectMouseEvent& event );

    //! called when the mouse enters the object
    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    //! called when the mouse leaves the object
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    //! object with mouse in flag set, will sent a leave event, and set flag off.
    //! recursive for children.
    void LeaveInObjects( a2dIterC& ic, a2dHitEvent& hitEvent );

    //! called if a mouse event occured on a child object, that is a handle
    void OnHandleEvent( a2dHandleMouseEvent& event );

protected:

    // hit on derived object
    bool LocalHit( a2dIterC& ic, a2dHitEvent& hitEvent );

    //!This is an internal function used by IsHitWorld(). Don't use it directly.
    a2dCanvasObject* IsHitWorldChildObjects( a2dIterC& ic, RenderChild& whichchilds, a2dHitEvent& hitEvent );

    //!Does hit test on the object (exclusif child objects)
    /*!
        DoIsHitWorld() should return the way the object is hit by filling a2dHitEvent::m_how with the correct info.

        \param ic iterative context ( e.g. current transform WITH the local transform applied )
        \param hitEvent stores hit information
        \return true if hit
    */
    virtual bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    //!This is an internal function used by IsHitWorldChildObjects(). Don't use it directly.
    a2dCanvasObject* HitChildObjectsOneLayer( a2dIterC& ic, RenderChild& whichchilds, a2dHitEvent& hitEvent );

    //\}
    //****************** END EVENT PROECSSING AND HIT TESTING ******************/

    //****************** CHILD TREE AND DOCUMENT STRUCTURE ******************/
    /*! \name Child tree and document structure
        Every a2dCanvasObject has a list of child objects. One a2dCanvasObject can
        be element of multiple child lists and can thus have multiple parents.
        Reference counting is used to control the life time of a a2dCanvasObject.
        Because a a2dCanvasObject can have multiple parents, it has no parent pointer.
        A a2dCanvasObject has a pointer to the a2dDrawing it belongs to.
    */
    //\{
public:

    //!get a2dDrawing of the object.
    /*!
        Root (a2dDrawing) needs to be known to each object for the following reasons:
         - It is used to inform the document that there are pending objects in the document.
         - To reach the layer setup of the document.
         - To reach the command processor of the document, which is used to submit commands to,
           that can be undone if needed.
    */
    inline a2dDrawing* GetRoot() const { return m_root; }

    //! Sets this object to a a2dDrawing.
    /*!
         \param root set the a2dDrawing for this object to this (if appropriate recursive)
         \param recurse default true, which uses a2dWalker_SetCanvasDocument for doing the
                same recursive for nested object and nested object in derived classes.
    */
    void SetRoot( a2dDrawing* root, bool recurse = true );

    //!prepend a a2dCanvasObject to the childobjects
    void Prepend( a2dCanvasObject* obj );

    //!append a a2dCanvasObject to the childobjects
    void Append( a2dCanvasObject* obj );

    //!insert a a2dCanvasObject to the childobjects
    void Insert( size_t before, a2dCanvasObject* obj, bool ignoreReleased = true );

    //!get the list where the child objects are stored in.
    /*!
         \return A pointer to the childlist. If there is no child list yet, it returns wxNullCanvasObjectList.

         \remark  wxNullCanvasObjectList has no children and therefore one can use
         GetChildObjectList()->GetCount() to test for children.
    */
    a2dCanvasObjectList* GetChildObjectList();
    const a2dCanvasObjectList* GetChildObjectList() const;

    //!create and get the list where the child objects are stored in.
    /*!
        If there is no childlist yet (wxNullCanvasObjectList), a new childlist will be created.
        Else the existing one will be returned.

         \return A pointer to the childlist.
    */
    a2dCanvasObjectList* CreateChildObjectList();

    //! get number of child objects
    /*!
        \return number of child objects
    */
    unsigned int GetChildObjectsCount() const;

    //!object with the same given mask are made into a group.
    /*!
        \param mask: mask for objects to assemble for creating a group of child objects
        \param createref: next to a new a2dCanvasObject, create a reference to the new object.
        \return return newly created object only if objects where found else NULL
    */
    a2dCanvasObject* CreateHierarchy( a2dCanvasObjectFlagsMask mask, bool createref = true );

    //! move childs of childs and members which have hierarchy one level up to this object its child list.
    /*!
        The base implementation takes child object in m_childobjects to the parent object.
        The child object are transformed by the parent its transform.
    */
    virtual void RemoveHierarchy();

    //!remove the given object from the childobjects
    /*! The object will be Realeased.
        if its refcount is 0 it will be deleted else its refcount decremented.

        \param obj object to release
        \param backwards start at the end
        \param all if true remove all references to object
        \param now if true remove all references to object now!, else only delete flag is set,
        and Update() takes care of it.
        \param undoCommands when object is connected to othere, use commands do be able to undo.

        \return number of released objects
    */
    int ReleaseChild( a2dCanvasObject* obj, bool backwards = false, bool all = false, bool now = false, bool undoCommands = false );


    //!removes and release only from the childobjects the objects with the given mask
    /*!
        \return true if some object were released
    */
    bool ReleaseChildObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //!returns index of the given a2dCanvasObject in this childobjects
    int IndexOf( a2dCanvasObject* obj ) const;

    //!return the object which fits the filter.
    /*!
        \param objectname object with this name to search for
        \param classname If classname is empty it collects all objects else only object with this class name.
        \param mask object must have this mask.
        \param propid if a property id is given, the object must have a property with this id
        \param valueAsString StringValueRepresentation of the property that is required (if not empty).
        \param id GetId() should be this unless 0
    */
    a2dCanvasObject* Find( const wxString& objectname, const wxString& classname = wxT( "" ),
                           a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL , const a2dPropertyId* propid = NULL, const wxString& valueAsString = wxT( "" ),
                           wxUint32 id = 0 ) const;

    //!return the child object if it is part of this object
    /*!
        \param obj object to search for
    */
    a2dCanvasObject* Find( a2dCanvasObject* obj ) const;

    //! the object with the given name is released and switched in place to newobject
    /*!
        \return true if object was found else false
    */
    bool SwitchChildNamed( const wxString& objectname, a2dCanvasObject* newobject );

    //!Copy objects with the right conditions to the total list.
    /*!
        \param total list of object found (may already contain elements found in earlier call)
        \param classname If classname is empty it collects all objects else only object with this class name.
        \param mask object must have this mask.
        \param id  If property id is set the object needs to have this property.
        \param bbox only find objects within this box.

        \remark only searches this object and its children, not deeper.

        \return number of objects found
    */
    int CollectObjects( a2dCanvasObjectList* total,
                        const wxString& classname = wxT( "" ),
                        a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL,
                        const a2dPropertyId* id = 0,
                        const a2dBoundingBox& bbox = wxNonValidBbox
                      ) const;

    //\}
    //****************** END CHILD TREE AND DOCUMENT STRUCTURE ******************/

    //****************** UPDATE AND PENDING ******************/
    /*! \name Update and Pending
    */
    //\{
public:
    //! Various mode flags for Update
    /*! It doesn't make sense to bit-or these, but these are bitflags anyway
        for quick multi-testing (e.g. ( mode & (mode1|mode2|mode3) ) */
    enum UpdateMode
    {
        /*! saves old valid boundingbox as boundingbox property */
        update_save = 0x0001,

        /*! includes boundingbox properties in object its boundingbox */
        update_includebboxprop = 0x0002,

        updatemask_countObjects = 0x0004,

        /*!  update_save and update_includebboxprop combined */
        updatemask_save_includebboxprop =
            update_includebboxprop |
            update_save,

        //! All updates of these modes force an update (e.g. update non-pending valid bounding boxes)
        updatemask_force =
            update_includebboxprop |
            update_save,

        //! object with invalid boundingbox or pending objects their boundingbix is recalculated.
        //! When boundingbox properties are found, theye will be include in the box.
        updatemask_normal =
            update_includebboxprop,

        updatemask_force_and_count =
            update_includebboxprop | 
            updatemask_countObjects |
            update_save,

    };

	//! child objects which have relesed flag set, are removed as child, called recursive on rleased child objects.
	bool RemoveReleased();

    //! Update the state of the object according to its current position etc.
    /*!
        This usually does the following:
          - calculates the new bounding box in world coordinates relative to parents.
          - Release childs that have m_release set

        This function is usually called in a2dDrawing::AddPendingUpdatesOldNew between adding
        the old and the new bounding box to the pending area.
        You should NOT do any change here that issues commands to the command processor,
        because such commands wouldn't have a proper context. Such things should be
        done in UpdateImmediate.
        Other things like updating the cache data need to be done here.

        Takes into account associated child, properties and derived objects.
        If a2dCanvasObject::UpdateMode = updatemask_force all boundingboxes are invalidated and recalculated,
        else it only recalculates if invalid or one of the children is invalid.

        This function calls DoUpdate(), when needed. The idea is to minimize the number of recalculated boudingboxes.
        So if an object is setpending or a child became pending, the boudingbox needs to be recalculated,
        but since this is a combination of property, child and derived object boundingboxes, this is optimized.

        \return true if the bounding box did change and the parent has to recalculate


        \remark The object its layers is set in the layersetup as being available in the document.
        \remark if overridden in derived class, also call baseclass Update
        \remark in derived classes DoUpdate() should be implemented to at least calculate the boundingbox of the object
        without children in m_childobjects, but including any referenced objects in the derived objects. Like arrow point
        objects on a line or polyline.
    */
    virtual bool Update( UpdateMode mode );

    //!is this object pending for update?
    /*!(overruled by parent object in some cases during rendering)
     \remark set the object its pending flag and inform root about this
    */
    bool GetPending() const { return m_flags.m_pending; }

    //!set this object pending for update
    /*!
     This flag is set when an object has changed, which means that it needs to be redrawn.
     Since an object can be Referenced also, it can be drawn at several positions.
     Therefore the redrawing needs to be done indirectly from the top, and for each a2dDrawingPart
     displaying the object. In general old and new boundingbox areas are updated on the display.
     \li Step 1 add pending objects occupied areas to drawer.
     \li Step 2 The new boundingbox is calculated.
     \li Step 3 add pending objects occupied areas to draweronce more.
     \li Step 4 reset all pending flags for objects.

     \remark Does also inform the a2dDrawing that there are pending objects.
     \remark If because of this object becoming pending other objects should become pending also,
     one should override this function to set those other objects pending.
     \remark If redefined in derived object always call base also.

     \sa DependencyPending()

     \param pending if true set object pending

    */
    virtual void SetPending( bool pending );

    //! extra flag set when object is pending because of change in select status
    bool GetPendingSelect() const { return m_flags.m_selectPending; }

    //! extra flag set when object is pending because of change in select status
    void SetPendingSelect( bool pending );

    //! Is object pending for whatever reason (selection or general pending flag).
    bool IsPending() const { return m_flags.m_pending || m_flags.m_selectPending; }

    //!search objects ( if nested recursive ) that have the pending flag Set
    /*!
        And add the current absolute boudingbox of the object to the pendingareas in the active
        a2dDrawingPart for update/redrawing that area later. After a this normally the boundingbox of
        this object will be recalculated in a2dCanvasObject::Update(), followed by one more call to this function
        here, in order to also redraw the new boundingbox its area.
        When all pending areas are combined to a minimum in a2dDrawingPart, the areas will be redrawn.

        The area currently occupied by the object in the active a2dDrawingPart is added for update in case
        the object was pending. Next to that the  a2dDrawingPart is informed that the object its layer
        is available in the document and should be rendered.
        In case the object will be released ( m_Release is set ), the a2dDrawingPart::SetLayerCheck( objectlayer )
        is called, in order to recheck if there or still object on that layer as seen from that view.

        \remark this method is fast if many objects need an update.
        \remark You are responsible for setting and resetting the pending flag.
        \remark Not valid boundingbox in the object, means the object and its children will be ignored.

        \param ic iterative context, contains cumulative matrix for transforming object to absolute position
    */
    void AddPending( a2dIterC& ic );


    //!called by  to check if this object becomes pending as a result of other objects
    /*!
        You can Override DependencyPending() to set your object pending, when another object was
        set pending.
    */
    virtual void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    //! call this when a a2dCommands should result in extra commands, to become part of a parent a2dCommandGroup.
    /*!
        In some cases a command (e.g. delete an object), needs to be followed by extra commands. For example to disconnect other objects.

        \param parent the parent of this object instance.
        \param command the command for which the dependcies need to be found.
    */
    virtual void DependentCommands( a2dCanvasObject* parent, a2dCommand* command );

    //****************** END UPDATE AND PENDING ******************/

    //****************** RENDERING AND STYLE ******************/
    /*! \name Rendering and style
    */
    //\{
public:

    //!Render this object to the active a2dDrawingPart
    /*!

    This function is called via the a2dDrawing class or its nested children.
    The active a2dDrawingPart which is set for the document is used to draw
    to (either on its buffer or directly on the device)
    All rendering goes through this function to be able to render specific things
    in the base class first (like a2dCanvasObject its properties.)
    Somewhere internal DoRender will be called to really render the derived object itself.
    Next to that nested a2dCanvasObject's will be rendered too.

    If wanted only the canvas objects with a certain mask set, will be rendered.
    Mask example: Checking visible and selected

    use: mask=(a2dCANOBJ_VISIBLE | a2dCANOBJ_SELECTED)

    \remark
    Do not use directly from outside the containing document of this object.
    Normally only used directly in derived classes \sa a2dDrawing

    \remark The active clipping rectangle is set within the active drawer.
    It can be used to decide which parts of the object need to be re-drawn or not.
    This may speed up drawing in sepcial cases, but in general the active a2dDrawingPart, simply
    clips all that is drawn on it to active clipping rectangle.
    The fact that the object is called to redrawn itself, is not decided here, instead it is part
    of the pending object mechanism. Which uses the object its boundingbox to request redrawing.

    \remark Normally an object will be rendered if the given layer is equal to the object its layer.

    \remark wxLAYER_ALL has a special meaning.
    If the given input layer is wxLAYER_ALL, then the m_layer of objects is not tested.
    So the child objects their own layer settings are ignored/not checked in that case.
    All child objects are drawn at once, still the style of the layers settings is used when needed.

    If the flag "m_flags.m_childrenOnSameLayer" is set, a new iteration over layers will take place.
    The effect is that all children will be drawn at once ( not only objects on the given layer ),
    but the order of the layers will be taken into account to draw the children.
    If the flag "m_flags.m_childrenOnSameLayer" is NOT set, a new iteration on layers is NOT done,
    and only the objects on the given layer will be drawn.

    \remark  ic.GetPerLayerMode() is true, then only ic.GetLayer() is rendered, which  can not be wxLAYER_ALL.

    \param ic iteration context (has a2dDrawingPart accumulative matrix to calculate absolute position of the object)
    \param clipparent this must be the clip status of parent object, it is used to optimize drawing speed.
           e.g If parent is completely within the current clipping rectangle of the a2dDrawingPart,
           there is no need to check child objects.
    */
    virtual void Render( a2dIterC& ic, OVERLAP clipparent );

    //! update the transform matrix for objects with property 'PROPID_viewDependent'
    /*!
        will recursive call the UpdateViewDependentObjects routines of all the child objects (TODO for optimize: with flag 'm_childpixelsize=true').
        \param ic iteration context (has a2dDrawingPart accumulative matrix to calculate absolute position of the object)

        \remark Check all child objects with (TODO for optimize: flag 'm_childpixelsize=true').
    */
    void UpdateViewDependentObjects( a2dIterC& ic );

    //! called by Render() if m_flags.m_HighLight is set
    /*!
        \param ic iteration context (has a2dDrawingPart accumulative matrix to calculate absolute position of the object)
    */
    virtual void DrawHighLighted( a2dIterC& ic );

    //!set if this object will visible (be rendered or not)
    /*!(overruled by parent object in some cases during rendering)
        \remark set the object its pending flag and inform root about this
        \param visible if true set object visible
    */
    void SetVisible( bool visible )
    {
        // BR2019 if( m_flags.m_visible != visible ) { m_flags.m_visible = visible; SetPendingSelect( true ); }
        if( m_flags.m_visible != visible ) { m_flags.m_visible = visible; SetPending( true ); }
    }

    //!get visibility (rendering depends on layer settings also)
    bool GetVisible() const { return m_flags.m_visible; }

    //!get visibility (rendering depends on layer settings also)
    /*!
        Alias for GetVisible
         \see GetVisible
    */
    inline bool IsVisible() const { return GetVisible(); }

    //!Set a fill for the object which will be used instead of the layer fill
    /*!
    The fill is for filling the object.
    Use a2dCanvasNullFill or 0 to remove the fill from a object.
    Use a2dTRANSPARENT_FILL to not fill the object.
    \remark m_flags.m_filled flag overrules the fill to fill TRANSPARENT
    \remark inheritance to children of object
    \remark object is free to use style or not
    \remark a style resulting in wxLAYER_FILL means property will be removed
    \sa a2dStyleProperty
    */
    void SetFill( const a2dFill& fill );

    a2dFill GetFill() const;

    //!Set a fill color for the object which will be used instead of the layer fill
    /*!
    \param fillcolor color to fill object with
    \param style style for one colour fill
    \remark creates a a2dOneColourFill internal
    \remark inheritance to children of object
    \remark object is free to use style or not
    \sa a2dStyleProperty
    \sa a2dOneColourFill
    */
    void SetFill( const wxColour& fillcolor, a2dFillStyle style = a2dFILL_SOLID );

    //!Set a fill color for the object which will be used instead of the layer fill
    /*!
    \param fillcolor color to fill object with
    \param fillcolor2 color to fill object with
    \param style style for one colour fill
    \remark inheritance to children of object
    \remark object is free to use style or not
    \sa a2dStyleProperty
    \sa a2dTwoColourFill
    */
    void SetFill(  const wxColour& fillcolor, const wxColour& fillcolor2, a2dFillStyle style = a2dFILL_SOLID );

    //! set first colour of fill
    void SetFillColour( const wxColour& colour );

    //! get first colour of fill
    wxColour GetFillColour() const;

    //!Set a stroke  for the object which will be used instead of the layer stroke
    /*!
    \param strokecolor color to stroke object with
    \param width width of stroke in world coordinates
    \param style style for one colour stroke
    \remark a style resulting in wxLAYER_STROKE means property will be removed
    \remark inheritance to children of object
    \remark object is free to use style or not
    \sa a2dStyleProperty
    */
    void SetStroke(  const wxColour& strokecolor, double width = 0,  a2dStrokeStyle style = a2dSTROKE_SOLID );

    //!Set a stroke  for the object which will be used instead of the layer stroke
    /*!
    \param strokecolor color to stroke object with
    \param width width of stroke in device coordinates
    \param style style for one colour stroke
    \remark creates a a2dOneColourStroke property internal
    \remark inheritance to children of object
    \remark object is free to use style or not
    \sa a2dStyleProperty
    */
    void SetStroke(  const wxColour& strokecolor, int width ,  a2dStrokeStyle style = a2dSTROKE_SOLID );

    //! Set stroke using pointer to a stroke
    /*!
    The stroke is for drawing outlines of the object.
    Use a2dNullStroke to remove the stroke  from a object.
    Use a2dTRANSPARENT_STROKE to not fill the object.
    \remark a style resulting in wxLAYER_STROKE means property will be removed
    */
    void SetStroke( const a2dStroke& stroke );

    a2dStroke GetStroke() const;

    //! set first colour of stroke
    void SetStrokeColour( const wxColour& colour );

    //! get first colour of stroke
    wxColour GetStrokeColour() const;

    //!set the Contour width of the shape
    /*!
        Next to the stroke width one can sometimes set a contour width e.g. a circular donut.
    */
    virtual void SetContourWidth( double WXUNUSED( width ) ) {}

    //!get the Contour width of the shape
    virtual double GetContourWidth() const { return 0; }

    //!sets fill and stroke of object to a2dDrawingPart
    /*!
        sets local fill and pen into a2dDrawingPart if available, else
        they will be set on basis of the layer index.
        \sa a2dDrawingPart
    */
    void SetDrawerStyle( a2dIterC& ic, a2dStyleProperty* style );

    //!if set children are rendered on the same layer as this object.
    /*!
        Children will be rendered at the same moment as the object itself, the effect
        is that they appear at the same layer.
        The rendering style of the children will be based on their own layer id's or style properties.

        \param samelayer if true render children on same layer
    */
    inline void SetChildrenOnSameLayer( bool samelayer ) { SetPending( true ); m_flags.m_childrenOnSameLayer = samelayer; }

    //!are children rendered on the same layer as this object?
    inline bool GetChildrenOnSameLayer() const { return m_flags.m_childrenOnSameLayer; }

    //!set the object view dependent and maybe process all children to set these flags
    /*!
        \param aView view for adding next properties on this object and maybe its children.
        \param viewdependent if true, then add property 'PROPID_viewDependent' to objects
                             that are depending on 'aView' view when it comes to size.
                             Else to remove this property.
        \param viewspecific  if true then add property 'PROPID_viewSpecific' to objects
                             that are only visible on 'aView' view.
                             Else to remove this property to make visible on all views.
        \param onlyinternalarea removing scale for children
        \param deep if true then set flags for all nested children objects
    */
    void SetViewDependent(a2dDrawingPart* aView, bool viewdependent, bool viewspecific = false, bool onlyinternalarea = false, bool deep = false );

    //! How a child is placed towards its parent object.
    /*!
        If set true this object as a child object is placed only relative to the position of the
        parent object. Rotation and scaling of parent object are ignored.
    */
    inline void SetChildOnlyTranslate( bool onlytranslate ) { SetPending( true ); m_flags.m_ChildOnlyTranslate = onlytranslate; }

    //!are children rendered using just the translation of the parent or also rotation and scale.
    inline bool GetChildOnlyTranslate() const { return m_flags.m_ChildOnlyTranslate; }

    //! If set, this object has a higher priority in rendering than other children objects.
    /*!
        This method sets a priority in rendering: All children which have set
        the property m_prerenderaschild will be rendered before other children objects.

        As default the a2dCanvasObject has no priority in rendering, you have to
        set this property to true, if you want prerendering.

        \param prerender <code>true</code> to prerender this object, else <code>false</code>

    */
    inline void SetPreRenderAsChild( bool prerender ) { SetPending( true ); m_flags.m_prerenderaschild = prerender; }

    //! Returns if this object should be rendered before other children objects.
    inline bool GetPreRenderAsChild() const { return m_flags.m_prerenderaschild; }

    //!is the object filled flag set?
    //!Rendering is only outline when not set.
    bool GetFilled() const { return m_flags.m_filled; }

    //!set the filled flag to render only outlines
    /*!Rendering is only outline when not set.
     \remark set the object its pending flag and inform root about this
     \param filled if true set object filled
    */
    void  SetFilled( bool filled ) { if ( m_flags.m_filled != filled ) SetPending( true ); m_flags.m_filled = filled; }

    //! If True shadow object is visible (if property is there)
    bool GetShowShadow() const { return m_flags.m_showshadow; }

    //! If True shadow object is visible (if property is there)
    void SetShowShadow( bool showshadow ) { m_flags.m_showshadow = showshadow; }

    //! see SetResizeOnChildBox()
    bool GetResizeOnChildBox() const { return m_flags.m_resizeToChilds; }

    //! If True resize to child boundingbox
    /*!
        In a2dCanvasObject::Update() a wxEVT_CANVASOBJECT_RESIZE_EVENT will be sent to this object.
        Default there is no handler for the event.
    */
    void SetResizeOnChildBox( bool resizeToChilds ) { m_flags.m_resizeToChilds = resizeToChilds; }

    //!quick way to get the style property containing Shadow property
    /*! style is maintained within an object as a a2dStyleProperty called __SHADOW__
    \remark object is free to use style or not
    \sa a2dShadowStyleProperty
    */
    const a2dShadowStyleProperty* GetShadowStyle() const ;

    void CallDoRender( a2dIterC& ic, OVERLAP clipparent )
    {
        DoRender( ic, clipparent );
    }

    //!when wanting to traverse hierarchy what level is down here.
    virtual a2dCanvasObject* PushInto( a2dCanvasObject* parent );

    virtual a2dCanvasObject* GetCommandProcessor() { return NULL; }
    virtual a2dLayers* GetLayerSetup() { return NULL; }

protected:

    const a2dFill& MX_GetFill() const;
    void MX_SetFill( const a2dFill& value );
    const a2dStroke& MX_GetStroke() const;
    void MX_SetStroke( const a2dStroke& value );


    //!render derived object
    /*!if the object has sub objects (apart from the childobject which are handled here),
    those subobject most rendered by iterating on layer when needed/wanted, simular to child objects.
    We do not iterate here, since that is only needed if indeed there or subobjects.
    This will be know in a "wxDerivedCanvasObject DoRender".

    SO parent objects that call this function, must:

        \li 1-  clip object against area to redraw.
        \li 2-  iterate on layers when needed.

    A a2dCanvasObject is rendered as a + (plus sign) when there or no children.

    */
    virtual void DoRender( a2dIterC& WXUNUSED( ic ), OVERLAP WXUNUSED( clipparent ) );

    //! update transform matrix   klion: I'm not sure that this function necessary as virtual
    virtual void DoUpdateViewDependentTransform( a2dIterC& ic );

    //! update derived objects
    virtual void DoUpdateViewDependentObjects( a2dIterC& WXUNUSED( ic ) );

    //! render only the child objects
    /*!
    \param ic iteration context (has a2dDrawingPart accumulative matrix to calculate absolute position of the object)
    \param whichchilds defined which childs will be rendered now, and detects while rendering other types.
    \param clipparent this must be the clip status of parent object.
    */
    virtual void RenderChildObjects( a2dIterC& ic, RenderChild& whichchilds, OVERLAP clipparent );

    //!to render the child objects
    /*!
    \param ic iteration context (hasa2dDrawingPart accumulative matrix to calculate absolute position of the object)
    \param whichchilds defined which childs will be rendered now, and detects while rendering other types.
    \param clipparent this must be the clip status of parent object.
    */
    virtual void RenderChildObjectsOneLayer( a2dIterC& ic, RenderChild& whichchilds, OVERLAP clipparent );

    //\}
    //****************** END RENDERING AND STYLE ******************/

    //****************** EDITING AND HIGHLIGHTING ******************/
    /*! \name Editing and Highlighting
    */
    //\{
public:

    //!Sets if this object may be edited
    /*!
        If editable is set to <code>true</code> the object may be edited by an user.
        The default behaviour of this object is to allow modifications (editable).

        \param editable <code>true</code> to allow modifications, else <code>false</code>
    */
    inline void SetEditable( bool editable ) { m_flags.m_editable = editable; }

    //!get if the object may be edited
    inline bool GetEditable() const { return m_flags.m_editable; }

    //! Set object to fixed style ot not.
    inline void SetFixedStyle( bool fixedStyle ) { m_flags.m_fixedStyle = fixedStyle; }

    //! Get object fixed style setting
    inline bool GetFixedStyle() const { return m_flags.m_fixedStyle; }

    //!get if the object may be edited
    /*!
        Alias for GetEditable
         \see GetEditable
    */
    inline bool IsEditable()  const { return GetEditable(); }

    //!is the object selected flag set
    /*!
        Rendering is different when set, it will use a special layer in this case.
    */
    bool GetSelected() const { return m_flags.m_selected;}

    //!Is the object selected flag set
    /*!
        Alias for GetSelected
         \see GetSelected
    */
    bool IsSelected() const { return GetSelected(); }

    //!Set the object selected flag if allowed
    /*!
        When set the second rendering cycle from the document a2dDrawing::Render() etc.
        will draw this object selected.

         \remark sets the object its pending flag and inform root about this
         \param selected if true set object selected
    */
    void  SetSelected( bool selected )
    {
        if ( m_flags.m_selectable ) 
        { 
            if( m_flags.m_selected != selected ) 
            {
                SetPendingSelect( true );
            }
            m_flags.m_selected = selected;
        }
    }

    //!is the object selected2 flag set
    /*!
        Rendering is different when set, it will use a special layer in this case.
    */
    bool GetSelected2() const { return m_flags.m_selected2; }

    //!Set the object selected2 flag if allowed
    /*!
        When set the second rendering cycle from the document a2dDrawing::Render() etc.
        will draw this object selected with select2 color.

         \remark sets the object its pending flag and inform root about this
         \param selected if true set object selected
    */
    void  SetSelected2( bool selected )
    {
        if ( m_flags.m_selectable ) 
        { 
            if( m_flags.m_selected2 != selected ) 
            {
                SetPendingSelect( true );
            }
            m_flags.m_selected2 = selected;
        }
    }

    //!is the object selectable flag set
    /*!
         \return <code>true</code> if object is selectable, else <code>false</code>
    */
    bool GetSelectable() const { return m_flags.m_selectable; }

    //! Is the object selectable flag set
    /*!
        Alias for GetSelectable
         \see GetSelectable
    */
    bool IsSelectable() const { return GetSelectable(); }

    //!Allows to select this object.
    /*!
        If selectable is set to <code>true</code> the object may be selected by an user.
        The default behaviour of this object is to allow selection.

        \param selectable <code>true</code> to allow selection, else <code>false</code>
    */
    void  SetSelectable( bool selectable ) { m_flags.m_selectable = selectable; }

    //!Sets if this object may be dragged
    /*!
        If draggable is set to <code>true</code> the object may be dragged by an user.
        The default behaviour of this object is to allow dragg-operations.

        \param draggable <code>true</code> to allow selection, else <code>false</code>
    */
    inline void SetDraggable( bool draggable ) { m_flags.m_draggable = draggable; }

    //!get if the object can be dragged
    inline bool GetDraggable() const { return m_flags.m_draggable; }

    //!get if the object can be dragged
    /*!
        Alias for GetDraggable
         \see GetDraggable
    */
    inline bool IsDraggable() const  { return GetDraggable(); }

    //!is snap flag set?
    bool GetSnap() const { return m_flags.m_snap; }

    //!Sets snap flag
    /*!
        If snap is set to <code>true</code> this object is snapped by
        the restriction engine of a2dCanvasGlobal, when it is edited/moved etc.

        \see Restrict
    */
    void SetSnap( bool snap ) { m_flags.m_snap = snap; }

    //!Sets snap_to flag
    /*!
        If snap is set to <code>true</code> others objects may snap to this object by
        the restriction engine of a2dCanvasGlobal.

        \see Restrict
    */
    void SetSnapTo( bool snap ) { m_flags.m_snap_to = snap; }

    //!is snap_to flag set?
    bool GetSnapTo() const { return m_flags.m_snap_to; }

    //! you may use it to modify rendering of the object depending on setting
    /*!
        Override this function to define a way to modify rendering for a derived object.

        e.g. When hiting an object you could modify the rendering to notify it to the user.
    */
    virtual void SetMode( int WXUNUSED( mode ) ) {}

    //! get the rendering mode of the object.
    virtual int GetMode() const { return 0; }

    //! create an editcopy and initialize editing of the editcopy
    /*! This is called for an original object and creates an editcopy object.

        Calls DoStartEdit() in the end

        \param tool tool from which this function is called. Currently this is the a2dObjectEditTool
        \param editmode tobe used in derived object to switch between different way of editing
              ( 0 means matrix mode of a2dCanvasObject )
        \param editstyle style of editing e.g. filled and stroked or with a wire frame.
    */
    virtual a2dCanvasObject* StartEdit( a2dBaseTool* tool, wxUint16 editmode, wxEditStyle editstyle = wxEDITSTYLE_COPY, a2dRefMap* refs = NULL );

    //! cleanup an editcopy object
    /*!
        This will Remove the editcopy from its parent.
        If editstyle is not wxEDITSTYLE_COPY, EditEnd is called for an original
        object, which is deprecated. The only tool using non wxEDITSTYLE_COPY is
        the wxMultEditTool, which will be changed.
        Calls DoEndEdit() first
    */
    virtual void EndEdit();

    //! to restart editing in a different mode
    void ReStartEdit( wxUint16 editmode );

    //! if this is an editcopy, return the orginal else NULL
    a2dCanvasObject* GetOriginal();

    //! set a2dHandle position with the given name
    a2dHandle* SetHandlePos( wxString name, double x, double y );

    //!redirect all mouse events for the canvas to this object
    void CaptureMouse( a2dIterC& ic );

    //!release the mouse capture for this object
    void ReleaseMouse( a2dIterC& ic );

    //!is the mouse captured for this object
    bool IsCapturedMouse( a2dIterC& ic ) const ;

    //! selected object itself or one of its recursive children
    bool GetHasSelectedObjectsBelow() const { return m_flags.m_HasSelectedObjectsBelow; }

    //! selected object itself or one of its recursive children
    void SetHasSelectedObjectsBelow( bool value ) { m_flags.m_HasSelectedObjectsBelow = value; }

    //! tool object itself or one of its recursive children
    bool GetHasToolObjectsBelow() const { return m_flags.m_HasToolObjectsBelow; }

    //! selected object itself or one of its recursive children
    void SetHasToolObjectsBelow( bool value ) { m_flags.m_HasToolObjectsBelow = value; }

    //\}
    //****************** END EDITING AND HIGHLIGHTING ******************/

    //****************** SERIALIZATION, IDs AND REFERENCES *****************/
    /*! \name Serialization, IDs and references
        In the CVG format one can store multiple references to one and the same object.
        The object that is referenced is only written once, for the rest only
        the reference id is written to the CVG file.
        When loading a file in CVG, such references are resolved inside the document.
        So in the end the old reference is restored via by searching the object having that id.
        As such the refid attribute is just a place holder for a reference to the actual referenced object.
        The reason behind all this, is that the actual object might to be read yet, when a reference
        is encountered in the CVG file, and therefore the link can not be directly created.
        In other formats references are used in a simular manner, and to resolve them the same mechanism
        is used.
    */
    //\{
public:

    //!Check if this is a temporary object, which should not be saved
    virtual bool IsTemporary_DontSave() const;

#if wxART2D_USE_CVGIO

    //!write object specific CVGL data
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    //!load object specific CVG data
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

#endif //wxART2D_USE_CVGIO


    //\}
    //**************** END SERIALIZATION, IDs AND REFERENCES ***************/

    //********************** CONNECTION PINS AND WIRES *********************/
    /*! \name Connection via Pins and Wires

    - a a2dCanvasObject can have a2dPin objects as childs.
      These objects can connect to other a2dPin's and thus to other objects.

    - a2dPin objects know their parent, so that a communication path
      a2dCanvasObject (has child) a2dPin (is connected to) a2dPin (has parent) a2dCanvasObject
      exists.

    - There are wire objects, that connect pins in objects and modify themselfes if the
      objects they connect move. A wire object overloads "IsConnect" to return true.

    - Note: There are two quite different wire classes: a2dWires and a2dWirePolylineL.
      a2dWires is more like a least-distance "airline" network used e.g. in routing applications to
      show unconnected nets.
      a2dWirePolylineL is a usual polyline with wire functioanlity.
      Some of the functions apply to one type, one to the other type.
    */
    //\{
public:

    //! return true, if this object is used to connect other object's using rubberband like structures.
    virtual bool IsConnect() const { return false; }

    //! object is a virtual connection (connects invisible with other objects without using wires )
    virtual bool IsVirtConnect() const { return false; }

    //!used in case of flexible canvas objects (wires).
    /*!
        If another object connected to this object changes, and that has an effect on this
        object, return true;
    */
    virtual bool NeedsUpdateWhenConnected() const { return false; }

    //! if return true, connection with other object on this object its pins is allowed.
    /*! This is usually used for temporarily disabling connectivity */
    bool DoConnect() { return m_flags.m_doConnect; }

    //! If set to true this object may be connected to other object on its pins.
    /*! This is usually used for temporarily disabling connectivity */
    void DoConnect( bool doconnect ) { m_flags.m_doConnect = doconnect; }

    //! check connect to other object
    /*! After the call Pin objects which can connect, have their bin flag set.
    */
    bool CanConnectWith( a2dIterC& ic, a2dCanvasObject* toConnect, bool autocreate );

    //! is the given pin close enough to another a2dPin to allow connection?
    /*!
        The default searches for a2dPin children in this object, and a pointer to the one which
        can connect with the given pin is returned. If non, returns NULL.

        Before trying to connect to the object DoCanConnectWith() is called, here the object can be prepared
        to be able to connect to the given pin here. The default implementation asks the object to GeneratePins()
        if autoccreate is set true.

        \param ic iteration context
        \param pin pin to check for connection to this object
        \param margin pin as to be this close.
        \param autocreate when true, pins maybe be created at the position in a derived object.

    */
    a2dPin* CanConnectWith( a2dIterC& ic, a2dPin* pin, double margin, bool autocreate );

    //! Is the object connected ( Optinal to specific object ) ?
    /*!
        \param needsupdate if true, only return true when connected Object(s) need an update.
        \param toConnect when not ( NULL ), the object needs to be connected with toConnect.
    */
    bool IsConnected( bool needsupdate, a2dCanvasObject* toConnect = 0 );

    //! Find pins in other object which are connected to this object.
    /*!
        If this object is connected on pins to other objects, those pins in the other objects
        will be put into the result list.
        The search uses the bin flag on pins to detect if this pin was already checked.
        Therefore you should clear that flag on pins of all object at the same level as this.
        Meaning the parent object of this object here. For the root object of a document, that can be done like:
        <code>
        a2dCanvasObject* root = doc->GetRootObject();
        a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
        setflags.Start( root, false );

        a2dCanvasObject* checkthis = root->GetChildObjectList()->Find( wxT("MyNameObject") );
        a2dCanvasObjectList result;
        wxString names;
        if ( checkthis )
            checkthis->FindConnectedPins( result, NULL, true, NULL );
        </code>

        If you want to skip certain Pins you can set the bin flag in advance. 
        Pins on objects which are of connect type, like wires, will be skipped if walkWires is true.
        In that case the reported pins will be pins on normal objects connected to this, also via wires.

        \param result all pins found
        \param pin If not NULL find only connections on this pin
        \param walkWires If true, all wires like object are recursively iterated for pins to other non wire objects.
                The wire pins are skipped from the result.
        \param searchPinClass if set search pin with this pinclass 
        \param isConnectedTo if not NULL, only pins connected to this object will be searched.
    */
    virtual bool FindConnectedPins( a2dCanvasObjectList& result, a2dPin* pin = NULL, bool walkWires = true, a2dPinClass* searchPinClass = NULL, a2dCanvasObject* isConnectedTo = NULL );

    //! find wires on pins of this objects.
    /*!
        A depth first search for correct branches. Wire passed and fitting condition, are stored to result.

        \param result all wires found
        \param pin If not NULL find only wires on this pin
        \param walkWires If true, all wires like object are recursively iterated for pins to other wire objects.
        \param selectedEnds if true, end of wire to a non wire object must have selected flag set.
        \param addToResult if true, this object is added to result. (default false, but recursive call set true).
    */
    bool FindConnectedWires( a2dCanvasObjectList& result, a2dPin* pin, bool walkWires = true, bool selectedEnds = false, bool stopAtSelectedWire = false, bool addToResult = false );


    //! get connected objects that are connected to this object via its pins.
    /*!
         \param connected list to which connected objects will be added.
         \param needsupdate if true only connected objects which need updating will be added.
    */
    bool GetConnected( a2dCanvasObjectList* connected, bool needsupdate );

    //!Do connect with another a2dCanvasObject by pinname
    /*!
        If pinname is an empty string the pins that lie on top of each-other will be connected.
        Zero Length wires or objects (having pins on top of eachother) are not connected if already a connection exists.
        If needed (eg pin was already connected to another object) extra wires will be added to keep connection intact.
    */
    virtual bool ConnectWith( a2dCanvasObject* parent, a2dCanvasObject* toconnect, const wxString& pinname = wxT( "" ), double margin = 1, bool undo = false );

    //!Do connect with another a2dCanvasObject by pin pointer
    /*!
        Searches this object for a pin, which is at the same position as the one given.
        If that pin allows connection, those pins will be connected.
    */
    virtual bool ConnectWith( a2dCanvasObject* parent, a2dPin* pin, double margin = 1, bool undo = false );

    //! connect two pins
    /*! by creating a a2dWires object in between, unless already connected and
        they are exactly at the same position.
        They idea is to connect pins without changing there position, and keep already connected
        objects at the given pins connected also (to the wire object that is created ).
    */
    virtual void ConnectPinsCreateConnect( a2dCanvasObject* parent, a2dPin* pinc, a2dPin* pinother, bool undo = false );

    //! pins one wires to the same kind of wire are removed.
    bool CleanWires( a2dCanvasObjectFlagsMask mask  = a2dCanvasOFlags::ALL );

    //! connect two pins which must be unconnected and at the same position
    /*!
        This is a simple straight forward connection of the two pins.
        When Undo is true proper commands are issued to connect the pins.
        If the pins are already connected with something, they will be first disconnected.
    */
    void ConnectPins( a2dCanvasObject* parent, a2dPin* pinc, a2dPin* pinother, bool undo = false );

    //!Do Disconnect from another a2dCanvasObject by pin name
    /*!
        \param toDisConnect object to disconnect, if NULL all objects will be disconnected. 
        \param pinname If pinname is an empty string the pins connected to toConnect will be dis-connected,
        else only the ones with the pinname will be disconnected.
        \param undo if true commands will be used to disconnect pins.
    */
    virtual bool DisConnectWith( a2dCanvasObject* toDisConnect = NULL, const wxString& pinname = wxT( "" ), bool undo = false );

    //!Do Disconnect from another a2dCanvasObject by pin pointer
    /*!
        Disconnect at pin (of this object ).
    */
    virtual bool DisConnectAt( a2dPin* pin, bool undo = false );

    //! rewire the object to keep connection or to make connection with other objects
    /*!
        \param parent parent object to get and release to connected objects.
        \param undo if true commands will be used to disconnect pins.
    */
    void ReWireConnected( a2dCanvasObject* parent, bool undo = false );

    //! create wires on pins which do not have wires, but directly are connected to other objects.
    /*!
        This prepares the object for dragging/moving, while preserving the connection, since then wires will
        be rerouted when dragging.
    */
    bool CreateWiresOnPins( a2dCanvasObject* parent, bool undo, bool onlyNonSelected = false );

    //! set connected pending or not pending
    /*!
         \param onoff set connected object pending or not pending
         \param needsupdateonly if true only connected object which need updating will treated.
    */
    bool SetConnectedPending( bool onoff, bool needsupdateonly );

    //!are there a2dPin derived children
    /*!
        \param realcheck if true checks all children else the flag m_flags.m_hasPins is returned
        \remark if realcheck is true also the m_flags.m_hasPins will be updated.
    */
    bool HasPins( bool realcheck = false );

    //!are there a2dPin derived children which matches the given pin name?
    /*!
        \param pinName the name of the pin to search for ( uses wxString::Matches() ).
        \param NotConnected if true the pin must be unconnected too.

        \return the a2dPin object found or NULL

        \remark the m_flags.m_hasPins is ignored here for the moment
    */
    a2dPin* HasPinNamed( const wxString pinName, bool NotConnected = false );

    //!How many a2dPin derived children are there.
    int GetPinCount();

    //! generates pins on all possible locations where the object can be connected.
    /*!
        Default calls pinClass->GetConnectionGenerator()->GeneratePossibleConnections()
        And that leads to calling a2dCanvasObject::GeneratePins()
        This way a a2dConnectionGenerator can limit the pins which are generated.

        \param pinClass if not NULL, only generate temporary pins that may connect to this pinClass.
        \param task what/how to connect
        \param x can be used to create pins depending on the poition of the mouse inside the object.
        \param y can be used to create pins depending on the poition of the mouse inside the object.
        \param margin if not 0 the genarted pins must be this close to x and y.
    */
    virtual bool GeneratePinsPossibleConnections( a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin = 0 );

    //! create pins in derived objects.
    /*!
        When wanting to connect to shapes which at construction has no pins, but still make sence
        to connect too, this function generates pins for the object, when connection is asked for by a tool.

        The idea is to generate temporary pins in objects, as feedback for the tools.
        This way the user sees where connections are possible.

        You need to implement this function per object. The default is not doing anything.

        \sa GeneratePinsPossibleConnections()

        \param toConnectTo the pinClass to which the generated pin must be able to connect.
        \param task what/how to connect
        \param x x position of mouse or pin which wants/needs connection
        \param y y position of mouse or pin which wants/needs connection
        \param margin if not 0 the genarted pins must be this close to x and y.
    */
    virtual bool GeneratePins( a2dPinClass* WXUNUSED( toConnectTo ), a2dConnectTask WXUNUSED( task ), double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED(margin) = 0 ) 
    { return false; }

    //! Allow change in pin location when wiring things up.
    virtual bool AdjustPinLocation() { return false; }

    //! based on the a2dPinClass's of eventually a2dPin's wanted in both objects, a
    /*! connection object will be delivered as a template to be cloned by the caller.
        In principle calling this->GetConnectTemplate() and other->GetConnectTemplate()
        should result in the same.
        The default implementation uses a2dCanvasGlobals->GetPinClassMaps() which is a simple system
        to supply a template object based ob the two entry maps.
        You can override this function to define your own way of connections.
    */
    virtual a2dCanvasObject* GetConnectTemplate( a2dPinClass* mapThis, a2dCanvasObject* other, a2dPinClass* mapOther ) const;

    //! generates a connection object with pins and all.
    /*!
        Default calls pinThis->GetConnectionGenerator()->CreateConnectObject()
        This way a a2dConnectionGenerator can decide on the type of object to generate
        as a plugin.
    */
    virtual a2dCanvasObject* CreateConnectObject( a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo = false ) const;

    //! add a a2dPin as child
    /*!
        \param name name which the pin will get
        \param x x-position of pin
        \param y y-position of pin
        \param a2dpinFlags
                - a2dPin::dynamic if true pin will be dynamic
                - a2dPin::temporary if true pin will be temporary
                - a2dPin::objectPin if true pin is seen as part of an object and not a wire/connect
        \param pinClass pinClass of the pin, created pin cloned from a2dPinClass->GetPin()
        \param undo if set undo-able commands will be used

        \return the pin that was created and added to the object.
    */
    a2dPin* AddPin( const wxString name, double x, double y, wxUint32 a2dpinFlags, a2dPinClass* pinClass, bool undo = false );

    //! Remove all a2dPin children
    /*!
        \param NotConnected when true only pins which are not connected tp another pin will be deleted
        \param onlyTemporary when true only pins with the temporary flag set
        using a2dPin::SetTemporaryPin() will be deleted.
        These type of pins are used in automatic connection situations, and often
        after an editing attempt of other objects need to be deleted in the end.
        \param now if true remove pin object now!, else only delete flag is set,
        and it will be deleted in idle time.
    */
    void RemovePins( bool NotConnected = false, bool onlyTemporary = false, bool now = false );

    //! Set a2dPin children visible or not
    void ShowPins( bool onoff );

    //! Calls a2dPin::SetRenderConnected() for all pins
    /*! By default connected pins are not rendered, you can set it true for all pins here.
        But you can also set each pin individual.
    */
    void SetRenderConnectedPins( bool onoff );

    //! Remove all pin connections by issuing a2dCommand_DisConnectPins commands
    /*! This should be done before a a2dCommand_ReleaseObject is issued, else the pins will be disconnected to late.
        \param withundo if true use a2dCommand_DisConnectPins commands.
    */
    virtual void ClearAllPinConnections( bool withundo = true );

    //! set parent object of the pin or some other objects that needs a parent
    virtual void SetParent( a2dCanvasObject* WXUNUSED( parent ) ) {};

    //! return NULL, because a a2dCanvasObject normally does not have only one parent.
    //! In a derived class, it can be a defined.
    virtual a2dCanvasObject* GetParent() const { return NULL; };

    //! find wires startin at one Pin towards a second pin, used from within a2dPin do not call directly.
    /*!
        A depth first search for correct branches. Wire passed towards  second pin, are stored to result.
        Wires on the path to the second pin, do get the bin2 flag set.

        \param pinFrom start searching for pinTo here
        \param pinTo wire path ending at this pin is correct
        \param result wires to pinTo stored here, unless NULL
		\param wentToPin return where in the object another pin was found.
    */    
    virtual bool FindWiresPinToPin2( a2dPin* pinFrom, a2dPin* pinTo, a2dCanvasObjectList* result, a2dSmrtPtr<a2dPin>& wentToPin );

protected:

    //! prepare an object for being connected to a given pin
    /*!
        Before trying to connect in CanConnectWith() to the object DoCanConnectWith() is called,
        here the object can be prepared to be able to connect to the given pin here.
        The default implementation asks the object to GeneratePins() if autoccreate is set true.

        In a derived class one can do more complicated things to decide if a pin needs to be created or not.
    */
    virtual bool DoCanConnectWith( a2dIterC& ic, a2dPin* pin, double margin, bool autocreate );

    //\}
    //******************** END CONNECTION PINS AND WIRES********************/

    //********************** PROPERTIES *********************/
    /*! \name Properties
        a a2dCanvasObject has a list of general named properties
    */
    //\{
public:

    //! a2dCanvasObject set as property will be rendered after all other child objects
    //! when it is rendreed from a parent a2dCanvasObject
    inline void SetIsProperty( bool IsProperty ) { SetPending( true ); m_flags.m_IsProperty = IsProperty; }

    //! a2dCanvasObject set as property will be rendered after all other child objects
    //! when it is rendreed from a parent a2dCanvasObject
    inline bool GetIsProperty() const { return m_flags.m_IsProperty; }

    //! quickly set a property name __OBJECTTIP__
    /*!
        This function stores an OBJECTTIP property which will be shown when mouse is within an object.

        \param tip    the tip which should be shown
        \param x      x-pos of the tip
        \param y      y-pos of the tip
        \param size  font-size in world coordinates (font size of the font-param will be ignored, see a2dText documentation)
        \param angle rotation in degrees
        \param font the font to use

        \return a pointer to the a2dText object used to show the tip.
    */
    a2dText* SetObjectTip( const wxString& tip, double x, double y,  double size = 30, double angle = 0,
                           const a2dFont& font = *a2dDEFAULT_CANVASFONT );

    //! quickly set a property a2dTipWindowProperty
    /*!
    This function stores a a2dTipWindowProperty property which will be shown when mouse is within an object.
    The function a2dCanvasObject::OnEnterObject() will show it.

    \param tip    the tip which should be shown
    */
    void SetTipWindow( const wxString& tip );

    //! quickly get first property with name __OBJECTTIP__
    a2dObject* GetObjectTip();

    //! edit properties of the object
    /*!
        This default implementation sends the a2dPropertyEditEvent with
        id wxEVT_PROPOBJECT_EDITPROPERTIES_EVENT.

        This can be intercepted by any registrated class in order to edit the properties.
        When after return, the event its GetEdited() returns true, this indicates that the
        properties where indeed edited.

        \param id If property id is set only matching properties are selected
        \param withUndo   If true, the changes can be undone later.
    */
    virtual bool EditProperties( const a2dPropertyId* id, bool withUndo );

protected:

    //! This function is called after a property obn this object did change
    //! This is overloaded to set e.g. a pending flag
    void OnPropertyChanged( a2dComEvent& event );

public:

    //\}
    //**************** END PROPERTIES ***************/

    //********************** FLAGS *********************/
    /*! \name Flags
        a a2dCanvasObject has some falgs for general and algorithmic use.
        Other flags have special meaning or are used internally. The flags with special
        meaning (e.g. m_visible) are detailed in the proper sections.
    */
    //\{
public:

    //!set all bit flags in object that or true in mask to true or false
    /*!
        set specific flags to true or false

        \remark the object is not setpending when something changed ( actually the pending flag can be set here also )

        \param setOrClear if true sets the flag to true else to false
        \param which set only those flags in object to true or false
    */
    void SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which );

    //!Compares all flags in object to the given mask and return true is the same.
    bool CheckMask(  a2dCanvasObjectFlagsMask mask ) const;

    //!set bit flags of object (true or false) to given newmask values
    /*!
        \remark does not recurse into children

        \param newmask mask to set flags to in object (either true or false)
    */
    void SetFlags( a2dCanvasObjectFlagsMask newmask );

    //!get specific bitflag value
    bool GetFlag( const a2dCanvasObjectFlagsMask which ) const;

    //!get bitflags as an integer
    a2dCanvasObjectFlagsMask GetFlags() const;

    //!general flag use at will.
    /*!
        \remark
        This flag should only be used for temporarly purposes.
        This object uses this flag too and you might run into problems if you use this flag.
        It's a good practice to set this flag if you need it and reset this flag to <code>false</code>
        if you don't need it anymore. Another possibility might be to add a new property to this object
        if you want to be on the secure side.

        \param bin temporarely status information
    */
    inline void SetBin( bool bin ) { m_flags.m_bin = bin; }

    //!general flag use at will.
    inline bool GetBin() const {return m_flags.m_bin;}

    //!get the groupA flag
    /*! used to define operands in operation on two groups of objects
    */
    bool GetGroupA() const { return m_flags.m_a; }

    //!set the groupA flag
    /*! used to define operands in operation on two groups of objects
        \param value true to set group flag A
    */
    void SetGroupA( bool value ) { SetPending( true ); m_flags.m_a = value; }

    //!get the groupA flag
    /*! used to define operands in operation on two groups of objects
    */
    bool GetGroupB() const { return m_flags.m_b; }

    //!set the groupA flag
    /*! used to define operands in operation on two groups of objects
        \param value true to set group flag B
    */
    void SetGroupB( bool value ) { SetPending( true ); m_flags.m_b = value; }

    //!get the GeneratePins flag
    /*! used to define operands in operation on two groups of objects
    */
    bool GetGeneratePins() const { return m_flags.m_generatePins; }

    //!set the GeneratePins flag
    /*! used to define operands in operation on two groups of objects
        \param value true to set group flag C
    */
    void SetGeneratePins( bool value ) { SetPending( true ); m_flags.m_generatePins = value; }

    //!set IgnoreSetpending flag
    /*!If this flag is set, the object will be not be set pending in SetPending()*/
    void SetIgnoreSetpending( bool value = true ) { m_flags.m_ignoreSetpending = value; }

    //!get IgnoreSetpending flag
    /*!If this flag is set, the object will be not be set pending in SetPending()*/
    bool GetIgnoreSetpending( ) const { return m_flags.m_ignoreSetpending; }

    //!set static IgnoreAllSetpending flag
    /*!If this flag is set, all a2dCanvasObject will be not be set pending in SetPending()*/
    static void SetIgnoreAllSetpending( bool value = true ) { m_ignoreAllSetpending = value; }

    //!get static IgnoreSetpending flag
    /*!If this flag is set, all a2dCanvasObject will be not be set pending in SetPending()*/
    static bool GetIgnoreAllSetpending( ) { return m_ignoreAllSetpending; }

    void SetIgnoreLayer( bool value = true ) { m_flags.m_ignoreLayer = value; }

    bool GetIgnoreLayer( ) const { return m_flags.m_ignoreLayer; }

    void SetSubEdit( bool value ) { m_flags.m_subEdit = value; }
    bool GetSubEdit( ) const { return m_flags.m_subEdit; }
    void SetSubEditAsChild( bool value ) { m_flags.m_subEditAsChild = value; }
    bool GetSubEditAsChild( ) const { return m_flags.m_subEditAsChild; }
    void SetShowshadow( bool value ) { m_flags.m_showshadow = value; }
    bool GetShowshadow( ) const { return m_flags.m_showshadow; }
    void SetPushin( bool value ) { m_flags.m_pushin = value; }
    bool GetPushin( ) const { return m_flags.m_pushin; }
    void SetBin2( bool value ) { m_flags.m_bin2 = value; }
    bool GetBin2( ) const { return m_flags.m_bin2; }
    void SetPrerenderaschild( bool value ) { m_flags.m_prerenderaschild = value; }
    bool GetPrerenderaschild( ) const { return m_flags.m_prerenderaschild; }
    void SetVisiblechilds( bool value ) { m_flags.m_visiblechilds = value; }
    bool GetVisiblechilds( ) const  { return m_flags.m_visiblechilds; }
    void SetEditing( bool value ) { m_flags.m_editing = value; }
    bool GetEditing( ) const { return m_flags.m_editing; }
    void SetEditingRender( bool value ) { m_flags.m_editingCopy = value; }
    bool GetEditingRender( ) const { return m_flags.m_editingCopy; }
    void SetDoConnect( bool value ) { m_flags.m_doConnect = value; }
    bool GetDoConnect( ) const { return m_flags.m_doConnect; }
    void SetIsOnCorridorPath( bool value ) { m_flags.m_isOnCorridorPath = value; }
    bool GetIsOnCorridorPath( ) const { return m_flags.m_isOnCorridorPath; }
    void SetHasPins( bool value ) { m_flags.m_hasPins = value; }
    bool GetHasPins( ) const { return m_flags.m_hasPins; }
    void SetMouseInObject( bool value ) { m_flags.m_MouseInObject = value; }
    bool GetMouseInObject( ) const { return m_flags.m_MouseInObject; }
    void SetHighLight( bool value ) { m_flags.m_HighLight = value; }
    bool GetHighLight( ) const { return m_flags.m_HighLight; }
    void SetAlgoSkip( bool value ) { m_flags.m_AlgoSkip = value; }
    bool GetAlgoSkip( ) const { return m_flags.m_AlgoSkip; }

protected:

    //\}
    //**************** END FLAGS ***************/

    //********************** LAYERS *********************/
    /*! \name Layers
        a a2dDrawing is rendered in layers. Every canvas object belongs to excatly
        one layer.
    */
    //\{
public:

    //! Returns the layer index where this object is drawn upon.
    /*!
        The order of the a2dLayers in the root object decides if and when it will be drawn.

        \see a2dDrawing::SetLayerSetup
        \see a2dLayers
    */
    inline wxUint16 GetLayer() const { return m_layer; }

    //!set layer index where this object is drawn upon.
    /*!
        Default is layer wxLAYER_DEFAULT (colours etc taken from layer list)
        Some layers are predefined and used for special purposes (i.e. for selection of
        an object etc.). Please refer to wxLayerNames.

        \see wxLayerNames
        \see a2dLayerInfo
        \see a2dLayers

        \param layer the index of the layer
    */
    virtual void SetLayer( wxUint16 layer );

    //\}
    //**************** END LAYERS ***************/

    //********************** APPLICATION SPECIFIC EXTENSIONS *********************/
    /*! \name Application specific extensions.
        The functions in this section can be used to extend the functionality of
        a2dCanvasObject without adding new virtual member functions to the base class.
    */
    //\{
public:

    //!call fp for each object
    void foreach_f( void ( *fp ) ( a2dCanvasObject* item ) );

    //!call fp for each object
    void foreach_mf( void ( a2dCanvasObject::*mfp ) () );

    //!can be used by the user to implement a function that affects all a2dCanvas  derived objects
    virtual bool UserBaseFunction() { return true;}

    //!can be used by the user to implement a function that affects all a2dCanvas derived objects
    /*!
       a2dIOHandler can be used to transfer data to the function, for example a a2dWalkerIOHandler handler
       can be used to iterate the document, and calll this function on every object wanted.

       \param function function id, to be able to use this function for more tasks.
       \param handler a2dIOHandler which was use to iterate the document
    */
    virtual bool UserBaseFunctionEx( int WXUNUSED( function ), a2dIOHandler* WXUNUSED( handler ) = NULL ) { return true; }

    //!can be used by the user to implement a function using a variable argument list that affects all a2dCanvas derived objects
    virtual bool UserBaseFunctionVar( ... ) { return true;}

    //!can be used by the user to implement a function using a variable argument list and format string that affects all a2dCanvas  derived objects
    virtual bool UserBaseFunctionFormat( wxString format, ... ) { return true;}

    //! This is used to recursively walk through an object tree
    void WalkerWithContext( a2dIterC& ic, wxObject* parent, a2dWalkerIOHandlerWithContext& handler );

protected:

    //! iterate over this object and its children
    /*!
        This function allows you to extend the functionality of all a2dCanvasObject classes
        in a a2dDrawing, without adding extra members to these objects.

        Default functions are called on the a2dWalkerIOHandler, which redirect the
        calls to other functions based on this object its classname.
        On can register classes to a2dWalkerIOHandler or derived classes.
        This way for each unique object in the document there can be a function
        in a2dWalkerIOHandler.

        \return false if some object did not have a function attached via a2dWalkerIOHandler.

        See a2dWalkerIOHandler for more.
    */
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    // used if context is needed.
    virtual void DoWalkerWithContext( a2dIterC& ic, wxObject* parent, a2dWalkerIOHandlerWithContext& handler );

    //\}
    //********************** END APPLICATION SEPCIFIC EXTENSIONS *********************/

    //********************** DEBUGGING *********************/
    /*! \name Debugging functions
        These functions are only enabled in Debug mode
    */
    //\{
public:
#ifdef _DEBUG

    //! Dump an Object with its childs and properties
    /*! Note: this function is not virtual, because virtual functions cannot
        be called from the debugger
    */
    void Dump( int indent = 0 );
    //! Called by Dump to Dump class specific stuff
    /*! Add class specific info to line or output line and create a new line */
    virtual void DoDump( int indent, wxString* line );

#endif
    //\}

    //! when implemented the object without its children, is converted to
    /*!
        to a list of a2dVectorPath's.
        Else wxNullCanvasObjectList is returned.
    */
    virtual a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    //! convert to a list of polygons.
    /*!
        Return non wxNullCanvasObjectList if possible and implemented.
    */
    virtual a2dCanvasObjectList* GetAsPolygons( bool transform = true ) const;

    //! convert to a list of polylines.
    /*!
        Return non wxNullCanvasObjectList if possible and implemented.
    */
    virtual a2dCanvasObjectList* GetAsPolylines( bool transform = true ) const;

    //! convert to a  polygon.
    /*!
        Return non NULL if possible and implemented.
    */
    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const { return NULL; }

    void SetTemplate( bool b = true );
    void SetExternal( bool b = true );
    void SetUsed( bool b = true );

    bool GetTemplate() const ;
    bool GetExternal() const ;
    bool GetUsed() const ;

protected:

    bool ProcessCanvasEventChild( a2dIterC& ic, RenderChild& whichchilds, a2dHitEvent& hitEvent );

    bool ProcessCanvasEventChildOneLayer( a2dIterC& ic, RenderChild& whichchilds, a2dHitEvent& hitEvent );

public:

    //!only used for editable objects and under control of a editing tool.
    /*!
        If object is editable this function is used to initialize the object for editing.
        In general this means adding editing handles to the child list.
        In the event handling of the object those handles are hit and moved, the object itself
        is changed accordingly.

        \return true is this object can be edited and is initialized for that.
    */
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

protected:

    virtual bool DoIgnoreIfNotMember( const a2dPropertyId& id );

    //!root group for rendering and accessing the canvas's also contains layer settings
    a2dDrawing* m_root;

    //!holds flags for objects
    a2dCanvasOFlags m_flags;

    //bitset<64> m_bflags;

    //!holds value for flags to initialize m_flags
    static a2dCanvasOFlags m_flagsInit;

    //!allow hits on basis of those flags
    a2dCanvasOHitFlags m_hitflags;

    //!boundingbox in world coordinates
    a2dBoundingBox m_bbox;

    bool m_shapeIdsValid;
    a2dListId m_shapeIds;

    //! world extend in world coordinates.
    /*!
        Normally contains at least the stroke width when this is in world coordinates.
    */
    float m_worldExtend;

    //! Pixel extend
    /*!
        In case of pixel object or partial pixel object or pixel strokes,
        this will contain the needed oversize on top of the boundingbox.
        It is to be set in Update() or OnUpdate()
    */
    wxUint16 m_pixelExtend;

    //!layer of object, default wxLAYER_DEFAULT
    wxUint16 m_layer;

    //!used for positioning the object (x,y,ang,scale etc.)
    a2dAffineMatrix m_lworld;

    //!holds child objects
    a2dCanvasObjectList* m_childobjects;

    //! parse Cvg transform of object
    bool ParseCvgTransForm( a2dAffineMatrix& result, a2dIOHandlerXmlSerIn& parser );

    //!called by addPending
    virtual void DoAddPending( a2dIterC& ic );

    //! In derived object this should be overriden to calculate the boundingbox of the object without its children.
    /*!
        The default return a non Valid boundingbox.

        The real boundingbox of the object is often less desirable for editing. e.g. a rectangle with a contour,
        one does not want editing handles on the contour, instead they  should still be on the basic rectangle.
    */
    virtual a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //!Update derived Object specific things ( mainly boundingbox)
    /*!
    Calculates the boundingbox of the object (exclusif base class child objects but with other nested objects).

    \param mode way to update the objects
    \param childbox size of children boundingbox
    \param clipbox clip to this
    \param propbox size of properties boundingbox

    \remark in a derived class this function can also be used to update object specific cache data.

    \remark force may or may not have direct influence on the object itself, if this function is called directly
    for some reason (e.g from derived objects), you must invalidate the boudingbox yourself.
        GetDrawerBox()->SetValid( false );
    */
    virtual bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //!only used for editable objects and under control of a editing tool.
    /*!
        Do a clean up at the end of an editing sesion of the object.
        In general this means, remove editing handles from child list.
    */
    virtual void DoEndEdit() {};

public:

    //! when set all SetPending() calss are supressed.
    static bool m_ignoreAllSetpending;

    // member ids
    static a2dPropertyIdMatrix* PROPID_TransformMatrix;
    static a2dPropertyIdPoint2D* PROPID_Position;
    static a2dPropertyIdUint16* PROPID_Layer;
    static a2dPropertyIdBool* PROPID_Selected;
    static a2dPropertyIdBool* PROPID_Selectable;
    static a2dPropertyIdBool* PROPID_SubEdit;
    static a2dPropertyIdBool* PROPID_SubEditAsChild;
    static a2dPropertyIdBool* PROPID_Visible;
    static a2dPropertyIdBool* PROPID_Draggable;
    static a2dPropertyIdBool* PROPID_Showshadow;
    static a2dPropertyIdBool* PROPID_Filled;
    static a2dPropertyIdBool* PROPID_GroupA;
    static a2dPropertyIdBool* PROPID_GroupB;
    static a2dPropertyIdBool* PROPID_GeneratePins;
    static a2dPropertyIdBool* PROPID_Bin;
    static a2dPropertyIdBool* PROPID_Bin2;
    static a2dPropertyIdBool* PROPID_Pending;
    static a2dPropertyIdBool* PROPID_Snap;
    static a2dPropertyIdBool* PROPID_SnapTo;
    static a2dPropertyIdBool* PROPID_Pushin;
    static a2dPropertyIdBool* PROPID_Prerenderaschild;
    static a2dPropertyIdBool* PROPID_Visiblechilds;
    static a2dPropertyIdBool* PROPID_Editable;
    static a2dPropertyIdBool* PROPID_Editing;
    static a2dPropertyIdBool* PROPID_EditingRender;
    static a2dPropertyIdBool* PROPID_ChildrenOnSameLayer;
    static a2dPropertyIdBool* PROPID_DoConnect;
    static a2dPropertyIdBool* PROPID_IsOnCorridorPath;
    static a2dPropertyIdBool* PROPID_HasPins;
    static a2dPropertyIdBool* PROPID_IsProperty;
    static a2dPropertyIdBool* PROPID_MouseInObject;
    static a2dPropertyIdBool* PROPID_HighLight;
    static a2dPropertyIdBool* PROPID_Template;
    static a2dPropertyIdBool* PROPID_External;
    static a2dPropertyIdBool* PROPID_Used;
    static a2dPropertyIdBool* PROPID_Release;

    // commonly used member ids in derived classes
    static a2dPropertyIdCanvasObject* PROPID_Begin;
    static a2dPropertyIdCanvasObject* PROPID_End;
    static a2dPropertyIdDouble* PROPID_EndScaleX;
    static a2dPropertyIdDouble* PROPID_EndScaleY;
    static a2dPropertyIdBool* PROPID_Spline;
    static a2dPropertyIdDouble* PROPID_ContourWidth;

    // property ids
    static a2dPropertyIdBool* PROPID_preserveAspectRatio;
    static a2dPropertyIdBool* PROPID_DisableFeedback;
    static a2dPropertyIdBool* PROPID_Allowrotation;
    static a2dPropertyIdBool* PROPID_Allowsizing;
    static a2dPropertyIdBool* PROPID_Allowskew;
    static a2dPropertyIdBool* PROPID_IncludeChildren;
    static a2dPropertyIdBool* PROPID_SkipBase;
    static a2dPropertyIdRefObjectAutoZero* PROPID_Controller;
    static a2dPropertyIdCanvasObject* PROPID_Original;
    static a2dPropertyIdCanvasObject* PROPID_Editcopy;
    static a2dPropertyIdCanvasObject* PROPID_Parent;
    static a2dPropertyIdCanvasObject* PROPID_Objecttip;
    static a2dPropertyIdUint16* PROPID_Editmode;
    static a2dPropertyIdUint16* PROPID_Editstyle;
    static a2dPropertyIdUint16* PROPID_Index;
    static a2dPropertyIdCanvasShadowStyle* PROPID_Shadowstyle;
    static a2dPropertyIdFill* PROPID_Fill;
    static a2dPropertyIdStroke* PROPID_Stroke;
    static a2dPropertyIdUint32* PROPID_RefDesCount;
    static a2dPropertyIdUint32* PROPID_RefDesNr;

    //! set for objects that act as tool decorations, when a tool is in action.
    static a2dPropertyIdBool* PROPID_ToolDecoration;

    //! set for objects that act as tool object, when a tool is in action.
    static a2dPropertyIdVoidPtr* PROPID_ToolObject;

    //! set for objects that do not have to be saved
    static a2dPropertyIdBool* PROPID_TemporaryObject;

    //! used to store state ( redraw area) of the object as it was before a change
    static a2dPropertyIdBoundingBox* PROPID_BoundingBox;

    static a2dPropertyIdColour* PROPID_StrokeColour;
    static a2dPropertyIdColour* PROPID_FillColour;

    //! when an object should only be rendered in the view pointed to by this property.
    //! This is tipically used when using tool object.
    static a2dPropertyIdRefObject* PROPID_ViewSpecific;

    //! set in Startedit(), to be used to detect first (mouse)event sent to object.
    static a2dPropertyIdBool* PROPID_FirstEventInObject;

    //! used in GDSII and KEY format to specify the DATATYPE of elements
    /*! GDSII compatible to sub identify this object.
        you can use it as a special tagged object
    */
    static a2dPropertyIdUint16* PROPID_Datatype;

    //! used for objects that depend on 'aView' view when it comes to size.
    static a2dPropertyIdRefObject* PROPID_ViewDependent;
    //! used for objects with* PROPID_viewDependent but only for internal area
    static a2dPropertyIdMatrix* PROPID_IntViewDependTransform;

    //! some time property which a user wants to store
    static a2dPropertyIdDateTime* PROPID_DateTime;

    static a2dPropertyIdDateTime* PROPID_ModificationDateTime;

    static a2dPropertyIdDateTime* PROPID_AccessDateTime;

    //! when set used for popup menu by default in OnPopUpEvent()
    static a2dPropertyIdMenu* PROPID_PopupMenu;

    //! when set used for tip window by default in OnCanvasMouseEvent()
    static a2dPropertyIdWindow* PROPID_TipWindow;

    //! used in tool when grouping using tags
    static a2dPropertyIdTagVec* PROPID_Tags;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCanvasObject( const a2dCanvasObject& other );
};

#include "wx/canvas/canpin.h"

bool operator < ( const a2dCanvasObjectPtr& a, const a2dCanvasObjectPtr& b );

typedef bool ( *a2dCanvasObjectSorter ) ( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );
A2DCANVASDLLEXP extern a2dCanvasObjectSorter s_a2dCanvasObjectSorter;

#ifdef _DEBUG
// Here are two globals that can be used as registers in the debugger
extern a2dCanvasObject* _dbco1;
extern a2dCanvasObject* _dbco2;
#endif

#endif    // WXCANOBJ1


