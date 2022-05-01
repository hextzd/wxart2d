/*! \file wx/canvas/canobj2.h
    \brief Filtering and Iteration classes for rendering and traversing a a2dDrawing

    Classes for filtering object ( a2dCanvasObjectFilter )
    when iterating recursively through a document hierarchy.
    The class a2dIterC is used for holding the iteration context,
    which on its turn holds the drawing context when needed.

    \author Klaas Holwerda 

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj.h,v 1.85 2009/07/10 19:23:13 titato Exp $
*/

#ifndef __WXCANOBJ2_H__
#define __WXCANOBJ2_H__

//! class use by a2dIterC to filter objects for rendering.
/*!
    From a2dDrawing render cycles for the document are initiated.
    An iteration context is set up, and this context can have an object filter set,
    only a2dCanvasObject objects that return true, will be rendered.

    The idea is to derived from this class, and defined your own Filter()

    This object can contain dynamic properties which can be used for filtering.
    It is also refcounted for use with smart pointers.
*/
class A2DCANVASDLLEXP a2dCanvasObjectFilter : public a2dObject
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilter() {};

    ~a2dCanvasObjectFilter() {};

    //! called from a2dCanvasObject to filter objects for rendering
    virtual bool Filter( a2dIterC& WXUNUSED( ic ), a2dCanvasObject* WXUNUSED( canvasObject ) ) { return true; }

    //! called from a2dCanvasObject to reset filtering feature when filtered object goes out of context.
    virtual void EndFilter( a2dIterC& WXUNUSED( ic ), a2dCanvasObject* WXUNUSED( canvasObject ) ) {}
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dCanvasObjectFilter>;

#endif

//! filter on this layer and mask.
class A2DCANVASDLLEXP a2dCanvasObjectFilterLayerMask : public a2dCanvasObjectFilter
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterLayerMask( wxUint16 layer, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, a2dCanvasObjectFlagsMask antimask = a2dCanvasOFlags::NON )
    {
        m_layer = layer;
        m_mask = mask;
        m_antimask = antimask;
    }

    ~a2dCanvasObjectFilterLayerMask() {}

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

    wxUint16 m_layer;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectFlagsMask m_antimask;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; };
};

//! objects with m_editingcopy or m_toolobject are skipped.
class A2DCANVASDLLEXP a2dCanvasObjectFilterLayerMaskNoToolNoEdit : public a2dCanvasObjectFilterLayerMask
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterLayerMaskNoToolNoEdit( wxUint16 layer, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, a2dCanvasObjectFlagsMask antimask = a2dCanvasOFlags::NON )
        : a2dCanvasObjectFilterLayerMask( layer, mask, antimask )
    {
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

//! object not fitting the mask are drawn blind.
class A2DCANVASDLLEXP a2dCanvasObjectFilterOnlyNoMaskBlind : public a2dCanvasObjectFilter
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterOnlyNoMaskBlind( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
        : a2dCanvasObjectFilter()
    {
        m_mask = mask;
        m_maskedCanvasObject = 0;
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_maskedCanvasObject;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

//! objects not fitting the property are not drawn, inclusif its children.
/*!
     Mask is still checked for objects with that property and its children
     The drawing is only enabled by the filter, the top in a2dDrawing sets it off at start.
*/
class A2DCANVASDLLEXP a2dCanvasObjectFilterPropertyNoMaskBlind : public a2dCanvasObjectFilterOnlyNoMaskBlind
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterPropertyNoMaskBlind( const a2dPropertyId* id, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
        : a2dCanvasObjectFilterOnlyNoMaskBlind( mask )
    {
        m_id = id;
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

    const a2dPropertyId* m_id;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

//! filter for selected a2dCanvasObject's
/*!

*/
class A2DCANVASDLLEXP a2dCanvasObjectFilterSelected : public a2dCanvasObjectFilterOnlyNoMaskBlind
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterSelected( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
        : a2dCanvasObjectFilterOnlyNoMaskBlind( mask )
    {
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

//! filter for selected a2dCanvasObject's
/*!

*/
class A2DCANVASDLLEXP a2dCanvasObjectFilterSelected2 : public a2dCanvasObjectFilterOnlyNoMaskBlind
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterSelected2( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
        : a2dCanvasObjectFilterOnlyNoMaskBlind( mask )
    {
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

//! filter for selected a2dCanvasObject's
/*!

*/
class A2DCANVASDLLEXP a2dCanvasObjectFilterHighLighted : public a2dCanvasObjectFilterOnlyNoMaskBlind
{
public:

    //!
    /*!
    */
    a2dCanvasObjectFilterHighLighted( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
        : a2dCanvasObjectFilterOnlyNoMaskBlind( mask )
    {
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; };
};

//! filter for tool related a2dCanvasObject's
/*!

*/
class A2DCANVASDLLEXP a2dCanvasObjectFilterToolObjects : public a2dCanvasObjectFilterOnlyNoMaskBlind
{
public:

    //!constructor
    /*!
    */
    a2dCanvasObjectFilterToolObjects( const a2dPropertyId* id, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
        : a2dCanvasObjectFilterOnlyNoMaskBlind( mask )
    {
        m_id = id;
    }

    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject );

    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject );

protected:

    const a2dPropertyId* m_id;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; };
};


class A2DCANVASDLLEXP a2dIterPP
{
    friend class a2dIterC;
public:
    //! Update iteration context
    /*! Create an object of this type locally in a a2dCanvasObject child tree
        iteration function at each recusion level. This will update the iteration
        context and restore it later
    */
    a2dIterPP( a2dIterC& ic, a2dCanvasObject* object, OVERLAP clip = _IN, a2dHit type = a2dHit() );

    //! construction of intitial
    a2dIterPP( a2dIterC& ic, const a2dAffineMatrix& matrix = a2dIDENTITY_MATRIX, OVERLAP clip = _IN );

    //! copy constructor
    a2dIterPP( const a2dIterPP& cu );

    //! Restore the iteration context
    ~a2dIterPP();

    //! Get the current object
    a2dCanvasObject* GetObject() { return m_object; }

    //! Get the accumulated transforms up to and including m_object->m_lworld
    const a2dAffineMatrix& GetTransform() { return m_relativetransform; }

    //! Get the accumulated transforms up to and including m_object->m_lworld
    const a2dAffineMatrix& GetInverseTransform() { return m_inverseRelativetransform; }

    const a2dAffineMatrix& GetLocalTransform() { return m_localtransform; }

    //! type of the hit
    const a2dHit& GetHitType() const { return m_type; }

    //! the canvas object at the current level of iteration
    a2dCanvasObjectPtr m_object;

    a2dAffineMatrix m_localtransform;

    //! the accumulated transforms up to and including m_object->m_lworld
    a2dAffineMatrix m_relativetransform;

    //! inverse of m_relativetransform
    a2dAffineMatrix m_inverseRelativetransform;

    //! indicates type of constructor used.
    bool m_objectLevel;

    //! how far this object in the view being rendered
    OVERLAP m_clip;

    //! type of the hit
    a2dHit m_type;

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dIterPP* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dIterCU Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;

};

//! An object of this class will update a a2dIterC with the required information
/*! As iteration goes down the child hierarchy, a a2dIterCU is created to
    update a a2dIterC. It will update the parent list as well as the matrix

Assume the following object tree

\verbatim
     A
   /   \
  B     C
 / \   / \
D   E F   G
\endverbatim

  The constructors / destructors are called in the following order, and the values
of GetParent and GetPreviousSibling, GetPreviousOrParent and GetPreviousDeep are shown
(time is going from left to right):

\verbatim
Const:    A   B   D       E           C   F       G
Destr:                D       E   B           F       G   C   A

Parent: X   0   A   B   A   B   A   0   A   C   A   C   A   0   X

PrevSi: X   0   0   0   0   D   0   0   B   0   B   F   B   0   X

PrevOP: X   0   A   B   A   D   A   0   B   C   B   F   B   0   X

PrevD:  X   0   0   0   D   D   E   B   B   B   F   F   G   C   A

Stack:  -   A   A   A   A   A   A   A   A   A   A   A   A   A   -
        -   -   B   B   B   B   B   -   C   C   C   C   C   -   -
        -   -   -   D   -   E   -   -   -   F   -   G   -   -   -

\endverbatim

\see a2dIterC
*/
class A2DCANVASDLLEXP a2dIterCU
{
    friend class a2dIterC;
public:
    //! Update iteration context
    /*! Create an object of this type locally in a a2dCanvasObject child tree
        iteration function at each recusion level. This will update the iteration
        context and restore it later
    */
    a2dIterCU( a2dIterC& ic, a2dCanvasObject* object, OVERLAP clip = _IN );

    //! construction of intitial
    a2dIterCU( a2dIterC& ic, const a2dAffineMatrix& matrix = a2dIDENTITY_MATRIX, OVERLAP clip = _IN );

    //! copy constructor
    a2dIterCU( const a2dIterCU& cu );

    //! Restore the iteration context
    ~a2dIterCU();

    //! Get the current object
    a2dCanvasObject* GetObject() { return m_pp->m_object; }

    //! Get the accumulated transforms up to and including m_object->m_lworld
    const a2dAffineMatrix& GetTransform() { return m_pp->m_relativetransform; }

    //! Get the accumulated transforms up to and including m_object->m_lworld
    const a2dAffineMatrix& GetInverseTransform() { return m_pp->m_inverseRelativetransform; }

private:

	a2dIterPP* m_pp;

	a2dIterC* m_iterC;

};


//! mask flags for a2dDrawingPart::OnUpdate
/*! a2dCanViewUpdateFlags
\sa  a2dDrawingPart

    \ingroup docview
*/
enum a2dCanViewUpdateFlags
{
    a2dCANVIEW_UPDATE_NON  = 0x0000, /*!< nothing hinted */

    a2dCANVIEW_UPDATE_OLDNEW  = 0x0001, /*!< only add pending object areas from document to arealist of all drawer views,
                                           does reset pending objects */
    a2dCANVIEW_UPDATE_PENDING = 0x0002, /*!< only add pending object areas from document to arealist of this drawer view,
                                           do NOT reset pending objects */
    a2dCANVIEW_UPDATE_ALL     = 0x0004, /*!< total area displayed by drawer is added as pending to arealist
                                           (removes all other pending areas) */
    a2dCANVIEW_UPDATE_AREAS   = 0x0008, /*!< redraw arealist into buffer or directly to device when not double buffered */

    a2dCANVIEW_UPDATE_AREAS_NOBLIT = 0x0010, /*!< redraw arealist into buffer but do not blit them by directly deleting the redrawn area */

    a2dCANVIEW_UPDATE_BLIT         = 0x0020,  /*!< blit updated areas to device window*/

    a2dCANVIEW_UPDATE_VIEWDEPENDENT = 0x0040, /*!< update objects which are view dependent */

    a2dCANVIEW_UPDATE_VIEWDEPENDENT_RIGHTNOW = 0x0080, /*!< update objects which are view dependent right now*/

    a2dCANVIEW_UPDATE_PENDING_PREUPDATE = 0x0100, /*!< only add pending object areas from document to arealist of this drawer view,
                                           do NOT reset pending objects */

    a2dCANVIEW_UPDATE_PENDING_POSTUPDATE = 0x0200, /*!< only add pending object areas from document to arealist of this drawer view,
                                           do NOT reset pending objects */

    a2dCANVIEW_UPDATE_PENDING_PREVIEWUPDATE = 0x0800, /*!< same as  a2dCANVIEW_UPDATE_PENDING_PREUPDATE but event called before that*/

    a2dCANVIEW_UPDATE_SYNC_DRAWERS = a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT

};

typedef unsigned int a2dCanViewUpdateFlagsMask;

//! while iterating a a2dDrawing, this holds the context.
/*!
    The context is a collection of information, which might be needed when
    traversing the document.
    e.g. Transforms for drawing and conversion to absolute coordinates are stored here.

    Specific information in the context can be valid or not.
    If the information is set and is valid, a flag is set too.
    This flag is checked when information is asked for. Asking for non valid information
    will result in an assert.
*/
class A2DCANVASDLLEXP a2dIterC
{
    friend class a2dIterPP;
    friend class a2dIterCU;
    friend class a2dCorridor;

public:

    //! constructor used when drawer is not known
    a2dIterC();

    //! constructor used when drawer is known ( usually the case )
    /*!
        \param drawer the drawer that is currently in use.
        \param level how deep is the starting object to render from the ShowObject of the a2dDrawingPart.
    */
    a2dIterC( a2dDrawingPart* drawer, int level = 0 );

    //! destructor
    ~a2dIterC();

    //! when true, disable inversion of matrixes
    /*!
        e.g. when rendering a document the inverted matrix is not needed, so we disable the calculation to gain speed.
    */
    void SetDisableInvert( bool disableInvert ) { m_disableInvert = disableInvert; }

    //! see SetDisableInvert()
    inline bool GetDisableInvert() { return m_disableInvert; }

    //! Reset this object for beeing reused. It will keep the drawer but NULL object infos
    void Reset();

	a2dIterPP* Push( const a2dAffineMatrix& matrix, OVERLAP clip );

	a2dIterPP* Push( a2dCanvasObject* object, OVERLAP clip );

	void Pop();

    a2dIterPP* Last() { return m_contextList.back(); }

    //! used to extend a hittest with the number of pixels.
    /*! to be able to hit a line of width zero, a margin is needed to hit it,
        which is set here.
        In fact this value is directly converted to a value in world coordinates,
        using the current GetDrawer2D(). GetHitMarginWorld() returns this value.

        \remark default value is taken from a2dDrawingPart or a2dCanvasGlobal
    */
    void SetHitMarginDevice( int pixels );

    //! used to extend a hittest with the given margin in world coordinates.
    /*! to be able to hit a line of width zero, a margin is needed to hit it,
        which is set here.

        \remark default value is taken from a2dCanvasGlobal::GetHitMarginWorld
    */
    void SetHitMarginWorld( double world );

    //! Get HitMargin in World units.
    double GetHitMarginWorld();

    //! Transform a pixel extend from device to world units
    /*! If no drawer is there (e.g. command procesing) the return value will be zero */
    double ExtendDeviceToWorld( int extend );

    //! transformed to object its coordinate system
    double GetTransformedHitMargin();

    //! get the layer that is to be rendered
    inline wxUint16 GetLayer() { return m_layer; }

    //! set the layer that is to be rendered
    /*!
        wxLAYER_ALL has the special meaning that it renderers all layers at once.
    */
    void SetLayer( wxUint16 layer ) { m_layer = layer; }

    //! get setting for command generation or not.
    inline bool GetGenerateCommands() { return m_generateCommands; }

    //! set to generate command or not in certain situations.
    /*!
    */
    void SetGenerateCommands( bool generateCommands ) { m_generateCommands = generateCommands; }

    //!get the mapping matrix
    const a2dAffineMatrix& GetMappingTransform() const;

    //! get matrix which transforms directly from relative world coordinates to device
    const a2dAffineMatrix& GetUserToDeviceTransform() const;

    //! get current a2dDrawingPart
    a2dDrawingPart* GetDrawingPart() const;

    //! get current a2dDrawer2D
    a2dDrawer2D* GetDrawer2D() const;

    //!set drawstyle used for rendering the document
    /*!
        The drawstyle is set in a2dDrawing when rendering parts of the document.

        \param drawstyle one of the draw styles
    */
    void SetDrawStyle( a2dDocumentRenderStyle drawstyle ) { m_drawstyle = drawstyle; }

    //!get drawstyles used for drawing the document
    a2dDocumentRenderStyle GetDrawStyle() { return m_drawstyle; }

    //! when traversing tree this the object one level higher.
    /*!
        Used internal during rendering etc.
        During recursive traversing a a2dDrawing from top group that is displayed
        this holds the a2dCanvasObject that is one level higher in the branch that is traversed
        used during rendering etc., to know what is the parent object in a branch that is being rendered
    */
    a2dCanvasObject* GetParent() const;

    //! the object where the iterative context is currently
    a2dCanvasObject* GetObject() const;

    //! Get the accumulated transform up to and including m_lworld of the current object
    /*! This converts from relative local coordinates of the current object to world coordinates.
        This matrix transforms all drawing primitives used to draw a a2dCanvasObject from relative
        world coordinates to absolute world coordinates.
    */
    const a2dAffineMatrix& GetTransform() const;

    //! Inverse of GetTransform()
    const a2dAffineMatrix& GetInverseTransform() const;

    //! Get the accumulated transform up to but NOT including m_lworld of the current obejct
    /*! This converts from local coordinates of the curent object to world coordinates
        !!!! I am not sure if it should contains the view transform (world->device) as well !!!!
    */
    const a2dAffineMatrix& GetParentTransform() const;

    //! inverse of GetParentTransform()
    const a2dAffineMatrix& GetInverseParentTransform() const;

    //! number of levels deep we are inside a document as seen from the m_top
    int GetLevel() const { return m_levels; }

    //! to set corridor path, its a2dCanvasOFlags::IsOnCorridorPath flag is set on or off.
    /*!
        The iteration context knows its parent object via its m_bottom and m_parent a2dIterCU.
        Each a2dIterCU added when going deeper into the drawing hierarchy, knows from which object it came.
        This way, it is possible to iterate back to the top/show object in a a2dDrawingPart.
        All canvasobjects passed that way is called the event path.
        Of course when going up in hierarchy, the event path is becoming smaller again. And when arriving at the top
        it will be zero.
        To preserve the event path to a specific canvas object, a flag can be set along the canvas objects in
        the current event path. Those flags will be kept intact inside the canvasobjects, even if the event path
        is changing after setting the flags.
        The a2dCanvasOFlags::IsOnCorridorPath set this way, is/can be used to find and redirect events to the
        objects along or at the end of this path. The path created like this is called the Corridor path.

        The corridor is used to redirect events to the end corridor object, even
        if the object is a deeper nested child. Events go to the last object in the corridor, next normal event processing
        continues for the object or nested objects. 

        \param OnOff to set the corridor path on or off.
    */
    void SetCorridorPath( bool OnOff );

    /* set corridor path to parent of current iteration context */
    void SetCorridorPathToParent();

    //! adds to corridorpath, and sets the object at the end of the corridor to
    //! the given object.
    void SetCorridorPathToObject( a2dCanvasObject* object );

    //! when there is a corridor path set, this return if end has bin found while iterating.
    bool GetFoundCorridorEnd() { return m_foundCorridorEnd; }

    //! when there is a corridorPath set, this is used internal to set this flag to indicate that the end of it was found.
    void SetFoundCorridorEnd( bool foundCorridorEnd ) { m_foundCorridorEnd = foundCorridorEnd; }

    //! Set strokeworld extend of last added object, used in a2dCanvasObject::DoIsHitWorld()
    /*!
        Holds the size/width of the Stroke in world coordinates, else 0.
        This can be used in derived a2dCanvasObject to do a proper hittest.
        If the stroke is of type pixel, that value will be converted into world coordinates.
    */
    void SetStrokeWorldExtend( double worldStrokeExtend ) { m_worldStrokeExtend = worldStrokeExtend; }

    //! \see SetWorldStrokeExtend()
    double GetWorldStrokeExtend() { return m_worldStrokeExtend; }

    //! what is the clipping withing the current view for the last added object in context
    OVERLAP GetClipStatus() const;

    //! what is the clipping withing the current view for the second last added object in context
    OVERLAP GetParentClipStatus() const;

    //! set the clipping withing the current view for the last added object in context
    void SetClipStatus( OVERLAP status );

    //! get the filter set for the iteration context.
    a2dCanvasObjectFilter* GetObjectFilter() { return m_objectFilter; }

    //! set object filter class.
    void SetObjectFilter( a2dCanvasObjectFilter* filter ) { m_objectFilter = filter; }

    //! apply object filter
    bool FilterObject( a2dCanvasObject* canvasObject );

    //! called when filter ends
    void EndFilterObject( a2dCanvasObject* canvasObject );

    //! if set the rendering is done layers by layer from the top
    void SetPerLayerMode( bool value ) { m_perLayerMode = value; }

    //! if set the rendering is done layers by layer from the top
    bool GetPerLayerMode() { return m_perLayerMode; }

    //! when set child object in derived a2dCanvasObject are rendered, else only the object itself.
    void SetRenderChildDerived( bool value ) { m_renderChildDerived = value; }

    //! when set child object in derived a2dCanvasObject are rendered, else only the object itself.
    bool GetRenderChildDerived() { return m_renderChildDerived; }

    //! set during event processing down the hierarchy, to the deepest object that was hit
    void SetDeepestHit( a2dCanvasObject* canvasObject ) { m_deepestHit = canvasObject; }

    //! get deepest object that was hit during event processing down the hierarchy.
    a2dCanvasObject* GetDeepestHit() const { return m_deepestHit; }

    void SetUpdateHint( a2dCanViewUpdateFlagsMask updateHint ) { m_updateHint = updateHint; }

    a2dCanViewUpdateFlagsMask GetUpdateHint() { return m_updateHint; }

	a2dSmrtPtrList< a2dIterPP > m_contextList;
	a2dSmrtPtrList< a2dIterPP > m_contextListDeep;

private:

    //! if true no inversion on matrixes is done.
    bool m_disableInvert;

    //!world to device coordinate mapping
    a2dAffineMatrix m_mapping;

    //!relative world to device transform matrix ( so includes mapping matrix )
    a2dAffineMatrix m_usertodevice;

    //! during event processing down the hierarchy, this is set to deepest object hit.
    a2dSmrtPtr<a2dCanvasObject> m_deepestHit;

    //! from which a2dDrawingPart the iteration started.
    a2dSmrtPtr<a2dDrawingPart> m_drawingPart;

    //! how close in pixles does a hit need to be to the object you are trying to hit.
    /*! this is in world units */
    double m_hitmargin_world;

    //! How much child levels deep are we, seen from the ShowObject of the a2dDrawingPart.
    int m_levels;

    //! when there is a corridorPath set, this is set when end is found.
    bool m_foundCorridorEnd;

    //! \see SetWorldExtend()
    double m_worldStrokeExtend;

    //! drawstyles used to render document
    a2dDocumentRenderStyle m_drawstyle;

    //! for filtering objects when e.g. rendering
    a2dSmrtPtr<a2dCanvasObjectFilter> m_objectFilter;

    //! the layer that is currently rendered.
    wxUint16 m_layer;

    //! see SetGenerateCommands()
    bool m_generateCommands;

    bool m_ownDrawer;

    //! if set the rendering is done layers by layer from the top
    bool m_perLayerMode;

    //! when set child object in derived a2dCanvasObject are rendered, else only the object itself.
    bool m_renderChildDerived;

    a2dCanViewUpdateFlagsMask m_updateHint;

};


#endif    // WXCANOBJ2



