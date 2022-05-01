/*! \file canvas/src/layerinf.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: layerinf.cpp,v 1.106 2009/05/20 18:42:10 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/canobj.h"
#include "wx/artbase/liner.h"
#include "wx/canvas/drawer.h"

#include <algorithm>
#include <functional>

struct SortLayerOrder : public std::binary_function< const a2dLayerInfoPtr&, const a2dLayerInfoPtr&, bool>
{
    bool operator()( const a2dLayerInfoPtr& x, const a2dLayerInfoPtr& y )
    {
        if ( x == wxNullLayerInfo && y == wxNullLayerInfo )
            return false;
        if ( x != wxNullLayerInfo && y == wxNullLayerInfo )
            return true;
        if ( x == wxNullLayerInfo && y != wxNullLayerInfo )
            return false;

        if ( x->GetOrder() < y->GetOrder() )
            return true;
        return false;
    }
};

struct SortLayerReverseOrder : public std::binary_function< const a2dLayerInfoPtr&, const a2dLayerInfoPtr&, bool>
{
    bool operator()( const a2dLayerInfoPtr& x, const a2dLayerInfoPtr& y )
    {
        if ( x == wxNullLayerInfo && y == wxNullLayerInfo )
            return false;
        if ( x != wxNullLayerInfo && y == wxNullLayerInfo )
            return true;
        if ( x == wxNullLayerInfo && y != wxNullLayerInfo )
            return false;

        if ( x->GetOrder() > y->GetOrder() )
            return true;
        return false;
    }
};

struct SortLayerNr : public std::binary_function< const a2dLayerInfoPtr&, const a2dLayerInfoPtr&, bool>
{
    bool operator()( const a2dLayerInfoPtr& x, const a2dLayerInfoPtr& y )
    {
        if ( x == wxNullLayerInfo && y == wxNullLayerInfo )
            return false;
        if ( x != wxNullLayerInfo && y == wxNullLayerInfo )
            return true;
        if ( x == wxNullLayerInfo && y != wxNullLayerInfo )
            return false;

        if ( x->GetLayer() < y->GetLayer() )
            return true;
        return false;
    }
};


bool LayerSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dLayerInfo* firstc = wxStaticCast( x.Get(), a2dLayerInfo );
    a2dLayerInfo* secondc = wxStaticCast( y.Get(), a2dLayerInfo );

    if ( firstc->GetLayer() > secondc->GetLayer() )
        return true;
    return false;
}

bool OrderSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dLayerInfo* firstc = wxStaticCast( x.Get(), a2dLayerInfo );
    a2dLayerInfo* secondc = wxStaticCast( y.Get(), a2dLayerInfo );

    if ( firstc->GetOrder() < secondc->GetOrder() )
        return true;
    return false;
}

bool ReverseOrderSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dLayerInfo* firstc = wxStaticCast( x.Get(), a2dLayerInfo );
    a2dLayerInfo* secondc = wxStaticCast( y.Get(), a2dLayerInfo );

    if ( firstc->GetOrder() > secondc->GetOrder() )
        return true;
    return false;
}

/**************************************************
    a2dLayerInfo
**************************************************/
IMPLEMENT_DYNAMIC_CLASS( a2dLayerInfo, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dLayers, a2dCanvasObject )

a2dPropertyIdString* a2dLayerInfo::PROPID_layerName = NULL;
a2dPropertyIdBool* a2dLayerInfo::PROPID_layerSelectable = NULL;
a2dPropertyIdBool* a2dLayerInfo::PROPID_layerVisible = NULL;
a2dPropertyIdBool* a2dLayerInfo::PROPID_readlayer = NULL;
a2dPropertyIdUint16* a2dLayerInfo::PROPID_order = NULL;
a2dPropertyIdUint16* a2dLayerInfo::PROPID_inmap = NULL;
a2dPropertyIdUint16* a2dLayerInfo::PROPID_outmap = NULL;

INITIALIZE_PROPERTIES( a2dLayerInfo, a2dCanvasObject )
{
    A2D_PROPID_M_F( a2dPropertyIdString, a2dLayerInfo, layerName, wxT( "" ), m_layername, a2dPropertyId::flag_notify );
    A2D_PROPID_M_F( a2dPropertyIdBool, a2dLayerInfo, layerSelectable, false, m_layerselectable, a2dPropertyId::flag_notify );
    A2D_PROPID_M_F( a2dPropertyIdBool, a2dLayerInfo, layerVisible, false, m_layervisible, a2dPropertyId::flag_notify );
    A2D_PROPID_M_F( a2dPropertyIdBool, a2dLayerInfo, readlayer, false, m_readlayer, a2dPropertyId::flag_notify );
    A2D_PROPID_M_F( a2dPropertyIdUint16, a2dLayerInfo, order, false, m_order, a2dPropertyId::flag_notify );
    A2D_PROPID_M_F( a2dPropertyIdUint16, a2dLayerInfo, inmap, false, m_inmap, a2dPropertyId::flag_notify );
    A2D_PROPID_M_F( a2dPropertyIdUint16, a2dLayerInfo, outmap, false, m_outmap, a2dPropertyId::flag_notify );
    return true;
}

const a2dSignal a2dLayerInfo::sig_changedLayerInfo = wxNewId();

a2dSmrtPtr<a2dLayerInfo> wxNullLayerInfo = new a2dLayerInfo();

BEGIN_EVENT_TABLE( a2dLayerInfo, a2dCanvasObject )
    EVT_COM_EVENT( a2dLayerInfo::OnPropertyChanged )
END_EVENT_TABLE()

a2dLayerInfo::a2dLayerInfo()
{
    m_layer = 0;
    m_layeravailable = false;
    m_layervisible    = true;
    m_layerselectable = true;
    m_readlayer  = true;
    m_order      = 0;
    m_polarity = true;
    m_plated = false;
    m_area = 0;

    m_inmap   = 0;
    m_outmap  = 0;

    m_width = LAYERINFO_WIDTH;
    m_height = LAYERINFO_HEIGHT;

    m_pinclass = a2dPinClass::Standard;

    m_type = wxT( "SIGNAL" );
    m_side = wxT( "TOP" );

    m_canvasObjectCount = 0;
    m_canvasPreviousObjectCount = 0;
}

a2dLayerInfo::a2dLayerInfo( wxUint16 index, wxString name )
{
    wxASSERT_MSG( wxMAXLAYER > index, wxT( " wxLAYER_ALL layer is reserved do not use" ) );

    m_layer = index;

    m_layeravailable = false;
    m_layervisible    = true;
    m_layerselectable = true;
    m_readlayer  = true;
    m_order      = index;
    m_polarity = true;
    m_plated = false;
    m_area = 0;

    m_inmap   = index;
    m_outmap  = index;

    m_layername = name;

    MX_SetFill( a2dFill( wxColour( 0, 0, 0 ), a2dFILL_SOLID ) );
    MX_SetStroke( a2dStroke( wxColour( 255, 0, 0 ), 0, a2dSTROKE_SOLID ) );

    m_width = LAYERINFO_WIDTH;
    m_height = LAYERINFO_HEIGHT;

    m_pinclass = a2dPinClass::Standard;
    m_type = wxT( "SIGNAL" );
    m_side = wxT( "TOP" );
    m_canvasObjectCount = 0;
    m_canvasPreviousObjectCount = 0;
}

a2dLayerInfo::a2dLayerInfo( const a2dLayerInfo& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_layervisible = other.m_layervisible;
    m_layerselectable = other.m_layerselectable;
    m_layeravailable =  other.m_layeravailable;

    m_order = other.m_order;
    m_polarity = other.m_polarity;
    m_plated = other.m_plated;
    m_area = other.m_area;
    m_readlayer = other.m_readlayer;
    m_inmap = other.m_inmap;
    m_outmap = other.m_outmap;

    m_type = other.m_type;
    m_side = other.m_side;
    m_span = other.m_span;
    m_feature = other.m_feature;
    m_subFeature = other.m_subFeature;
    m_description = other.m_description;

    m_width = other.m_width;
    m_height = other.m_height;

    m_layername = other.m_layername;

    m_pinclass = other.m_pinclass;
    m_canvasObjectCount = 0;
    m_canvasPreviousObjectCount = 0;
}

a2dLayerInfo::~a2dLayerInfo()
{
}

void a2dLayerInfo::SetPending( bool pending )
{
    a2dCanvasObject::SetPending( pending );
}

wxString a2dLayerInfo::GetName() const
{
    return m_layername;
}

void a2dLayerInfo::SetName( const wxString& name )
{
    m_layername = name;
}

a2dObject* a2dLayerInfo::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dLayerInfo( *this, options, refs );
}

void a2dLayerInfo::OnPropertyChanged( a2dComEvent& event )
{
    a2dCanvasObject::OnPropertyChanged( event );
    if ( m_root )
    {
        //wxASSERT_MSG(m_root->GetLayerSetup()->GetLayerIndex()[m_layer] != wxNullLayerInfo,
        //    wxT("layer not defined in layer table"));

        a2dComEvent changedlayer( this, event.GetPropertyId(), sig_changedLayerInfo );
        m_root->ProcessEvent( changedlayer );
    }
}

#if wxART2D_USE_CVGIO
void a2dLayerInfo::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //out.WriteAttribute( wxT("layername"), m_layername ); //is written in base class
        out.WriteAttribute( wxT( "layervisible" ), m_layervisible );
        out.WriteAttribute( wxT( "layerselectable" ), m_layerselectable );
        out.WriteAttribute( wxT( "readlayer" ), m_readlayer );
        out.WriteAttribute( wxT( "inmap" ), m_inmap );
        out.WriteAttribute( wxT( "outmap" ), m_outmap );
        out.WriteAttribute( wxT( "order" ), m_order );
        if ( m_width != LAYERINFO_WIDTH )
        {
            out.WriteNewLine();
            out.WriteAttribute( wxT( "width" ), m_width );
        }
        if ( m_height != LAYERINFO_HEIGHT )
        {
            out.WriteNewLine();
            out.WriteAttribute( wxT( "height" ), m_height );
        }

        out.WriteAttribute( wxT( "description" ), m_description );
        out.WriteAttribute( wxT( "feature" ), m_feature );
        out.WriteAttribute( wxT( "subfeature" ), m_subFeature );
        out.WriteAttribute( wxT( "type" ), m_type );
        out.WriteAttribute( wxT( "polarity" ), m_polarity );
        out.WriteAttribute( wxT( "span" ), m_span.GetAsString() );
        out.WriteAttribute( wxT( "area" ), m_area );
    }
    else
    {
    }
}

void a2dLayerInfo::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_width = parser.GetAttributeValueDouble( wxT( "width" ), LAYERINFO_WIDTH );
        m_height = parser.GetAttributeValueDouble( wxT( "height" ), LAYERINFO_HEIGHT );

        m_layername = parser.GetAttributeValue( wxT( "name" ), m_layername );
        m_inmap = parser.GetAttributeValueUint16( wxT( "inmap" ), m_layer );
        m_outmap = parser.GetAttributeValueUint16( wxT( "outmap" ), m_layer );
        m_order = parser.GetAttributeValueUint16( wxT( "order" ), m_layer );
        m_readlayer = parser.GetAttributeValueBool( wxT( "readlayer" ), m_readlayer );

        m_description = parser.GetAttributeValue( wxT( "description" ), m_description );
        m_feature = parser.GetAttributeValue( wxT( "feature" ), m_feature );
        m_subFeature = parser.GetAttributeValue( wxT( "subfeature" ), m_subFeature );
        m_type = parser.GetAttributeValue( wxT( "type" ), m_type );
        m_polarity = parser.GetAttributeValueBool( wxT( "polarity" ), m_polarity );
        m_span = a2dSpan( parser.GetAttributeValue( wxT( "span" ), m_span.GetAsString() ) ); 
        m_area = parser.GetAttributeValueDouble( wxT( "area" ), m_area );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dLayerInfo::GetVisible()
{
    return m_layervisible;
}

bool a2dLayerInfo::GetSelectable()
{
    return m_layerselectable;
}

int a2dLayerInfo::GetOrder() const
{
    return m_order;
}

bool a2dLayerInfo::GetRead()
{
    return m_readlayer;
}

void a2dLayerInfo::SetVisible( bool status )
{
    m_layervisible = status;
}

void a2dLayerInfo::SetSelectable( bool status )
{
    m_layerselectable = status;
}

void a2dLayerInfo::SetOrder( wxUint16 order )
{
    m_order = order;
}

void a2dLayerInfo::SetRead( bool status )
{
    m_readlayer = status;
}

int a2dLayerInfo::GetInMapping()
{
    return m_inmap;
}

int a2dLayerInfo::GetOutMapping()
{
    return m_outmap;
}

void a2dLayerInfo::SetInMapping( wxUint16 layer )
{
    m_inmap = layer;
}
void a2dLayerInfo::SetOutMapping( wxUint16 layer )
{
    m_outmap = layer;
}

bool a2dLayerInfo::GetPixelStroke()
{
    return GetStroke().GetPixelStroke();
}

void a2dLayerInfo::SetPixelStroke( bool pixel )
{
    GetStroke().SetPixelStroke( pixel );
}

a2dBoundingBox a2dLayerInfo::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( m_width , m_height );
    return bbox;
}

void a2dLayerInfo::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    ic.GetDrawer2D()->DrawRoundedRectangle( 0, 0, m_width, m_height, 0 );
}

bool a2dLayerInfo::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = ic.GetTransformedHitMargin();

    double xmin = wxMin( 0 , m_width );
    double ymin = wxMin( 0 , m_height );
    double xmax = wxMax( 0 , m_width );
    double ymax = wxMax( 0 , m_height );

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, ic.GetTransformedHitMargin() + margin );

    return hitEvent.m_how.IsHit();
}

void a2dLayerInfo::SignalChange()
{
	if ( m_root )
	{
		a2dComEvent changed( this, sig_changedLayerInfo );
		m_root->ProcessEvent( changed );
	}
}


/******************************************************************************
    Layers settings
*******************************************************************************/

a2dPropertyIdBool* a2dLayers::PROPID_visibleAll = NULL;
a2dPropertyIdBool* a2dLayers::PROPID_readAll = NULL;
a2dPropertyIdBool* a2dLayers::PROPID_selectableAll = NULL;


const a2dSignal a2dLayers::sig_visibleAll = wxNewId();
const a2dSignal a2dLayers::sig_selectAll = wxNewId();
const a2dSignal a2dLayers::sig_outlineAll = wxNewId();
const a2dSignal a2dLayers::sig_readAll = wxNewId();
const a2dSignal a2dLayers::sig_availableAll = wxNewId();

INITIALIZE_PROPERTIES( a2dLayers, a2dCanvasObject )
{
    A2D_PROPID_D( a2dPropertyIdBool, visibleAll, false );
    A2D_PROPID_D( a2dPropertyIdBool, readAll, false );
    A2D_PROPID_D( a2dPropertyIdBool, selectableAll, false );
    return true;
}

/*
#include <functional>

template<class _Ty>
struct greater2 : std::binary_function<_Ty, _Ty, bool> {
    bool operator()(const _Ty& _X, const _Ty& _Y) const
        {return (_X > _Y); }
    };


bool operator < (const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y)
{
    a2dLayerInfo* firstc = wxStaticCast( x.Get(), a2dLayerInfo );
    a2dLayerInfo* secondc = wxStaticCast( y.Get(), a2dLayerInfo );

    switch ( a2dLayers::m_sortOn )
    {
    default:
    case 0:
        if ( firstc->GetOrder() < secondc->GetOrder() )
            return true;
    case 1:
        if ( firstc->GetOrder() > secondc->GetOrder() )
            return true;
    case 2:
        if ( firstc->GetLayer() < secondc->GetLayer() )
            return true;
    }
    return false;
}


namespace std{
struct sortgreater : public binary_function< a2dCanvasObjectPtr, a2dCanvasObjectPtr, bool>
    {
        bool operator()(const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y) const
        {
            a2dLayerInfo* firstc = wxStaticCast( x.Get(), a2dLayerInfo );
            a2dLayerInfo* secondc = wxStaticCast( y.Get(), a2dLayerInfo );

            if ( firstc->GetOrder() < secondc->GetOrder() )
                return true;
            return false;
        }
    };

}
*/

a2dLayers::a2dLayers(): a2dCanvasObject()
{
    m_indexed = false;
    UpdateIndexes();
}

a2dLayers::a2dLayers( const a2dLayers& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    wxASSERT( ( options & clone_deep ) == clone_deep );
    m_flags.m_pending = true;
    m_indexed = false;
    m_filename =  other.m_filename;

	UpdateIndexes();
}

a2dLayers::~a2dLayers()
{
}

a2dObject* a2dLayers::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dLayers( *this, options, refs );
}

void a2dLayers::SetPending( bool pending )
{
    a2dCanvasObject::SetPending( pending );

    //if ( m_root )
    {
        m_indexed = false;
    }
}

a2dLayerMapNr& a2dLayers::GetLayerIndex()
{
    if ( !m_indexed )
        UpdateIndexes();
    return m_layermapNr;
}

a2dLayerIndex& a2dLayers::GetLayerSort()
{
    if ( !m_indexed )
        UpdateIndexes();
    return m_layerArrayOnLayer;
}

a2dLayerIndex& a2dLayers::GetOrderSort()
{
    if ( !m_indexed )
        UpdateIndexes();
    return m_layerArrayOnOrder;
}

a2dLayerIndex& a2dLayers::GetReverseOrderSort()
{
    if ( !m_indexed )
        UpdateIndexes();
    return m_layerArrayOnReverseOrder;
}

bool a2dLayers::LoadLayers( const wxString& filename )
{
    wxString foundfile = filename;
#if wxART2D_USE_CVGIO

    ReleaseChildObjects();
	m_indexed = false;

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream stream( foundfile.mb_str() );
    if ( stream.fail() || stream.bad() )
#else
    wxFileInputStream stream( foundfile );
    if ( !stream.Ok() )
#endif
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Could not open layers settings file %s" ), foundfile.c_str() );
        return false;
    }

    a2dIOHandlerCVGIn cvgparser;
    if ( cvgparser.CanLoad( stream, this ) )
    {
        bool result = cvgparser.LoadLayers( stream, this );
        SetPending( true );
        if ( !result )
            a2dGeneralGlobals->ReportErrorF( a2dError_LoadLayers, _( "Cannot load layers from this file %s" ), foundfile.c_str() );

        AddDefaultLayers();
        UpdateIndexes();
        return result;
    }
    else
        a2dGeneralGlobals->ReportErrorF( a2dError_LoadLayers, _( "Cannot load layers from this file %s" ), foundfile.c_str() );
    return false;
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to load layers from a CVG file" ) );
    return false;
#endif //wxART2D_USE_CVGIO
}


bool a2dLayers::SaveLayers( const wxString& filename )
{
#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream store( filename.mb_str() );
    if ( store.fail() || store.bad() )
#else
    wxFileOutputStream storeUnbuf( filename );
    wxBufferedOutputStream store( storeUnbuf );
    if ( store.GetLastError() != wxSTREAM_NO_ERROR )
#endif
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), filename.c_str() );
        return false;
    }
    if ( m_childobjects != wxNullCanvasObjectList )
        m_childobjects->MakeUnique();

#if wxART2D_USE_CVGIO
    //assume it is the default CVG format.
    a2dIOHandlerCVGOut* handler = new a2dIOHandlerCVGOut();
    bool res = handler->SaveLayers( store, this );
    delete handler;
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to save object to a CVG file" ) );
    return false;
#endif //wxART2D_USE_CVGIO
    return res;
}

void a2dLayers::InitWith( int highlayer, bool blackWhite )
{
    CreateChildObjectList()->clear();

    if ( highlayer > wxMAXLAYER )
        highlayer = wxMAXLAYER;

    int j;
    for ( j = 0; j < highlayer; j++ )
    {
        wxString namelay;
        namelay.Printf( wxT( "Layer %d" ), j );
        a2dLayerInfo* defLayer = new a2dLayerInfo( j, namelay );
        if ( !blackWhite )
        {
            a2dFill fill = a2dFill( wxColour( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );
            a2dStroke stroke = a2dStroke( wxColour( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );
            defLayer->SetFill( fill );
            defLayer->SetStroke( stroke );
        }
        else
        {
            int graycol = rand() % 254;
            wxColour gray( graycol, graycol, graycol );
            a2dFill fill = a2dFill( gray );
            graycol = rand() % 254;
            wxColour grays( graycol, graycol, graycol );
            a2dStroke stroke = a2dStroke( grays );
            defLayer->SetFill( fill );
            defLayer->SetStroke( stroke );
        }
        Append( defLayer );
        m_indexed = false;
    }
    UpdateIndexes();
}

void a2dLayers::AddIfMissing( int layer )
{
    if ( !m_indexed )
        UpdateIndexes();
    if ( !m_layermapNr.count( layer ) )
    {
        bool layerfound = false;
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dLayerInfo* lobj = ( a2dLayerInfo* ) ( *iter ).Get();
            if ( lobj->m_layer == layer )
                layerfound = true;
        }
        wxString mes;
        mes.Printf( "layer %d already in child, wrong index", layer );
        wxASSERT_MSG( !layerfound, mes );
 
        wxString namelay;
        namelay.Printf( "Layer %d", layer );
        a2dLayerInfo* defLayer = new a2dLayerInfo( layer, namelay );

        a2dFill fill = a2dFill( wxColour( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );
        a2dStroke stroke = a2dStroke( wxColour( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );
        defLayer->SetFill( fill );
        defLayer->SetStroke( stroke );

        Append( defLayer );
        m_indexed = false;
        UpdateIndexes();
    }
}

wxUint16 a2dLayers::AddIfMissing( const wxString& layerName )
{
    if ( !m_indexed )
        UpdateIndexes();
    a2dLayerMapName::iterator it = m_layermapName.find( layerName );
    if (it != m_layermapName.end())
        return it->second->GetLayer();

    if ( !m_layermapNr.empty() )
    {
        a2dLayerMapNr::iterator it= m_layermapNr.end();
        it--;
        wxUint16 layermax = it->second->GetLayer();
        a2dLayerInfo* defLayer = new a2dLayerInfo( layermax+1, layerName );

        a2dFill fill = a2dFill( wxColour( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );
        a2dStroke stroke = a2dStroke( wxColour( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );
        defLayer->SetFill( fill );
        defLayer->SetStroke( stroke );

        Append( defLayer );
        m_indexed = false;
        UpdateIndexes();
        return layermax+1;
    }
    else
        AddIfMissing( 0 );
    return 0;
}

a2dLayerInfo* a2dLayers::operator[] ( int layer )
{
    AddIfMissing( layer );

    return m_layermapNr[layer];
}

/*
const a2dLayerInfo* a2dLayers::operator[] ( int indx ) const
{
    return const_cast<a2dLayerInfo*>( m_layermapNr[indx] );
}
*/

a2dLayerInfo* a2dLayers::GetLayerInfo( int layer )
{
    AddIfMissing( layer );

    return m_layermapNr[ layer ];
}

void a2dLayers::AddDefaultLayers()
{
    bool hasReservedDefault = false;
    bool hasReservedEdit = false;
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        wxUint16 maxorder = 0;
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dLayerInfo* lobj = ( a2dLayerInfo* ) ( *iter ).Get();
            wxUint16 layer = lobj->m_layer;
            if ( layer == wxLAYER_DEFAULT )
                hasReservedDefault = true;  
        }
    }

    // make sure  wxLAYER_DEFAULT is available
    if ( !hasReservedDefault )
    {
        a2dLayerInfo* info = new a2dLayerInfo( wxLAYER_DEFAULT, "default" );
        info->SetStroke( a2dStroke( wxColour( 0, 255, 255 ), 0, a2dSTROKE_SOLID ) );
        info->SetFill( a2dFill( wxColour( 0, 0, 0 ), a2dFILL_SOLID ) );
        Append( info );
        SetPending( true );
    }
}

void a2dLayers::UpdateIndexes()
{
    if ( !m_indexed )
    {
        m_indexed = true;

        m_layermapNr.clear(); 
        m_layermapName.clear();
        m_layerArrayOnLayer.clear();
        m_layerArrayOnOrder.clear();
        m_layerArrayOnReverseOrder.clear();

        if ( m_childobjects != wxNullCanvasObjectList )
        {
            wxUint16 maxorder = 0;
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dLayerInfo* lobj = ( a2dLayerInfo* ) ( *iter ).Get();
                wxUint16 layer = lobj->m_layer;

                wxASSERT_MSG( layer >= 0, wxT( "layer index must be >= 0" ) );

                a2dLayerMapNr::iterator it = m_layermapNr.find( layer );
                if (it != m_layermapNr.end() && it->second != lobj )
                {
                    wxString warn;
                    warn.Printf( wxT( "layer index %d defined twice" ), layer );
                    wxLogWarning( warn );
                }

                m_layerArrayOnLayer.push_back( lobj );
                m_layerArrayOnOrder.push_back( lobj );
                m_layerArrayOnReverseOrder.push_back( lobj );

                a2dLayerMapName::iterator itname = m_layermapName.find( lobj->GetName() );
                if ( itname != m_layermapName.end() && itname->second != lobj )
                {
                    a2dLayerInfo* layerobj = itname->second;
                    wxString warn;
                    warn.Printf( wxT( "layer name %s defined twice, was %d set to %d" ), lobj->GetName(), layerobj->m_layer, layer );
                    wxLogWarning( warn );
                }

                m_layermapNr[ layer ] = lobj; 
                m_layermapName[ lobj->GetName() ] = lobj; 
                //wxLogDebug( wxT(" found layer %d with name %s"), layer, lobj->GetName() );
            }

            std::sort( m_layerArrayOnLayer.begin(), m_layerArrayOnLayer.end(), SortLayerNr() );
            std::sort( m_layerArrayOnOrder.begin(), m_layerArrayOnOrder.end(), SortLayerOrder() );
            std::sort( m_layerArrayOnReverseOrder.begin(), m_layerArrayOnReverseOrder.end(), SortLayerReverseOrder() );

            /* if you need to check it.
                        for ( j=0; j < GetHabitat()->GetMaxLayer(); j++ )
                        {
                            if ( m_layerArrayOnLayer[j] != wxNullLayerInfo )
                            {
                                wxString namelay;
                                namelay.Printf( wxT("index %d Layer %d %s"), j, m_layerArrayOnLayer[j]->GetLayer(), m_layerArrayOnLayer[j]->GetName()  );
                                wxLogDebug( namelay );
                            }
                            else
                                wxLogDebug( wxT("NULL") );
                            if ( m_layerArrayOnOrder[j] != wxNullLayerInfo )
                            {
                                wxString namelay;
                                namelay.Printf( wxT("index %d Order %d"), j, m_layerArrayOnOrder[j]->GetOrder()  );
                                wxLogDebug( namelay );
                            }
                            else
                                wxLogDebug( wxT("NULL") );
                            if ( m_layerArrayOnReverseOrder[j] != wxNullLayerInfo )
                            {
                                wxString namelay;
                                namelay.Printf( wxT("index %d reverse %d"), j, m_layerArrayOnReverseOrder[j]->GetOrder()  );
                                wxLogDebug( namelay );
                            }
                            else
                                wxLogDebug( wxT("NULL") );
                        }
            */

        }
    }
}

void a2dLayers::SetAvailable( a2dDrawing* drawing )
{
    SetAllLayersAvailable( false );
    //this one always available because editing handles are on it.
    AddIfMissing( wxLAYER_DEFAULT );
    m_layermapNr[wxLAYER_DEFAULT]->SetAvailable( true );

    a2dWalker_SetAvailable set( this );
    set.SetSkipNotRenderedInDrawing( true );
    set.Start( drawing );
}

void a2dLayers::SetAvailable( a2dLayers* other )
{
    if ( !m_indexed )
        UpdateIndexes();

    for ( a2dLayerMapNr::iterator it= other->m_layermapNr.begin(); it!= other->m_layermapNr.end(); ++it)
    {
        wxUint16 layer = it->second->GetLayer();
        AddIfMissing( layer );
        m_layermapNr[ layer ]->SetAvailable( it->second->GetAvailable() );
    }

    //this one always available because editing handles are on it.
    m_layermapNr[wxLAYER_DEFAULT]->SetAvailable( true );
}

bool a2dLayers::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_indexed )
    {
        UpdateIndexes();

        return true;
    }
    return false;
}

#if wxART2D_USE_CVGIO
void a2dLayers::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        // all layers that are added temporarely into the setup, are now checked,
        // and those available ( has objects on it in the document(s) ), will be set non temporary, and therefore saved.
        if ( m_childobjects != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dLayerInfo* lobj = ( a2dLayerInfo* ) ( *iter ).Get();
                if ( lobj->GetAvailable() )
                    PROPID_TemporaryObject->SetPropertyToObject( lobj, false );
            }
        }
    }

    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dLayers::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        a2dCanvasObjectList::iterator iter = m_childobjects->begin();
        while ( iter != m_childobjects->end() )
        {
            a2dLayerInfo* lobj = wxDynamicCast( ( *iter ).Get(), a2dLayerInfo ) ;
            if ( lobj->GetLayer() && lobj->GetLayer() < 0 )
            {
                //lobj->SetRelease( true );
                wxLogWarning(  wxT( "%s %d" ), _T( "Layer definition not in Range is removed: " ), lobj->GetLayer() );
                iter = m_childobjects->erase( iter );
            }
            else
			{
				lobj->SetParent( this );
                iter++;
			}
        }
    }
    m_indexed = false;
}
#endif //wxART2D_USE_CVGIO

wxString a2dLayers::GetName( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetName();
}

int a2dLayers::GetNumber( const wxString& name, bool createIfMissing )
{
    if ( !m_indexed )
        UpdateIndexes();
    a2dLayerMapName::iterator it = m_layermapName.find( name );
    if (it != m_layermapName.end())
        return it->second->GetLayer();

    if ( createIfMissing )
    {
        return AddIfMissing( name );
    }
    else
    {
        return -1;
    }
}

a2dStroke a2dLayers::GetStroke( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetStroke();
}

bool a2dLayers::GetVisible( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetVisible();
}

bool a2dLayers::GetSelectable( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetSelectable();
}

a2dFill a2dLayers::GetFill( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetFill();
}

int a2dLayers::GetOrder( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetOrder();
}

bool a2dLayers::GetRead( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetRead();
}

bool a2dLayers::GetPixelStroke( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetPixelStroke();
}

void a2dLayers::SetName( int layernumber, const wxString& name )
{
    int lay = GetNumber( name, false );
    if ( lay != -1 && lay != layernumber )
    {
        wxString warn;
        warn.Printf( wxT( "layer with name %s already defined with index %d, %d not renamed" ), name, lay, layernumber );
        wxLogWarning( warn );
    }
    else
    {
        AddIfMissing( layernumber );
        m_layermapNr[layernumber]->SetName( name );
    }
}

void a2dLayers::SetVisible( int layernumber, bool status )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetVisible( status );
}

void a2dLayers::SetAvailable( int layernumber, bool status )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetAvailable( status );
}

void a2dLayers::SetSelectable( int layernumber, bool status )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetSelectable( status );
}

void a2dLayers::SetFill( int layernumber, const a2dFill& fill )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetFill( fill  );
}

void a2dLayers::SetStroke( int layernumber, const a2dStroke& stroke )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetStroke( stroke );
}

void a2dLayers::SetPattern( int layernumber, const wxBitmap& stipple )
{
    AddIfMissing( layernumber );
    a2dFill fill = m_layermapNr[layernumber]->GetFill();
    a2dFill pfill = a2dFill( stipple );
    pfill.SetColour( fill.GetColour() );
    pfill.SetColour2( fill.GetColour2() );
    m_layermapNr[layernumber]->SetFill( pfill  );
}

void a2dLayers::SetOrder( int layernumber, int status )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetOrder( status );
}

void a2dLayers::SetRead( int layernumber, bool status )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetRead( status );
}

void a2dLayers::SetAll_Layers_Visible( bool onoff )
{
    if ( !m_indexed )
        UpdateIndexes();
    for ( a2dLayerMapNr::iterator it= m_layermapNr.begin(); it!= m_layermapNr.end(); ++it)
    {
        it->second->SetVisible( onoff );
    }

	if ( m_root )
	{
	    a2dComEvent changed( this, sig_visibleAll );
		m_root->ProcessEvent( changed );
	}
}

void a2dLayers::SetAll_Layers_Read( bool onoff )
{
    if ( !m_indexed )
        UpdateIndexes();
    for ( a2dLayerMapNr::iterator it= m_layermapNr.begin(); it!= m_layermapNr.end(); ++it)
    {
        it->second->SetRead( onoff );
    }
	if ( m_root )
	{
		a2dComEvent changed( this, sig_readAll );
		m_root->ProcessEvent( changed );
	}
}

void a2dLayers::SetAll_Layers_Selectable( bool onoff )
{
    if ( !m_indexed )
        UpdateIndexes();
    for ( a2dLayerMapNr::iterator it= m_layermapNr.begin(); it!= m_layermapNr.end(); ++it)
    {
        it->second->SetSelectable( onoff );
    }

	if ( m_root )
	{
	    a2dComEvent changed( this, sig_selectAll );
		m_root->ProcessEvent( changed );
	}
}

void a2dLayers::SetAll_Layers_Outline( bool onoff )
{
    if ( !m_indexed )
        UpdateIndexes();
    for ( a2dLayerMapNr::iterator it= m_layermapNr.begin(); it!= m_layermapNr.end(); ++it)
    {
        a2dFill fill = it->second->GetFill();
        fill.SetFilling( !onoff );
        it->second->SetFill( fill );
    }

	if ( m_root )
	{
	    a2dComEvent changed( this, sig_outlineAll );
		m_root->ProcessEvent( changed );
	}
}

void a2dLayers::SetAllLayersAvailable( bool onoff )
{
    if ( !m_indexed )
        UpdateIndexes();
    for ( a2dLayerMapNr::iterator it= m_layermapNr.begin(); it!= m_layermapNr.end(); ++it)
    {
        it->second->SetAvailable( onoff );
    }

    if ( m_root )
	{
		a2dComEvent changed( this, sig_availableAll );
		m_root->ProcessEvent( changed );
	}
}

int  a2dLayers::GetInMapping( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetInMapping();
}

int  a2dLayers::GetOutMapping( int layernumber )
{
    AddIfMissing( layernumber );
    return m_layermapNr[layernumber]->GetOutMapping();
}

void a2dLayers::SetInMapping( int layernumber, wxUint16 layer )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetInMapping( layer );
}

void a2dLayers::SetOutMapping( int layernumber, wxUint16 layer )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetOutMapping( layer );
}

void a2dLayers::SetPixelStroke( int layernumber, bool status )
{
    AddIfMissing( layernumber );
    m_layermapNr[layernumber]->SetPixelStroke( status );
}

