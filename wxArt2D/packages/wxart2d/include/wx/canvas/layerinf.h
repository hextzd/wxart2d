/*! \file wx/canvas/layerinf.h
    \brief layer settings for a a2dCanvasDocument
    Holds layers settings classes

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: layerinf.h,v 1.37 2009/04/02 19:31:58 titato Exp $
*/

#ifndef __LAYERINF_H__
#define __LAYERINF_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/geometry.h"
#include "wx/artbase/afmatrix.h"
//#include "wx/canvas/xmlpars.h"
#include "wx/artbase/liner.h"
#include "wx/artbase/bbox.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/canobj.h"
#include <map>

/******************************************************************************
Layers settings
*******************************************************************************/

const double LAYERINFO_WIDTH = 100;
const double LAYERINFO_HEIGHT  = 100;

class a2dSpan
{
public:

    a2dSpan()
    {
        m_from = "";
        m_to = "";
    }

    a2dSpan( const wxString& spanString )
    {
        m_from = spanString.BeforeFirst( ':' );
        m_to = spanString.AfterFirst( ':' );
    }

    bool IsValid()
    {
        return !m_from.IsEmpty() && !m_to.IsEmpty();
    }

    wxString GetFrom() const { return m_from; }
    wxString GetTo() const { return m_to; }

    wxString GetAsString() const
    {
        wxString out;
        out << m_from << ":" << m_to;
        return out;
    }

    wxString m_from;
    wxString m_to;
};

//!contains the layer properties for one layer,
/*!settings for brush and stroke and other layer related settings.

    \ingroup canvasobject style
*/
class A2DCANVASDLLEXP a2dLayerInfo : public a2dCanvasObject
{
    DECLARE_DYNAMIC_CLASS( a2dLayerInfo )

public:
    //!constructor
    a2dLayerInfo();

    //!constructor
    /*!
        The index of the layer should be between
        <code>wxLAYER_USER_FIRST</code> and <code>wxLAYER_USER_LAST</code>
        to avoid conflicts with predefined layers.

        \see wxLayerNames

        \param index layer index
        \param name layer name
		\parem parent of which layersetup this is a part.
    */
    a2dLayerInfo( wxUint16 index, wxString name );

    //!constructor
    a2dLayerInfo( const a2dLayerInfo& other, CloneOptions options = clone_deep, a2dRefMap* refs = NULL );

    ~a2dLayerInfo();

    //! Get name for layer
    virtual wxString GetName() const;

    //! Set name for layer
    /*!If set the name will be returned for GetName() from now on.

        \param name name to be set for the object
    */
    virtual void SetName( const wxString& name );

	//! sent a sig_changedLayerInfo to a2dDrawing
	void SignalChange();

	//! setpending to also set a2dLayers pending
    void SetPending( bool pending );

    //!is the layer visible
    bool GetVisible();

    //!can objects on this layer be selected
    bool GetSelectable();

    //! order index for drawing order.
    //! The layers are sorted on order before they or rendered.
    //! Rendering in order of visibility, requires rendering in reverse order.
    int GetOrder() const;

    //!does this layer need to be read in from a file.
    bool GetRead();

    //!mapping of external file layer to internal layer index
    int GetInMapping();

    //!mapping of internal layer index  to external layer index
    int GetOutMapping();

    //!stroke defined in pixels?
    bool GetPixelStroke();

    //!are there objects on this layer
    bool GetAvailable() { return m_layeravailable; }

    //! set layer available (will be rendered)
    void SetAvailable( bool status ) { m_layeravailable = status; }

    //! return the default pin class of this layer
    a2dPinClass* GetPinClass() const { return m_pinclass; }

    //! Set the default pin class for this layer
    void SetPinClass( a2dPinClass* pinClass ) { m_pinclass = pinClass; }

    //! set layer visible
    void SetVisible( bool status );
    //! set layer selectable
    void SetSelectable( bool status );
    //! set drawing order for layer
    void SetOrder( wxUint16 order );
    //! read this layer from input files
    void SetRead( bool status );
    //! read this layer from the given layer in input files
    void SetInMapping( wxUint16 layer );
    //! write this layer from the given layer to output files
    void SetOutMapping( wxUint16 layer );

    //! make stroke of layer a pixelstroke
    void SetPixelStroke( bool pixel );

    //! this layer is part of a group of layers to present a feature ( metalized via etc. ).
    const wxString& GetFeature() const { return m_feature; }

    //! this layer is part of a group of layers to present a feature ( metalized via etc. ).
    void SetFeature( const wxString& feature ) { m_feature = feature; }

    //! extra as part of a feature.
    const wxString& GetSubFeature() const { return m_subFeature; }

    //! extra as part of a feature.
    void SetSubFeature( const wxString& subfeature ) { m_subFeature = subfeature; }

    //! type of layer (mask, holes etc.)
    const wxString& GetType() const { return m_type; }

    //! type of layer (mask, holes etc.)
    void SetType( const wxString& type ) { m_type = type; }

    //! which side of board
    const wxString& GetSide() const { return m_side; }

    //! which side of board
    void SetSide( const wxString& side ) { m_side = side; }

    //! span being two layer names
    /*!
        Used to tell for layers of type hole/drill, in between which other two layers this hole starts and ends.
    */
    a2dSpan GetSpan() const { return m_span; }

    //! see GetSpan()
    void SetSpan( a2dSpan span ) { m_span = span; }

    void SetSpan( const wxString& spanString ) { m_span = a2dSpan( spanString ); }

    //! what is this layer for.
    wxString& GetDescription() { return m_description; }

    void SetDescription( const wxString& description ) { m_description = description; }

    //! used to save area occupied by all primitives on the layer
    double GetArea() { return m_area; }

    //! used to save area occupied by all primitives on the layer
    void SetArea( double area ) { m_area = area; }

    //! get if the mask is negative or positive
    bool GetPolarity() { return m_polarity; }

    //! set if the mask is negative or positive
    void SetPolarity( bool polarity ) { m_polarity = polarity; }

    //! in case of layers containing holes, what is the default hoel on the layer.
    //! May be overruled by properties on holes.
    bool GetPlated() { return m_plated; }

    //! in case of layers containing holes, what is the default hoel on the layer.
    //! May be overruled by properties on holes.
    void SetPlated( bool plated ) { m_plated = plated; }

    //! get currently counted number of objects on this layer inside a document.
    wxUint32 GetObjectCount() { return m_canvasObjectCount; }

    //! set currently counted number of objects on this layer inside a document.
    void SetObjectCount( wxUint32 numberOfObjects ) { m_canvasObjectCount = numberOfObjects; }

    //! increment currently counted number of objects on this layer inside a document.
    void IncrementObjectCount() { m_canvasObjectCount++; }

    //! decrement currently counted number of objects on this layer inside a document.
    void DecrementObjectCount() { m_canvasObjectCount--; }

    //! get currently counted number of objects on this layer inside a document.
    wxUint32 GetPreviousObjectCount() { return m_canvasPreviousObjectCount; }

    //! set currently counted number of objects on this layer inside a document.
    void SetPreviousObjectCount( wxUint32 numberOfObjects ) { m_canvasPreviousObjectCount = numberOfObjects; }

    //! layer became empty or filled?
    bool StateChange()
    {
        return
            (
                (m_canvasPreviousObjectCount != 0  && m_canvasObjectCount == 0) ||
                (m_canvasPreviousObjectCount == 0  && m_canvasObjectCount != 0)
            );
    }

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    friend class a2dLayers;

#if wxART2D_USE_CVGIO
    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //virtual bool DoStartEdit( int editmode, wxEditStyle editstyle );

    //! called via a2dPropertyId SetPropertyToObject() etc.
    void OnPropertyChanged( a2dComEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //!is the layer selectable
    bool m_layerselectable;

    //!is the layer visible
    bool m_layervisible;

    //!will the layer be read from a file
    bool m_readlayer;

    //! used to check if a layer is indeed available in a document or whatever strcuture.
    //! a2dWalker_SetAvailable can set it
    bool m_layeravailable;

    //!drawing order number
    wxUint16 m_order;

    //!mapping to this layer from the file layer in the input file
    wxUint16 m_inmap;

    //!mapping to this layer from the file layer in the output file
    wxUint16 m_outmap;

    //! width of rectangle
    double m_width;

    //! height of rectangle
    double m_height;

    //! name of the layer
    wxString m_layername;

    //! this layer is part of a group of layers to present a feature ( metalized via etc. ).
    wxString m_feature;

    //! extra as part of a feature.
    wxString m_subFeature;

    //! type of layer (mask, holes etc.)
    wxString m_type;

    //! which side of board
    wxString m_side;

    //! Used to tell for layers of type hole, in between which other two layers this hole starts and ends.
    a2dSpan m_span;

    //! what is this layer for.
    wxString m_description;

    //! used to save area occupied by all primitives on the layer
    double m_area;

    //! negative or positive
    bool m_polarity;

    //! in case of layers containing holes, what is the default hoel on the layer.
    //! May be overruled by properties on holes.
    bool m_plated;

    //! The class defines to which other pins a pin on this layer can connect
    /*!
        Used for objects on this layer, when generating new default pins, see a2dPinClass::Any
    */
    a2dPinClass* m_pinclass;

    //! Gives the number of object on this layer in a document.
    //! Only accurate after a SetAvailable
    mutable wxUint32 m_canvasObjectCount;

    mutable wxUint32 m_canvasPreviousObjectCount;

public:

	DECLARE_EVENT_TABLE()

public:

    //! when one layer its a2dLayerInfo has changed ( e.g. visible or order of rendering )
    static const a2dSignal sig_changedLayerInfo;

    //! member ids
    static a2dPropertyIdString* PROPID_layerName;
    static a2dPropertyIdBool* PROPID_layerSelectable;
    static a2dPropertyIdBool* PROPID_layerVisible;
    static a2dPropertyIdBool* PROPID_readlayer;
    static a2dPropertyIdUint16* PROPID_order;
    static a2dPropertyIdUint16* PROPID_inmap;
    static a2dPropertyIdUint16* PROPID_outmap;

	DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    //a2dLayerInfo( const a2dLayerInfo &other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP std::allocator< a2dSmrtPtr<a2dLayerInfo> >;
template class A2DCANVASDLLEXP std::vector< a2dSmrtPtr<a2dLayerInfo> >;
#endif

A2DCANVASDLLEXP_DATA( extern a2dSmrtPtr<a2dLayerInfo> ) wxNullLayerInfo;

typedef a2dSmrtPtr<a2dLayerInfo> a2dLayerInfoPtr;
typedef std::vector<a2dLayerInfoPtr> a2dLayerIndex;

typedef std::map< wxUint16, a2dLayerInfoPtr> a2dLayerMapNr;
typedef std::map< wxString, a2dLayerInfoPtr> a2dLayerMapName;

//!contains the setup for the layers, the setup contains for each layer number
//!all settings like stroke and brush visibility.
/*!
    The a2dLayers is derived from a2dCanvasObject, to be able to view/edit
    a2dLayers as if it is a a2dCanvasDocument.

    To traverse the array based on the order set for each layer,
    you ask for the orderarray, which gives layer number sorted on order

    \sa a2dLayerInfo

    \ingroup canvasobject style
*/
class A2DCANVASDLLEXP a2dLayers : public a2dCanvasObject
{
    DECLARE_DYNAMIC_CLASS( a2dLayers )

public:

    enum Index
    {
        //! index on layer number
        OnLayer,
        //! index on layer order
        OnOrder,
        //! index on layer reverse order
        OnReverseOrder
    };

    a2dLayers();

    a2dLayers( const a2dLayers& other, CloneOptions options, a2dRefMap* refs );

    ~a2dLayers();

    //! set filename based on file where layers were read from
    /*!
        If the layers are part of the document file, this is empty.
    */
    void SetFileName( const wxFileName& filename ) { m_filename = filename; }

    //! get filename of layers files from which this object was read.
    wxFileName& GetFileName() { return m_filename; }

    //! next to base sets m_indexed false
    void SetPending( bool pending );

    void AddIfMissing( int layer );

    wxUint16 AddIfMissing( const wxString& layerName );

    //! element operator
    a2dLayerInfo* operator[] ( int layer );

    //! element operator
    //const a2dLayerInfo* operator[] ( int indx ) const;

    a2dLayerInfo* GetLayerInfo( int layer );

    //! return array index on Layer
    a2dLayerMapNr& GetLayerIndex();

    //! return array sorted on Layer
    a2dLayerIndex& GetLayerSort();
    //! return array sorted on Order
    a2dLayerIndex& GetOrderSort();
    //! return array sorted on ReverseOrder
    a2dLayerIndex& GetReverseOrderSort();

    //! save layers to a file
    bool LoadLayers( const wxString& filename );

    //! save layer object to CVG file
    bool SaveLayers( const wxString& filename );

    void InitWith( int highlayer, bool blackWhite );

    wxString GetName( int layernumber );
    int GetNumber( const wxString& name, bool createIfMissing = false );

    a2dStroke GetStroke( int layernumber );
    bool  GetVisible( int layernumber );
    bool  GetSelectable( int layernumber );
    a2dFill GetFill( int layernumber );
    int   GetOrder( int layernumber );
    bool  GetRead( int layernumber );
    int   GetInMapping( int layernumber );
    int   GetOutMapping( int layernumber );

    bool  GetPixelStroke( int layernumber );

    void SetName( int layernumber, const wxString& name );
    void SetStroke( int layernumber, const a2dStroke& stroke );
    void SetVisible( int layernumber, bool = true );
    void SetAvailable( int layernumber, bool = true );
    void SetSelectable( int layernumber, bool = true );
    void SetFill( int layernumber, const a2dFill& fill );
    void SetOrder( int layernumber, int status );
    void SetRead( int layernumber, bool = true );
    void SetInMapping( int layernumber, wxUint16 layer );
    void SetOutMapping( int layernumber, wxUint16 layer );

    //! sets a a2dPatternFill to this stipple, if not a a2dPatternFill, it will be changed.
    void SetPattern( int layernumber, const wxBitmap& stipple );

    void SetPixelStroke( int layernumber, bool = true );

    //! set all layers visible
    void SetAll_Layers_Visible( bool onoff );

    //! set all layers read
    void SetAll_Layers_Read( bool onoff );

    //! set all visible layers selectable
    void SetAll_Layers_Selectable( bool onoff );

    //! set all visible layers outline
    void SetAll_Layers_Outline( bool onoff );

    void SetAllLayersAvailable( bool onoff );

    //! set available flag based on given a2dDrawing
    void SetAvailable( a2dDrawing* drawing );

    //! set available flag based on given other layersetup
    void SetAvailable( a2dLayers* other );

    //! index on layer id and layer name will be updated.
    /*!
        When loading layer settings in some way, not all layers do have to be defined.
        The index maps, do search the layers, and create a map based on layer number and layer name.
        The index is used to quickly get to the layer properties of a certain layer, without having to iterate through all of them.
    */
    void UpdateIndexes();

    void AddDefaultLayers();

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dLayerMapNr m_layermapNr;
    a2dLayerMapName m_layermapName;

    a2dLayerIndex m_layerArrayOnLayer;
    a2dLayerIndex m_layerArrayOnOrder;
    a2dLayerIndex m_layerArrayOnReverseOrder;

    //! to test if index array are valid
    bool m_indexed;

public:
    static a2dPropertyIdBool* PROPID_visibleAll;
    static a2dPropertyIdBool* PROPID_readAll;
    static a2dPropertyIdBool* PROPID_selectableAll;

	static const a2dSignal sig_visibleAll;
	static const a2dSignal sig_selectAll;
	static const a2dSignal sig_outlineAll;
	static const a2dSignal sig_readAll;
	static const a2dSignal sig_availableAll;

    DECLARE_PROPERTIES()

private:

    wxFileName m_filename;

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dLayers( const a2dLayers& other );
};

extern bool LayerSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );

extern bool OrderSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );


extern bool ReverseOrderSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );


#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dLayers>;
#endif

#endif

