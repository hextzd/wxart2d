/*! \file canvas/src/rectangle.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canimage.cpp,v 1.3 2009/07/17 16:03:34 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"
#include "wx/canvas/rectangle.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/canimage.h"

IMPLEMENT_DYNAMIC_CLASS( a2dImage, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dRgbaImage, a2dRectMM )

//----------------------------------------------------------------------------
// a2dImage
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dImage::PROPID_Width = NULL;
a2dPropertyIdDouble* a2dImage::PROPID_Height = NULL;

INITIALIZE_PROPERTIES( a2dImage, a2dCanvasObject )
{
    A2D_PROPID_GSI( a2dPropertyIdDouble, a2dImage, Width, 0 );
    A2D_PROPID_GSI( a2dPropertyIdDouble, a2dImage, Height, 0 );
    return true;
}

BEGIN_EVENT_TABLE( a2dImage, a2dCanvasObject )
    //EVT_CANVASHANDLE_MOUSE_EVENT( a2dImage::OnHandleEvent )
END_EVENT_TABLE()

a2dImage::a2dImage()
    : a2dCanvasObject()
{
    m_width = 10;
    m_height = 10;
    m_OpacityFactor = 255;

    m_image = wxImage( 10, 10 );

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImage::a2dImage( const wxImage& image, double xc, double yc, double w, double h )
    : a2dCanvasObject()
{
    m_lworld.Translate( xc, yc );

    if ( w == 0 )
        w = image.GetWidth();

    if ( h == 0 )
        h = image.GetHeight();

    m_width = w;
    m_height = h;
    m_OpacityFactor = 255;

    m_image = image;

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImage::a2dImage( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h )
    : a2dCanvasObject()
{
    wxBitmap bitmap;

    m_type = type;
    if ( !wxFileExists( imagefile ) )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "could not open file %s image file" ), imagefile.c_str() );
        m_image = wxImage( 100, 100 );
    }
    else
    {
        bitmap.LoadFile( imagefile, type );
        m_image =  bitmap.ConvertToImage();
    }

    if ( w == 0 )
        w = m_image.GetWidth();

    if ( h == 0 )
        h = m_image.GetHeight();

    m_filename = imagefile;

    m_lworld.Translate( xc, yc );

    m_width = w;
    m_height = h;
    m_OpacityFactor = 255;

    m_drawPatternOnTop = false;
}

a2dImage::a2dImage( a2dCanvasObject* torender, double xc, double yc, double w, double h, int imagew, int imageh )
    : a2dCanvasObject()
{
    m_lworld.Translate( xc, yc );

    m_width = w;
    m_height = h;
    m_OpacityFactor = 255;

    RenderObject( torender, imagew, imageh );

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImage::a2dImage( const a2dImage& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_OpacityFactor = other.m_OpacityFactor;
    m_image = other.m_image;
    m_filename = other.m_filename;
    m_type = other.m_type;
    m_drawPatternOnTop = other.m_drawPatternOnTop;
}

a2dImage::~a2dImage()
{
}

a2dCanvasObjectList* a2dImage::GetAsRectangles( const wxColour& col1,  const wxColour& col2, bool transform )
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();

    unsigned char* source_data = m_image.GetData();
    long w = m_image.GetWidth();
    long h = m_image.GetHeight();

    double rectw = m_width / m_image.GetWidth();
    double recth = m_height / m_image.GetHeight();

    for ( long y = 0; y < h; y++ )
    {
        long rowlenght = 0;
        long startrow = 0;
        for ( long x = 0; x < w; x++ )
        {
            unsigned char* pixel = source_data + ( y * w ) * 3 + x * 3;
            unsigned char red = pixel[0] ;
            unsigned char green = pixel[1] ;
            unsigned char blue = pixel[2] ;
            unsigned char alpha = 255  ;

            if ( red >= col1.Red() && red <= col2.Red() &&
                    green >= col1.Green() && green <= col2.Green() &&
                    blue >= col1.Blue() && blue <= col2.Blue()
               )
            {
                if ( !rowlenght )
                    startrow = x;
                rowlenght++;
            }
            else if ( rowlenght )
            {
                a2dRect* pixRect = new a2dRect( -m_width / 2.0 +  startrow * rectw,  m_height / 2.0 - y * recth, rectw * rowlenght, -recth );
                pixRect->Transform( pworld );
                pixRect->SetLayer( m_layer );
                pixRect->SetContourWidth( GetContourWidth() );
                pixRect->SetRoot( m_root, false );
                canpathlist->push_back( pixRect );
                rowlenght = 0;
            }

        }
        if ( rowlenght )
        {
            a2dRect* pixRect = new a2dRect( -m_width / 2.0 +  startrow * rectw,  m_height / 2.0 - y * recth, rectw * rowlenght, -recth );
            pixRect->Transform( pworld );
            pixRect->SetLayer( m_layer );
            pixRect->SetContourWidth( GetContourWidth() );
            pixRect->SetRoot( m_root, false );
            canpathlist->push_back( pixRect );
            rowlenght = 0;
        }
    }

    return canpathlist;
}

void a2dImage::RenderObject( a2dCanvasObject* torender, int imagew, int imageh )
{
/* TODO
    a2dSmrtPtr<a2dDrawing> doc = torender->GetRoot();
    bool hasDoc = true;
    if ( !doc )
    {
        doc = new a2dDrawing(); //will use default layers
        doc->Append( torender );
        hasDoc = false;
    }

    a2dSmrtPtr<a2dDrawingPart> drawer = new a2dDrawingPart( imagew, imageh );
    drawer->SetDocument( doc );
    drawer->SetShowObject( torender );
    doc->SetCanvasDocumentRecursive();
    //set some mapping in order to be able to calculate boundingboxes
    drawer->GetDrawer2D()->SetMappingWidthHeight( 0, 0, 1000, 1000 );
    drawer->GetDrawer2D()->SetMappingWidthHeight( torender->GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN ) );
    drawer->UpdateArea( 0, 0, imagew, imageh );

    m_image = drawer->GetDrawer2D()->GetBuffer().ConvertToImage();

    drawer->SetClosed();

    if ( !hasDoc )
    {
        torender->SetCanvasDocument( NULL );
    }
*/
}

void a2dImage::SetFilename( const wxString filename, wxBitmapType type, bool doread )
{
    m_filename = filename;
    m_type = type;

    SetPending( true );

    if ( doread )
    {
        if ( !wxFileExists( m_filename ) )
            a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "image file %s not found" ), m_filename.c_str() );
        wxCHECK_RET( m_image.LoadFile( m_filename, type ), wxT( "invalid image file" ) );
    }
}


void a2dImage::SetDrawPatternOnTop( bool drawPatternOnTop )
{

    // change only if really neccessary
    if ( m_drawPatternOnTop != drawPatternOnTop )
    {
        m_drawPatternOnTop = drawPatternOnTop;
        SetPending( true );
    }
}

bool a2dImage::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        PROPID_Allowrotation->SetPropertyToObject( this, true );
        PROPID_Allowskew->SetPropertyToObject( this, false );

        return a2dCanvasObject::DoStartEdit( editmode, editstyle );
    }

    return false;
}


a2dObject* a2dImage::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dImage( *this, options, refs );
};

a2dBoundingBox a2dImage::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;

    bbox.Expand(  -m_width / 2,  -m_height / 2 );
    bbox.Expand(   m_width / 2,   m_height / 2 );
    return bbox;
}

void a2dImage::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    // Draws rectangle / pattern on back if necessary
    if ( !m_drawPatternOnTop )
    {
        if ( !GetStroke().IsSameAs( *a2dTRANSPARENT_STROKE ) || !GetFill().IsSameAs( *a2dTRANSPARENT_FILL ) )
        {
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0, 0, m_width, m_height, 0 );
        }
    }


    if ( ic.GetDrawer2D()->GetDrawStyle() != a2dWIREFRAME_INVERT )
    {
        ic.GetDrawer2D()->DrawImage( m_image, 0, 0, m_width, m_height, m_OpacityFactor );
    }
    else
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0, 0 , m_width, m_height, 0 );


    // Draws rectangle / pattern on top if necessary
    if ( m_drawPatternOnTop )
    {
        if ( GetStroke() != *a2dTRANSPARENT_STROKE || GetFill() != *a2dTRANSPARENT_FILL )
        {
            //feature of shape, so why not use it.
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0, 0, m_width, m_height, 0 );
        }
    }
    if ( ( ic.GetDrawStyle() == RenderWIREFRAME_SELECT || ic.GetDrawStyle() == RenderWIREFRAME_SELECT_INVERT ) && m_flags.m_selected )
    {
        //layer pens for select layer are set already on higher levels
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0, 0 , m_width, m_height, 0 );
    }
}

bool a2dImage::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = ic.GetTransformedHitMargin();

    double xmax = fabs( m_width / 2 );
    double ymax = fabs( m_height / 2 );
    double xmin = -xmax;
    double ymin = -ymax;

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}

#if wxART2D_USE_CVGIO
void a2dImage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_filename = parser.GetAttributeValue( wxT( "filename" ) );
        m_width = parser.GetAttributeValueDouble( wxT( "width" ) );
        m_height = parser.GetAttributeValueDouble( wxT( "height" ) );
        m_type = ( wxBitmapType ) parser.GetAttributeValueLong( wxT( "type" ) );
        m_drawPatternOnTop = parser.GetAttributeValueBool( wxT( "patternontop" ) );
        m_OpacityFactor = parser.GetAttributeValueLong( wxT( "opacityfactor" ) );

        if ( m_filename.IsEmpty() )
        {
            // Embedded image file:
            // Loading the image data from the base64 encoded image file data in the document.

            bool loaded = false;
            wxString imageDataBase64 = parser.GetAttributeValue( wxT( "filedata" ) );
            if ( !imageDataBase64.IsEmpty() )
            {

                wxMemoryBuffer buf = wxBase64Decode( imageDataBase64 );
                if ( !buf.IsEmpty() )
                {
                    wxMemoryInputStream stream( buf.GetData(), buf.GetDataLen() );
                    loaded = m_image.LoadFile( stream, m_type );
                }
            }
            if ( !loaded )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "could not load embedded image %s" ), GetName() );
                m_image = wxImage( ( int ) m_width, ( int ) m_height );
            }
        }
        else
        {
            // External image file:
            // Loading the image data from an external image file.

            wxString fullfilepath = parser.GetFileName().GetPathWithSep() + m_filename;
            if ( !::wxFileExists( fullfilepath ) )
            {
                fullfilepath = a2dGlobals->GetImagePathList().FindValidPath( m_filename );
                if ( !::wxFileExists( fullfilepath ) )
                {
                    a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "could not open file %s for loading image" ), m_filename.c_str() );
                    return;
                }
            }
            m_image.LoadFile( fullfilepath, m_type );
        }
    }
    else
    {
    }
}

void a2dImage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_filename.IsEmpty() )
        {
            // Embedded image file:
            // Saving the data of the image file base64 encoded in the document.

            wxString imageDataBase64 = wxEmptyString;
            wxMemoryOutputStream stream;
            if ( m_image.SaveFile( stream, m_type ) )
            {
                const wxStreamBuffer* buf = stream.GetOutputStreamBuffer();
                imageDataBase64 = wxBase64Encode( buf->GetBufferStart(), buf->GetBufferSize() );
            }

            out.WriteAttribute( wxT( "filedata" ), imageDataBase64 );
        }
        else
        {
            // External image file:
            // Saving the path to the external image file containing the image data.

            wxString filename = a2dGlobals->GetImagePathList().FindValidPath( m_filename, false );
            if ( filename.IsEmpty() )
            {
                wxString fullfilepath = out.GetFileName().GetPathWithSep() + m_filename;
                m_image.SaveFile( fullfilepath, m_type );
            }

            out.WriteAttribute( wxT( "filename" ), m_filename );
        }
        out.WriteAttribute( wxT( "width" ), m_width );
        out.WriteAttribute( wxT( "height" ), m_height );
        out.WriteAttribute( wxT( "type" ), ( wxInt32 ) m_type );
        out.WriteAttribute( wxT( "patternontop" ), m_drawPatternOnTop );
        out.WriteAttribute( wxT( "opacityfactor" ), ( wxUint8 ) m_OpacityFactor );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dRgbaImage
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dRgbaImage, a2dRectMM )
END_EVENT_TABLE()

a2dRgbaImage::a2dRgbaImage()
    : a2dRectMM()
{
}

a2dRgbaImage::a2dRgbaImage(  double x, double y, wxImage& image, wxUint8 OpacityFactor )
    : a2dRectMM( x, y, image.GetWidth(), image.GetHeight() )
{
    m_glimage = a2dImageRGBA( image, OpacityFactor );
    m_OpacityFactor = OpacityFactor;
    m_drawPatternOnTop = false;
}

a2dRgbaImage::~a2dRgbaImage()
{
}

a2dRgbaImage::a2dRgbaImage( const a2dRgbaImage& other, CloneOptions options, a2dRefMap* refs )
    : a2dRectMM( other, options, refs )
{
    m_glimage = other.m_glimage;
    m_OpacityFactor = other.m_OpacityFactor;
    m_flip = other.m_flip;
    m_drawPatternOnTop = other.m_drawPatternOnTop;
}

a2dObject* a2dRgbaImage::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRgbaImage( *this, options, refs );
};

void a2dRgbaImage::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    // Draws rectangle / pattern on back if necessary
    if ( !m_drawPatternOnTop )
    {
        if ( !GetStroke().IsSameAs( *a2dTRANSPARENT_STROKE ) || !GetFill().IsSameAs( *a2dTRANSPARENT_FILL ) )
        {
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
        }
    }

    if ( ic.GetDrawer2D()->GetDrawStyle() != a2dWIREFRAME_INVERT )
    {
        ic.GetDrawer2D()->DrawImage( m_glimage, m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, m_OpacityFactor );
    }
    else
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );


    // Draws rectangle / pattern on top if necessary
    if ( m_drawPatternOnTop )
    {
        if ( GetStroke() != *a2dTRANSPARENT_STROKE || GetFill() != *a2dTRANSPARENT_FILL )
        {
            //feature of shape, so why not use it.
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
        }
    }
    if ( m_flags.m_selected )
    {
        //layer pens for select layer are set already on higher levels
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
    }
}

#if wxART2D_USE_CVGIO

void a2dRgbaImage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dRectMM::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dRgbaImage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dRectMM::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


IMPLEMENT_DYNAMIC_CLASS( a2dImageMM, a2dRectMM )

//----------------------------------------------------------------------------
// a2dImageMM
//----------------------------------------------------------------------------

a2dImageMM::a2dImageMM()
    : a2dRectMM( 0, 0, 10, 10 )
{
    m_image = wxImage( 10, 10 );

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImageMM::a2dImageMM(  double x, double y, wxImage& image, unsigned char alpha )
    : a2dRectMM( x, y, image.GetWidth(), image.GetHeight() )
{
    m_image = image;

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImageMM::a2dImageMM( const wxImage& image, double x, double y, double w, double h )
    : a2dRectMM( x, y, w, h )
{
    m_image = image;

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImageMM::a2dImageMM( const wxString& imagefile, wxBitmapType type, double x, double y, double w, double h )
    : a2dRectMM( x, y, w, h )
{
    wxBitmap bitmap;

    m_type = type;
    if ( !wxFileExists( imagefile ) )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "could not open file %s image file" ), imagefile.c_str() );
        m_image = wxImage( 100, 100 );
    }
    else
    {
        bitmap.LoadFile( imagefile, type );
        m_image =  bitmap.ConvertToImage();
    }

    m_filename = imagefile;

    m_drawPatternOnTop = false;
}

a2dImageMM::a2dImageMM( a2dCanvasObject* torender, double x, double y, double w, double h, int imagew, int imageh )
    : a2dRectMM( x, y, w, h )
{
    RenderObject( torender, imagew, imageh );

    m_filename = wxT( "" );

    m_type = wxBITMAP_TYPE_PNG;

    m_drawPatternOnTop = false;
}

a2dImageMM::a2dImageMM( const a2dImageMM& other, CloneOptions options, a2dRefMap* refs )
    : a2dRectMM( other, options, refs )
{
    m_image = other.m_image;
    m_filename = other.m_filename;
    m_type = other.m_type;
    m_drawPatternOnTop = other.m_drawPatternOnTop;
}

a2dImageMM::~a2dImageMM()
{
}

a2dCanvasObjectList* a2dImageMM::GetAsRectangles( const wxColour& col1,  const wxColour& col2, bool transform )
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();

    unsigned char* source_data = m_image.GetData();
    long w = m_image.GetWidth();
    long h = m_image.GetHeight();

    double rectw = GetWidth() / m_image.GetWidth();
    double recth = GetHeight() / m_image.GetHeight();

    for ( long y = 0; y < h; y++ )
    {
        long rowlenght = 0;
        long startrow = 0;
        for ( long x = 0; x < w; x++ )
        {
            unsigned char* pixel = source_data + ( y * w ) * 3 + x * 3;
            unsigned char red = pixel[0] ;
            unsigned char green = pixel[1] ;
            unsigned char blue = pixel[2] ;
            unsigned char alpha = 255  ;

            if ( red >= col1.Red() && red <= col2.Red() &&
                    green >= col1.Green() && green <= col2.Green() &&
                    blue >= col1.Blue() && blue <= col2.Blue()
               )
            {
                if ( !rowlenght )
                    startrow = x;
                rowlenght++;
            }
            else if ( rowlenght )
            {
                a2dRect* pixRect = new a2dRect( -GetWidth() / 2.0 +  startrow * rectw,  GetHeight() / 2.0 - y * recth, rectw * rowlenght, -recth );
                pixRect->Transform( pworld );
                pixRect->SetLayer( m_layer );
                pixRect->SetContourWidth( GetContourWidth() );
                pixRect->SetRoot( m_root, false );
                canpathlist->push_back( pixRect );
                rowlenght = 0;
            }

        }
        if ( rowlenght )
        {
            a2dRect* pixRect = new a2dRect( -GetWidth() / 2.0 +  startrow * rectw,  GetHeight() / 2.0 - y * recth, rectw * rowlenght, -recth );
            pixRect->Transform( pworld );
            pixRect->SetLayer( m_layer );
            pixRect->SetContourWidth( GetContourWidth() );
            pixRect->SetRoot( m_root, false );
            canpathlist->push_back( pixRect );
            rowlenght = 0;
        }
    }

    return canpathlist;
}

void a2dImageMM::RenderObject( a2dCanvasObject* torender, int imagew, int imageh )
{
/* TODO
    a2dSmrtPtr<a2dCanvasDocument> doc = torender->GetCanvasDocument();
    bool hasDoc = true;
    if ( !doc )
    {
        doc = new a2dCanvasDocument(); //will use default layers
        doc->Append( torender );
        hasDoc = false;
    }

    a2dSmrtPtr<a2dDrawingPart> drawer = new a2dDrawingPart( imagew, imageh );
    drawer->SetDocument( doc );
    drawer->SetShowObject( torender );
    doc->SetCanvasDocumentRecursive();
    //set some mapping in order to be able to calculate boundingboxes
    drawer->GetDrawer2D()->SetMappingWidthHeight( 0, 0, 1000, 1000 );
    drawer->GetDrawer2D()->SetMappingWidthHeight( torender->GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN ) );
    drawer->UpdateArea( 0, 0, imagew, imageh );

    m_image = drawer->GetDrawer2D()->GetBuffer().ConvertToImage();

    drawer->SetClosed();

    if ( !hasDoc )
    {
        torender->SetCanvasDocument( NULL );
    }
*/
}

void a2dImageMM::SetFilename( const wxString filename, wxBitmapType type, bool doread )
{
    m_filename = filename;
    m_type = type;

    SetPending( true );

    if ( doread )
    {
        if ( !wxFileExists( m_filename ) )
            a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "image file %s not found" ), m_filename.c_str() );
        wxCHECK_RET( m_image.LoadFile( m_filename, type ), wxT( "invalid image file" ) );
    }
}


void a2dImageMM::SetDrawPatternOnTop( bool drawPatternOnTop )
{

    // change only if really neccessary
    if ( m_drawPatternOnTop != drawPatternOnTop )
    {
        m_drawPatternOnTop = drawPatternOnTop;
        SetPending( true );
    }
}

bool a2dImageMM::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        PROPID_Allowrotation->SetPropertyToObject( this, true );
        PROPID_Allowskew->SetPropertyToObject( this, false );
        PROPID_preserveAspectRatio->SetPropertyToObject( this, true );

        return a2dRectMM::DoStartEdit( editmode, editstyle );
    }

    return false;
}


a2dObject* a2dImageMM::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dImageMM( *this, options, refs );
};

void a2dImageMM::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{

    // Draws rectangle / pattern on back if necessary
    if ( !m_drawPatternOnTop )
    {
        if ( m_flags.m_selected )
        {
            //layer pens for select layer are set already on higher levels
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
        }
        else if ( !GetStroke().IsSameAs( *a2dTRANSPARENT_STROKE ) || !GetFill().IsSameAs( *a2dTRANSPARENT_FILL ) )
        {
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
        }
    }

    if ( ic.GetDrawer2D()->GetDrawStyle() != a2dWIREFRAME_INVERT )
    {
        ic.GetDrawer2D()->DrawImage( m_image, m_minx + GetWidth() / 2.0, m_miny + GetHeight() / 2.0, m_maxx - m_minx, m_maxy - m_miny );
    }
    else
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );

    // Draws rectangle / pattern on top if necessary
    if ( m_drawPatternOnTop )
    {
        if ( m_flags.m_selected )
        {
            //layer pens for select layer are set already on higher levels
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
        }
        else if ( GetStroke() != *a2dTRANSPARENT_STROKE || GetFill() != *a2dTRANSPARENT_FILL )
        {
            //feature of shape, so why not use it.
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
        }
    }
}

bool a2dImageMM::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    hitEvent.m_how = a2dHit::stock_fill;
    return true;
}

#if wxART2D_USE_CVGIO
void a2dImageMM::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dRectMM::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_filename = parser.GetAttributeValue( wxT( "filename" ) );
        m_type = ( wxBitmapType ) parser.GetAttributeValueLong( wxT( "type" ) );
        m_drawPatternOnTop = parser.GetAttributeValueBool( wxT( "patternontop" ) );

        if ( !m_filename.IsEmpty() )
        {
            wxString fname = a2dGlobals->GetImagePathList().FindValidPath( m_filename );
            if ( !::wxFileExists( fname ) )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "could not open file %s for loading image" ), m_filename.c_str() );
                return;
            }
            m_image.LoadFile( fname, m_type );
        }
        else
        {
            m_image = wxImage( ( int ) GetWidth(), ( int ) GetHeight() );
        }
    }
    else
    {
    }
}

void a2dImageMM::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dRectMM::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_filename.IsEmpty() )
        {
            m_image.SaveFile( GetName(), m_type );
            out.WriteAttribute( wxT( "filename" ), GetName() );
        }
        else
        {
            out.WriteAttribute( wxT( "filename" ), m_filename );
        }
        out.WriteAttribute( wxT( "type" ), ( wxInt32 ) m_type );
        out.WriteAttribute( wxT( "patternontop" ), m_drawPatternOnTop );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

