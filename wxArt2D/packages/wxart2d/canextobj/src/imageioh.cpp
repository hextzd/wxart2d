/*! \file canextobj/src/imageioh.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: imageioh.cpp,v 1.27 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canextobj/imageioh.h"
#include "wx/editor/candoc.h"


//----------------------------------------------------------------------------
// a2dCanvasDocumentIOHandlerImageIn
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasDocumentIOHandlerImageIn, a2dIOHandlerStrIn )

a2dCanvasDocumentIOHandlerImageIn::a2dCanvasDocumentIOHandlerImageIn( wxBitmapType type )
{
    m_type = type;
    m_docClassInfo = &a2dCanvasDocument::ms_classInfo;
    m_drawingPart = NULL;
    m_doc = NULL;
    m_xImage = 0;
    m_yImage = 0;
    m_ppi = 72;
    m_scale = -1;
    m_wImage = 1000;
    m_hImage = 1000;
    m_preserveAspectRatio = true;
}

a2dCanvasDocumentIOHandlerImageIn::~a2dCanvasDocumentIOHandlerImageIn()
{
}

wxBitmapType a2dCanvasDocumentIOHandlerImageIn::DetectImageType()
{
    wxASSERT_MSG( m_streami , wxT( "m_stream not set" ) );

    wxImageHandler* handlerfound = NULL;
    wxList::compatibility_iterator node = wxImage::GetHandlers().GetFirst();
    while ( node )
    {
        SeekI( 0 );
        wxImageHandler* handler = ( wxImageHandler* )node->GetData();
#if wxUSE_STD_IOSTREAM

        wxMemoryBuffer mem;
        const int BUFFSIZE = 10000;
        char* buf = new char[BUFFSIZE]; // a buffer to temporarily park the data
        size_t len = 0;
        while( len = Read( buf,  BUFFSIZE ) )
            mem.AppendData( buf, len );
        delete [] buf;
        buf = NULL;

        wxMemoryInputStream localStream( mem.GetData(), mem.GetDataLen() );

        if ( handler->CanRead( localStream ) )
#else
        if ( handler->CanRead( *m_streami ) )
#endif
        {
            handlerfound = handler;
            break;
        }
        node = node->GetNext();
    }
    SeekI( 0 );
    if ( handlerfound )
        return ( wxBitmapType ) handlerfound->GetType();
    return wxBITMAP_TYPE_ANY;
}

bool a2dCanvasDocumentIOHandlerImageIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{
    if ( obj && !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;

    if ( docClassInfo && m_docClassInfo && !docClassInfo->IsKindOf( m_docClassInfo ) )
        return false;

    m_streami = &stream;

    wxImageHandler* handlerfound = NULL;
    if ( m_type == wxBITMAP_TYPE_ANY )
    {
        long type = DetectImageType();
        m_streami = NULL;
        if ( type == wxBITMAP_TYPE_ANY )
            return false;
    }
    else
    {
        long type = DetectImageType();
        m_streami = NULL;
        if ( m_type != type )
            return false;
        if ( !wxImage::FindHandler( m_type ) )
            return false;
    }

    m_streami = NULL;
    return true;
}

bool a2dCanvasDocumentIOHandlerImageIn::Load( a2dDocumentInputStream& stream , wxObject* obj )
{
    m_streami = &stream;
    m_doc = wxStaticCast( obj, a2dCanvasDocument );

    bool oke = true;

    wxImageHandler* handler = NULL;
    if ( m_type == wxBITMAP_TYPE_ANY )
    {
        wxBitmapType type = DetectImageType();
        if ( type == wxBITMAP_TYPE_ANY )
            return false;
        handler = wxImage::FindHandler( type );
    }
    else
        handler = wxImage::FindHandler( m_type );

    if ( !handler )
        return false;

    try
    {
        InitializeLoad();

        wxImage image;

#if !wxUSE_STD_IOSTREAM
        if ( handler->LoadFile( &image, stream ) )
        {
            a2dSmrtPtr<a2dImage> canvas_image = new a2dImage( image, m_xImage, m_yImage, image.GetWidth() * m_scale, image.GetHeight() * m_scale ); 
            canvas_image->SetFill( *a2dTRANSPARENT_FILL );
            canvas_image->SetStroke( *a2dTRANSPARENT_STROKE );
            if ( !m_showObject )
                m_doc->GetDrawing()->GetRootObject()->Append( canvas_image );
            else
                m_showObject->Append( canvas_image );
            return true;
        }
#else


        wxMemoryBuffer mem;
        const int BUFFSIZE = 10000;
        char* buf = new char[BUFFSIZE]; // a buffer to temporarily park the data
        size_t len = 0;
        while( len = Read( buf,  BUFFSIZE ) )
            mem.AppendData( buf, len );
        delete [] buf;
        buf = NULL;

        wxMemoryInputStream localStream( mem.GetData(), mem.GetDataLen() );
        if ( handler->LoadFile( &image, localStream ) )        
        {
            double w = image.GetWidth(); 
            double h = image.GetHeight();
            a2dSmrtPtr<a2dImage> canvas_image;
            if ( m_ppi != -1 )
            {
                // get the resolution from the image options or fall back to default resolution
                // if not specified
                double resX, resY;
                switch ( GetResolutionFromOptions(image, &resX, &resY) )
                {
                    default:
                        wxFAIL_MSG( wxT("unexpected image resolution units") );
                        wxFALLTHROUGH;

                    case wxIMAGE_RESOLUTION_NONE:
                        // use the default resolution (e.g. 72 ppi)
                        resX =
                        resY = m_ppi;
                        wxFALLTHROUGH;// fall through to convert it to correct units

                    case wxIMAGE_RESOLUTION_INCHES:
                        // convert resolution in inches to resolution in centimeters
                        resX = 10*mm2inches*resX;
                        resY = 10*mm2inches*resY;
                        wxFALLTHROUGH;// fall through to convert it to resolution in meters

                    case wxIMAGE_RESOLUTION_CM:
                        // convert resolution in centimeters to resolution in meters
                        resX *= 100;
                        resY *= 100;
                        break;
                }

                double scale = m_doc->GetDrawing()->GetUnitsScale();
                double toUnits;
                wxString units;
                if ( m_drawingPart )
                    units = m_drawingPart->GetDrawing()->GetUnits();
                else
                    units = m_doc->GetDrawing()->GetUnits();
                if ( units == "non" ) //not defined units, we assume mm 
                    units = "mm";
                a2dDoMu::GetMultiplierFromString( units, toUnits );

                w = image.GetWidth() / (resX * toUnits * scale);
                h = image.GetHeight() / (resY * toUnits * scale);
                if ( m_drawingPart )
                {
                    m_xImage = m_drawingPart->GetDrawer2D()->GetVisibleMinX() + w/2.0;
                    m_yImage = m_drawingPart->GetDrawer2D()->GetVisibleMinY() + h/2.0;
                }               
                canvas_image = new a2dImage( image, m_xImage, m_yImage, w, h );
            }
            else if ( m_scale == -1 )
            {
                if ( m_wImage/image.GetWidth() > m_hImage/image.GetHeight() )
                {
                    h = m_wImage * h/w;
                    w = m_wImage;
                }
                else
                {
                    w = m_hImage * w/h;
                    h = m_hImage;
                }

                canvas_image = new a2dImage( image, m_xImage, m_yImage, w, h ); 
            }
            else
                canvas_image = new a2dImage( image, m_xImage, m_yImage, image.GetWidth() * m_scale, image.GetHeight() * m_scale ); 

            if ( m_preserveAspectRatio )
                a2dCanvasObject::PROPID_preserveAspectRatio->SetPropertyToObject( canvas_image, true );
            canvas_image->SetFill( *a2dTRANSPARENT_FILL );
            canvas_image->SetStroke( *a2dTRANSPARENT_STROKE );

            if ( !m_drawingPart )
                m_doc->GetDrawing()->GetRootObject()->Append( canvas_image );
            else
                m_drawingPart->GetShowObject()->Append( canvas_image );
            return true;
        }

#endif

    }
    catch ( ... )
    {
        oke = false;
        a2dGeneralGlobals->ReportErrorF( a2dError_NotSpecified, _( "error in Image file reading\n %s \n" ), _( "" ) );
    }

    ResetLoad();

    return oke;
}

wxImageResolution a2dCanvasDocumentIOHandlerImageIn::GetResolutionFromOptions(const wxImage& image, double *x, double *y)
{
    wxCHECK_MSG( x && y, wxIMAGE_RESOLUTION_NONE, wxT("NULL pointer") );

    if ( image.HasOption(wxIMAGE_OPTION_RESOLUTIONX) &&
         image.HasOption(wxIMAGE_OPTION_RESOLUTIONY) )
    {
        image.GetOption(wxIMAGE_OPTION_RESOLUTIONX).ToCDouble( x );
        image.GetOption(wxIMAGE_OPTION_RESOLUTIONY).ToCDouble( y );
    }
    else if ( image.HasOption(wxIMAGE_OPTION_RESOLUTION) )
    {
        image.GetOption(wxIMAGE_OPTION_RESOLUTION).ToCDouble( x );
        *y = *x;
    }
    else // no resolution options specified
    {
        *x =
        *y = 0;

        return wxIMAGE_RESOLUTION_NONE;
    }

    // get the resolution unit too
    int resUnit = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONUNIT);
    if ( !resUnit )
    {
        // this is the default
        resUnit = wxIMAGE_RESOLUTION_INCHES;
    }

    return (wxImageResolution)resUnit;
}

//----------------------------------------------------------------------------
// a2dCanvasDocumentIOHandlerImageOut
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasDocumentIOHandlerImageOut, a2dIOHandlerStrOut )

void a2dCanvasDocumentIOHandlerImageOut::SetViewRectangle( double minx, double miny, double maxx, double maxy )
{
    m_viewBox = a2dBoundingBox( minx, miny, maxx, maxy );
}

void a2dCanvasDocumentIOHandlerImageOut::SetBorder( wxUint16 border, bool percentual )
{
    m_border = border;
    m_borderpercentual = percentual;
    if ( m_borderpercentual )
    {
        wxASSERT_MSG( m_border >= 0 && m_border <= 100 , wxT( "percentage must be >=0 and <= 100" ) );
    }
}

a2dCanvasDocumentIOHandlerImageOut::a2dCanvasDocumentIOHandlerImageOut( wxBitmapType type )
{
    m_border = 0;
    m_borderpercentual = true;
    m_typeDetectedFromFilename = m_type = type;
    m_imageSize = wxSize( 0, 0 );
    m_showObject = NULL;
    m_yaxis = true;
    m_drawer2D = NULL;
    m_maxWidthHeight = 1000;
}

a2dCanvasDocumentIOHandlerImageOut::~a2dCanvasDocumentIOHandlerImageOut()
{
    if ( m_drawer2D )
        delete m_drawer2D;
}

void a2dCanvasDocumentIOHandlerImageOut::SetDrawer2D( a2dDrawer2D* drawer2d )
{
    if ( m_drawer2D )
        delete m_drawer2D;

    m_drawer2D = drawer2d;
    if ( m_drawer2D )
    {
        m_drawer2D->SetBufferSize( m_imageSize.GetWidth(), m_imageSize.GetHeight() );
    }
}

bool a2dCanvasDocumentIOHandlerImageOut::CanSave( const wxObject* obj )
{
    // if type is ANY, it will find the one liking the extension
    wxImageHandler* handler = NULL;
    handler = wxImage::FindHandler( m_type );
    if ( !handler )
        return false;

    m_typeDetectedFromFilename = m_type;
    return true;
}

bool a2dCanvasDocumentIOHandlerImageOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    m_doc = wxStaticCast( doc, a2dCanvasDocument );
    m_streamo = &stream;
    //int isOK = stream.IsOk ();

    InitializeSave();

    bool res = false;

    wxImage image = RenderImage( m_doc );
	//image.SaveFile( "c:/soft/aaaaaaaaaaap.png", wxBITMAP_TYPE_PNG );

    wxImageHandler* handler = wxImage::FindHandler( wxBITMAP_TYPE_ANY == m_type ? m_typeDetectedFromFilename : m_type );
    if ( handler )
    {
#if wxUSE_STD_IOSTREAM
        wxMemoryOutputStream streamimage;
        if (!image.SaveFile( streamimage, wxBITMAP_TYPE_ANY == m_type ? m_typeDetectedFromFilename : m_type))
            return false;

        const int BUFFSIZE = 100;
        char buf[BUFFSIZE]; // a buffer to temporarily park the data

        streamimage.SeekO(0);
        wxStreamBuffer* theBuffer = streamimage.GetOutputStreamBuffer();
        while ( theBuffer->Read( (void*) buf, BUFFSIZE ) > 0 )
            m_streamo->write( buf, BUFFSIZE );

        res = true;

#else //wxUSE_STD_IOSTREAM
        res = image.SaveFile( *m_streamo, m_type );
#endif //wxUSE_STD_IOSTREAM
    }
    ResetSave();

    return res;
}

wxImage a2dCanvasDocumentIOHandlerImageOut::RenderImage( a2dCanvasDocument* document )
{
    m_doc = document;

    if ( !m_showObject )
    {
        m_showObject = m_doc->GetDrawing()->GetRootObject();
    }

    if ( !m_viewBox.GetValid() )
    {
        // view will be based on showobject its boundingbox
        m_viewBox = m_showObject->GetBbox();
        a2dAffineMatrix cworld = m_showObject->GetTransformMatrix();
        cworld.Invert();
        m_viewBox.MapBbox( cworld );
    }

    double w = m_viewBox.GetWidth();
    double h = m_viewBox.GetHeight();

    if ( w == 0 )
        w = 1000;
    if ( h == 0 )
        h = 1000;

    //virtual coordinates box its miminum X ( calculated to
    double virt_minX;
    //virtual coordinates box its miminum Y
    double virt_minY;
    //user units per pixel in x
    double xpp;
    //user units per pixel in y
    double ypp;


    if ( m_imageSize.GetWidth() == 0 || m_imageSize.GetHeight() == 0 )
    {
        m_maxWidthHeight = wxGetNumberFromUser( _( "Give Maximum pixel size in width and height:" ), _( "Pixels:" ), _( "Pixel" ), m_maxWidthHeight, 0, 50000 );
        m_imageSizeCalc = wxSize( m_maxWidthHeight, m_maxWidthHeight );

        //first calculate an image size that fits the maximum size.
        xpp = w / m_maxWidthHeight;
        ypp = h / m_maxWidthHeight;
        if ( xpp > ypp )
            m_imageSizeCalc = wxSize( m_maxWidthHeight, ( int ) ( h / xpp ) );
        else
            m_imageSizeCalc = wxSize( ( int ) ( w / ypp ), m_maxWidthHeight );
    }
    else
        m_imageSizeCalc = m_imageSize;

    // the rect to show in world coordinates is calculated,
    // such that it fits within the image.
    double uppx;
    double uppy;
    int clientw = m_imageSizeCalc.GetWidth();
    int clienth = m_imageSizeCalc.GetHeight();

    int border = m_border;
    if ( m_borderpercentual )
        border = ( int ) ( wxMin( clientw, clienth ) * m_border / 100.0 ) ;

    if ( clientw - border > 0 )
        clientw = clientw - border;

    if ( clienth - border > 0 )
        clienth = clienth - border;

    uppx = w / clientw;
    uppy = h / clienth;
    if ( uppx < uppy )
        uppx = uppy;

    double middlexworld = m_viewBox.GetMinX() + w / 2.0;
    double middleyworld = m_viewBox.GetMinY() + h / 2.0;
    virt_minX = middlexworld - ( clientw + border ) / 2.0 * uppx;
    virt_minY = middleyworld - ( clienth + border ) / 2.0 * uppx;
    xpp = uppx;
    ypp = uppx;

    if ( !m_drawer2D )
        m_drawer2D = new a2dMemDcDrawer( m_imageSizeCalc.GetWidth(), m_imageSizeCalc.GetHeight() );
    else
        m_drawer2D->SetBufferSize( m_imageSizeCalc.GetWidth(), m_imageSizeCalc.GetHeight() );
    m_drawer2D->SetMappingDeviceRect( 0, 0, m_imageSizeCalc.GetWidth(), m_imageSizeCalc.GetHeight() );

    a2dSmrtPtr<a2dDrawingPart> bitmapview = new a2dDrawingPart( m_imageSizeCalc.GetWidth(), m_imageSizeCalc.GetHeight(), m_drawer2D );
    bitmapview->SetShowObjectAndRender( m_showObject );
    bitmapview->GetDrawer2D()->SetYaxis( m_yaxis );

    // ensure all pending objects are reported
    bitmapview->Update( a2dCANVIEW_UPDATE_OLDNEW );
    bitmapview->GetDrawer2D()->SetMappingUpp( virt_minX , virt_minY, xpp, ypp );
    bitmapview->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );

    wxImage image = bitmapview->GetDrawer2D()->GetBuffer().ConvertToImage();

    bitmapview->SetDrawer2D( NULL, true );

    return image;
}


