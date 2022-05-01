/*! \file editor/samples/common/screen.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: screen.cpp,v 1.7 2008/09/03 18:40:29 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

#if wxART2D_USE_CURVES
#include "wx/curves/meta.h"
#endif //wxART2D_USE_CURVES

#include "wx/editor/editmod.h"
#include "screen.h"
#include "smile.xpm"

a2dPropertyIdBool* PROPID_visblePropBool = new a2dPropertyIdBool( wxT( "visblePropBool" ), a2dPropertyId::flag_none, false );
a2dPropertyIdString* PROPID_property1 = new a2dPropertyIdString( wxT( "property1" ), a2dPropertyId::flag_none, wxT( "" ) );

a2dPropertyIdBool* PROPID_bool = new a2dPropertyIdBool( wxT( "bool" ), a2dPropertyId::flag_none, false );
a2dPropertyIdDouble* PROPID_doubleprop = new a2dPropertyIdDouble( wxT( "doubleprop" ), a2dPropertyId::flag_none, false );

a2dPropertyIdString* PROPID_prop1 = new a2dPropertyIdString( wxT( "prop1" ), a2dPropertyId::flag_none, wxT( "" ) );
a2dPropertyIdString* PROPID_prop2 = new a2dPropertyIdString( wxT( "prop2" ), a2dPropertyId::flag_none, wxT( "" ) );
a2dPropertyIdDouble* PROPID_prop3 = new a2dPropertyIdDouble( wxT( "prop3" ), a2dPropertyId::flag_none, false );

a2dPropertyIdCanvasObject* PROPID_renderpre = new a2dPropertyIdCanvasObject( wxT( "renderpre" ), a2dPropertyId::flag_none, 0 );

a2dPropertyIdString* PROPID_polyline = new a2dPropertyIdString( wxT( "polyline" ), a2dPropertyId::flag_none, wxT( "" ) );
a2dPropertyIdString* PROPID_pythagoras = new a2dPropertyIdString( wxT( "pythagoras" ), a2dPropertyId::flag_none, wxT( "" ) );

a2dPropertyIdCanvasClipPath* PROPID_clip1 = new a2dPropertyIdCanvasClipPath( wxT( "clip1" ), a2dPropertyId::flag_none );
a2dPropertyIdCanvasClipPath* PROPID_clip2 = new a2dPropertyIdCanvasClipPath( wxT( "clip2" ), a2dPropertyId::flag_none );
a2dPropertyIdCanvasClipPath* PROPID_clip3 = new a2dPropertyIdCanvasClipPath( wxT( "clip3" ), a2dPropertyId::flag_none );
a2dPropertyIdCanvasClipPath* PROPID_clip4 = new a2dPropertyIdCanvasClipPath( wxT( "clip4" ), a2dPropertyId::flag_none );

INITIALIZE_PROPERTIES( RelativeRectangle, a2dRect )
{
    AddPropertyId( PROPID_visblePropBool );
    AddPropertyId( PROPID_property1 );
    AddPropertyId( PROPID_bool );
    AddPropertyId( PROPID_doubleprop );
    AddPropertyId( PROPID_prop1 ); 
    AddPropertyId( PROPID_prop2 );
    AddPropertyId( PROPID_prop3 );
    AddPropertyId( PROPID_renderpre );
    AddPropertyId( PROPID_polyline );
    AddPropertyId( PROPID_pythagoras );
    AddPropertyId( PROPID_clip1 );
    AddPropertyId( PROPID_clip2 );
    AddPropertyId( PROPID_clip3 );
    AddPropertyId( PROPID_clip4 );
    return true;
}

RelativeRectangle::RelativeRectangle( double w, double h , double radius ):
    a2dRect( 0, 0, w, h, radius )
{
    SetHitFlags( a2dCANOBJ_EVENT_NON );
}

void RelativeRectangle::DependencyPending( a2dWalkerIOHandler* handler )
{
    /*
       if ( ic.GetPreviousOrParent() && ic.GetPreviousOrParent()->GetPending() )
       {
           m_flags.m_pending = true;
           //rectangle has position in lower left corner, so need to subtract my height from lowest point of previous object
           m_lworld.Identity();
           m_lworld.Translate( ic.GetPreviousSibling()->GetBboxMinX()+100, ic.GetPreviousOrParent()->GetBboxMinY()-GetBboxHeight() );
       }*/
}

bool RelativeRectangle::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool res = a2dRect::DoUpdate( mode, childbox, clipbox, propbox );
    if ( res )
    {
        res = a2dRect::DoUpdate( mode, childbox, clipbox, propbox );
    }

    return res;
}

RelativeRectangle2::RelativeRectangle2( a2dCanvasObject* refx, wxRelativeTextDcPos posx, a2dCanvasObject* refy, wxRelativeTextDcPos posy, double w, double h , double radius ):
    a2dRect( 0, 0, w, h, radius )
{
    //SetHitFlags( a2dCANOBJ_EVENT_NON );

    m_posx = posx;
    m_posy = posy;

    m_refx = refx;
    m_refy = refy;
}

void RelativeRectangle2::DependencyPending( a2dWalkerIOHandler* handler )
{
    if ( m_refx->GetPending() || m_refy->GetPending() )
        m_flags.m_pending = true;
}

bool RelativeRectangle2::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool res = a2dRect::DoUpdate( mode, childbox, clipbox, propbox );
    if ( res )
    {
        m_lworld.Identity();
        if ( m_posx == wxCANVAS_RIGHT )
        {
            m_lworld.Translate( m_refx->GetBboxMaxX(), 0 );
        }
        else if ( m_posx == wxCANVAS_LEFT )
        {
            m_lworld.Translate( m_refx->GetBboxMinX(), 0 );
        }
        else if ( m_posx == wxCANVAS_X )
        {
            m_lworld.Translate( m_refx->GetPosX(), 0 );
        }


        if ( m_posy == wxCANVAS_TOP )
        {
            m_lworld.Translate( 0, m_refy->GetBboxMaxY() );
        }
        else if ( m_posy == wxCANVAS_BOTTOM )
        {
            m_lworld.Translate( 0, m_refy->GetBboxMinY() - 100 );
        }
        else if ( m_posy == wxCANVAS_Y )
        {
            m_lworld.Translate( 0, m_refy->GetPosY() - 150 );
        }

        GetBbox().SetValid( false );
        res = a2dRect::DoUpdate( mode, childbox, clipbox, propbox );
    }

    return res;
}

wxRelativeTextDc::wxRelativeTextDc( a2dCanvasObject* refx, wxRelativeTextDcPos posx, a2dCanvasObject* refy, wxRelativeTextDcPos posy, const wxString& text, const a2dFont& font ):
    a2dText( text, 0, 0, font )
{
    //SetHitFlags( a2dCANOBJ_EVENT_NON );

    m_posx = posx;
    m_posy = posy;

    m_refx = refx;
    m_refy = refy;
}

void wxRelativeTextDc::DependencyPending( a2dWalkerIOHandler* handler )
{

    if ( m_refx->GetPending() )
    {
        if ( m_posx == wxCANVAS_RIGHT )
        {
            a2dBoundingBox bbox = m_refx->GetUnTransformedBbox( true );
            bbox.MapBbox( m_refx->GetTransformMatrix() );

            if ( GetPosX() != bbox.GetMaxX() )
                m_flags.m_pending = true;
        }
        else if ( m_posx == wxCANVAS_LEFT )
        {
            a2dBoundingBox bbox = m_refx->GetUnTransformedBbox( true );
            bbox.MapBbox( m_refx->GetTransformMatrix() );

            if ( GetPosX() != bbox.GetMinX() )
                m_flags.m_pending = true;
        }
        else if ( m_posx == wxCANVAS_X )
        {
            if ( GetPosX() != m_refx->GetPosX() )
                m_flags.m_pending = true;
        }
    }

    if ( m_refy->GetPending() )
    {
        if ( m_posy == wxCANVAS_TOP )
        {
            a2dBoundingBox bbox = m_refy->GetUnTransformedBbox( true );
            bbox.MapBbox( m_refy->GetTransformMatrix() );

            if ( GetPosY() != bbox.GetMaxY() )
                m_flags.m_pending = true;
        }
        else if ( m_posy == wxCANVAS_BOTTOM )
        {
            a2dBoundingBox bbox = m_refy->GetUnTransformedBbox( true );
            bbox.MapBbox( m_refy->GetTransformMatrix() );

            if ( GetPosY() != bbox.GetMinY() - 100 )
                m_flags.m_pending = true;
        }
        else if ( m_posy == wxCANVAS_X )
        {
            if ( GetPosY() != m_refy->GetPosY() - 150 )
                m_flags.m_pending = true;
        }
    }

}

bool wxRelativeTextDc::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool res = a2dText::DoUpdate( mode, childbox, clipbox, propbox );
    if ( res )
    {
        m_lworld.Identity();
        if ( m_posx == wxCANVAS_RIGHT )
        {
            m_lworld.Translate( m_refx->GetBboxMaxX(), 0 );
        }
        else if ( m_posx == wxCANVAS_LEFT )
        {
            m_lworld.Translate( m_refx->GetBboxMinX(), 0 );
        }
        else if ( m_posx == wxCANVAS_X )
        {
            m_lworld.Translate( m_refx->GetPosX(), 0 );
        }


        if ( m_posy == wxCANVAS_TOP )
        {
            m_lworld.Translate( 0, m_refy->GetBboxMaxY() );
        }
        else if ( m_posy == wxCANVAS_BOTTOM )
        {
            m_lworld.Translate( 0, m_refy->GetBboxMinY() - 100 );
        }
        else if ( m_posy == wxCANVAS_Y )
        {
            m_lworld.Translate( 0, m_refy->GetPosY() - 150 );
        }

        m_bbox.SetValid( false );
        res = a2dText::DoUpdate( mode, childbox, clipbox, propbox );
    }

    return res;
}

BEGIN_EVENT_TABLE( AnimaterObject, a2dCanvasObject )
    //impossible because of wrong base claas in timer.
    EVT_CANVASOBJECT_TIMER( -1, AnimaterObject::OnTimer )
END_EVENT_TABLE()

AnimaterObject::AnimaterObject( double x, double y ): a2dCanvasObject()
{
    m_lworld.Translate( x, y );
    m_x = x;
    m_y = y;
    m_timer = new a2dTimer(  this );
    m_timer->Start( 10, false );
}


AnimaterObject::~AnimaterObject()
{
    delete m_timer;
}

void AnimaterObject::OnTimer( wxTimerEvent& WXUNUSED( event ) )
{

    SetPending( true );
    a2dAffineMatrix rotate;
    if ( GetBboxWidth() > 1700 || GetBboxHeight() > 1800 )
    {
        Update( a2dCanvasObject::updatemask_force );
        SetPending( true );
        m_lworld.Identity();
        SetPosXY( m_x, m_y );
    }
    else
    {
        rotate.Translate( 2, 8 );
        rotate.Rotate( 1 );
        rotate.Scale( 1.001, 1.001, 0, 0 );
    }

    Transform( rotate );
    Update( a2dCanvasObject::updatemask_force );
    SetPending( true );
}

BEGIN_EVENT_TABLE( MyCanvasObjectGroup, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( MyCanvasObjectGroup::OnMouseEvent )
END_EVENT_TABLE()

MyCanvasObjectGroup::MyCanvasObjectGroup( double x, double y ):
    a2dCanvasObject()
{
    m_lworld.Translate( x, y );

    m_text = _T( "should event bubble down in hierarchy?:" );
    m_answer = _T( "yes" );
}

a2dCanvasObject*    MyCanvasObjectGroup::Clone()
{
    return new MyCanvasObjectGroup( 0, 0 );
};

void MyCanvasObjectGroup::OnMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    if ( event.GetMouseEvent().LeftDown() )
    {
        wxString text = wxGetTextFromUser( m_text, _T( "event test" ) , m_answer );
        if ( text == _T( "no" ) )
        {
            m_text = _T( "should event bubble down in hierarchy this time?:" );
            m_answer = _T( "yes" );
        }
        else
        {
            m_text = _T( "should event bubble down in hierarchy again?:" );
            m_answer = _T( "no" );
            event.Skip();
        }
    }
    else
        event.Skip();
}


a2dPropertyIdString* MyCircle::PROPID_tipproperty = NULL;

INITIALIZE_PROPERTIES( MyCircle, a2dCircle )
{
    PROPID_tipproperty = new a2dPropertyIdString( wxT( "tipproperty" ), a2dPropertyId::flag_none, wxT( "" ) );
    AddPropertyId( PROPID_tipproperty );

    return true;
}

BEGIN_EVENT_TABLE( MyCircle, a2dCircle )
    EVT_CANVASOBJECT_MOUSE_EVENT( MyCircle::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT( MyCircle::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( MyCircle::OnLeaveObject )
    EVT_SLIDER( -1, MyCircle::OnSliderEvent )
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS( MyCircle, a2dCircle )

MyCircle::MyCircle()
{
}

MyCircle::MyCircle( double x, double y, double radius ):
    a2dCircle( x, y, radius )
{
    a2dStringProperty* ppp = new a2dStringProperty( PROPID_tipproperty , _T( "value of property" ) );

    a2dVisibleProperty* pppvis = new a2dVisibleProperty( this, PROPID_tipproperty, 0, radius + 12, true,
            a2dFont( 30.0, wxFONTFAMILY_SWISS ) );
    pppvis->SetVisible( false );

    pppvis->SetFill( wxColour( 219, 215, 6 ), a2dFILL_SOLID );
    ppp->SetToObject( this );
    Append( pppvis );
}

void MyCircle::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    wxPoint pos = event.GetMouseEvent().GetPosition();

    if ( event.GetMouseEvent().LeftDown() )
    {
        wxMessageBox( _T( " arrived in MyCircle " ) , _T( "event test" ) , wxOK, NULL, pos.x, pos.y );
        event.Skip();
    }
    else
        event.Skip();
}

void MyCircle::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
        a2dVisibleProperty* visprop = wxDynamicCast( Find( "", "a2dVisibleProperty" ), a2dVisibleProperty );
        wxASSERT ( visprop );
        visprop->SetVisible( true );
        SetPending( true );
        event.Skip();
}

void MyCircle::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
        a2dVisibleProperty* visprop = wxDynamicCast( Find( "", "a2dVisibleProperty" ), a2dVisibleProperty );
        wxASSERT ( visprop );
        visprop->SetVisible( false );
        SetPending( true );
        event.Skip();
}

void MyCircle::OnSliderEvent( wxCommandEvent& event )
{
    a2dWidgetSlider* original = wxStaticCast( event.GetEventObject(), a2dWidgetSlider );
    int pos = original->GetPosition();
    m_radius = pos;
    SetPending( true );
}

//---------------------------------------------------
// class MyEventHandler
//---------------------------------------------------

BEGIN_EVENT_TABLE( MyEventHandler, wxEvtHandler )
    EVT_CANVASOBJECT_MOUSE_EVENT( MyEventHandler::OnMouseEvent )
END_EVENT_TABLE()

MyEventHandler::MyEventHandler( wxFrame* frame )
{
    m_frame = frame;
}

void MyEventHandler::OnMouseEvent( a2dCanvasObjectMouseEvent& event )
{

    a2dCanvasObject* obj = ( a2dCanvasObject* )event.GetEventObject();

    wxURI url = obj->GetURI();

    //double x = event.GetX();
    //double y = event.GetY();

    if ( !url.GetPath().IsEmpty() )
        m_frame->SetStatusText( url.GetPath() , 0 );

    event.Skip(); //surely other eventhandlers should get a change, like the objects its own.
}

/*
void FillData0( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    datatree->Append( top );

    a2dRect* rec5565 = new a2dRect(  120, -900, 650, -180, 0);
    rec5565->SetFill(wxColour(30,0,124));
    rec5565->SetStroke(wxColour(252,154,252));
    top->Append( rec5565 );

    RelativeRectangle* rec55 = new RelativeRectangle(350,270,21);
    rec55->SetFill(wxColour(150,0,24));
    rec55->SetStroke(wxColour(252,154,252));
    top->Append( rec55 );

    RelativeRectangle* rec555 = new RelativeRectangle(350,270,21);
    rec555->SetFill(wxColour(70,0,24));
    rec555->SetStroke(wxColour(252,154,252));

    top->Append( rec555 );

    RelativeRectangle* rec5555 = new RelativeRectangle(650,70,21);
    rec5555->SetFill(wxColour(90,0,24));
    rec5555->SetStroke(wxColour(252,154,252));

    top->Append( rec5555 );

    wxRelativeTextDc* tt=new wxRelativeTextDc( rec5555, wxCANVAS_LEFT, rec5555, wxCANVAS_BOTTOM, _T("relative to previous text\nIn wxCanvasWorld Nou Nou\nEn nog andere onzin") ,
                      80, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    tt->SetFill( wxColour(10,200,24) );
    tt->SetStroke(wxColour(220,10,200));
    top->Append( tt );

    RelativeRectangle* rec7 = new RelativeRectangle(250,370,21);
    rec7->SetFill(wxColour(210,0,24));
    rec7->SetStroke(wxColour(252,154,252));
    top->Append( rec7 );


    RelativeRectangle* rec66 = new RelativeRectangle( 650,270,31);
    rec66->SetFill(wxColour(200,12,240));
    rec66->SetStroke(wxColour(252,54,252));
    top->Append( rec66 );

    *settop = top;
}
*/
/*
void FillData0( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    datatree->Append( top );


    wxBitmap bitmap;
    wxPathList pathList;
    pathList.Add(".") );
    pathList.Add("..") );

    wxString path = pathList.FindValidPath("smaller.png") );

    int i;
    for (i = 0; i < 2; i++)
    {
        int j;
        for (j = 0; j < 2; j++)
        {
            a2dImage* im = new a2dImage( path,wxBITMAP_TYPE_PNG, i*100,j*100,100,100 );
            top->Append( im );
        }
    }


    a2dCanvasObject* group1 = new a2dCanvasObject();

    group1->SetLayer( wxLAYER_ALL );
    group1->Translate(-130,-123);

    a2dRect* rec3 = new a2dRect(20,-20,50,170,30);
    rec3->SetFill(wxColour(0,120,240));
    rec3->SetStroke(wxColour(252,54,252 ),3.0);

    group1->Prepend( rec3 );

    a2dRect* rec4 = new a2dRect(120,-120,150,70,30);
    rec4->SetFill(wxColour(10,0,240));
    rec4->SetStroke(wxColour(252,154,252 ),3.0);
    group1->Prepend( rec4 );

    top->Append( group1 );


    a2dRect *rm = new a2dRect( 0,0,30,10 );
    a2dAffineMatrix mat( 100, 250, 10, 20, 60 );
    rm->SetTransformMatrix( mat );
    rm->SetFill(wxColour(0,255,255));
    top->Append( rm );

    a2dRect *rm2 = new a2dRect( 0,0,30,10 );
    rm2->SetTransformMatrix(  200, 650, 10, 20, -60 );
    rm2->SetFill(wxColour(0,235,225));
    top->Append( rm2 );

    a2dRect* rec6 = new a2dRect(20,-20,650,270,31);
    rec6->SetFill(wxColour(0,12,240));
    rec6->SetStroke(wxColour(252,54,252));

    top->Append( rec6 );


    a2dText* tts=new a2dText( _T("start text object which is not relative \nan other line \nand one more line ") , 120,-120,
                      30, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    tts->SetFill( wxColour(10,200,24) );
    tts->SetStroke(wxColour(220,10,200));
    top->Append( tts );


    //a2dRect* rec55 = new a2dRect(  120, -900, 650, -180, 0);
    //rec55->SetFill(wxColour(30,0,24));
    //rec55->SetStroke(wxColour(252,154,252));
    //top->Append( rec55 );


    RelativeRectangle2* rec554 = new RelativeRectangle2(tts, wxCANVAS_X, tts, wxCANVAS_BOTTOM, 650, -180, 60);
    rec554->SetFill(wxColour(30,0,24));
    rec554->SetStroke(wxColour(252,154,252));
    top->Append( rec554 );

    // a line of text
    wxRelativeTextDc* ttr=new wxRelativeTextDc( rec554, wxCANVAS_RIGHT, tts, wxCANVAS_BOTTOM, _T("selative to previous text bottom\n and an extra line, left") ,
                      30, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    ttr->SetFill( wxColour(10,200,24) );
    ttr->SetStroke(wxColour(220,10,200));
    top->Append( ttr );

    ttr=new wxRelativeTextDc( ttr, wxCANVAS_RIGHT, tts, wxCANVAS_BOTTOM, _T("relative to previous text right bottom") ,
                      20, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxNORMAL, wxNORMAL ) );
    ttr->SetFill( wxColour(10,200,240) );
    ttr->SetStroke(wxColour(220,10,100));
    top->Append( ttr );

    RelativeRectangle2* rec5 = new RelativeRectangle2(ttr, wxCANVAS_RIGHT, tts, wxCANVAS_BOTTOM, 650, 80, 0);
    rec5->SetFill(wxColour(30,0,24));
    rec5->SetStroke(wxColour(252,154,252));
    top->Append( rec5 );

    ttr=new wxRelativeTextDc( rec5, wxCANVAS_RIGHT, tts, wxCANVAS_BOTTOM, _T("relative to previous text right bottom") ,
                      35, wxFont( 18,wxNORMAL, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    ttr->SetFill( wxColour(10,100,240) );
    ttr->SetStroke(wxColour(220,50,100));
    top->Append( ttr );

    ttr=new wxRelativeTextDc( ttr, wxCANVAS_RIGHT, tts, wxCANVAS_BOTTOM, _T("relative bottom") ,
                      45, wxFont( 18,wxNORMAL, wxNORMAL, wxNORMAL ) );
    ttr->SetFill( wxColour(10,100,240) );
    ttr->SetStroke(wxColour(20,50,100));
    top->Append( ttr );


    // a line of text relative to last line
    wxRelativeTextDc* ttr2=new wxRelativeTextDc( tts, wxCANVAS_X, ttr, wxCANVAS_Y, _T("to previous text bottom, left") ,
                      50, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    ttr2->SetFill( wxColour(10,200,24) );
    ttr2->SetStroke(wxColour(220,10,200));
    top->Append( ttr2 );


    ttr2=new wxRelativeTextDc( ttr2, wxCANVAS_RIGHT, ttr, wxCANVAS_Y, _T("relative to text right bottom") ,
                      30, wxFont( 18,wxSCRIPT, wxNORMAL, wxBOLD ) );
    ttr2->SetFill( wxColour(10,200,240) );
    ttr2->SetStroke(wxColour(220,10,100));
    top->Append( ttr2 );

    ttr2=new wxRelativeTextDc( ttr2, wxCANVAS_RIGHT, ttr, wxCANVAS_Y, _T("relative t text right bottom") ,
                      45, wxFont( 18,wxSCRIPT, wxFONTSTYLE_ITALIC, wxBOLD ) );
    ttr2->SetFill( wxColour(10,100,240) );
    ttr2->SetStroke(wxColour(220,50,100));
    top->Append( ttr2 );

    // a line of text relative to last line
    wxRelativeTextDc* ttr3=new wxRelativeTextDc( tts, wxCANVAS_X, ttr2, wxCANVAS_Y, _T("relative to previous text BOTTOM, left") ,
                      30, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxNORMAL, wxNORMAL ) );
    ttr3->SetFill( *a2dTRANSPARENT_FILL );
    ttr3->SetStroke(wxColour(220,10,200));
    top->Append( ttr3 );

    ttr3=new wxRelativeTextDc( ttr3, wxCANVAS_RIGHT, ttr2, wxCANVAS_Y, _T("relative to previous text right bottom") ,
                      50, wxFont( 18,wxSCRIPT, wxSLANT, wxBOLD ) );
    ttr3->SetFill( *a2dTRANSPARENT_FILL );
    ttr3->SetStroke(wxColour(220,10,100));
    top->Append( ttr3 );

    ttr3=new wxRelativeTextDc( ttr3, wxCANVAS_RIGHT, ttr2, wxCANVAS_Y, _T("relative to previous text right bottom") ,
                      25, wxFont( 18,wxSCRIPT, wxSLANT, wxBOLD ) );
    ttr3->SetFill( *a2dTRANSPARENT_FILL );
    ttr3->SetStroke(wxColour(220,50,100));
    top->Append( ttr3 );

    wxRelativeTextDc* prevline = ttr3;


    a2dTextGroup* textgroup = new a2dTextGroup( 300, 100 );

    int k;
    for (k = 0; k < 5; k++)
    {

        a2dText* tts=new a2dText( _T("start text object which is not relative nan other line \nand one more line ") ,
                            120,-120 - k*40,
                          30, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
        tts->SetFill( wxColour(10,200,24) );
        tts->SetStroke(wxColour(220,10,200));
        textgroup->Append( tts );


        // a line of text relative to last line
        wxRelativeTextDc* ttr4=new wxRelativeTextDc( tts, wxCANVAS_X, prevline, wxCANVAS_Y, _T("relative to previous text BOTTOM, left") ,
                          70, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxNORMAL, wxNORMAL ) );
        ttr4->SetFill( *a2dTRANSPARENT_FILL );
        ttr4->SetStroke(wxColour(220,10,200));
        //textgroup->Append( ttr4 );
        top->Append( ttr4 );

        ttr4=new wxRelativeTextDc( ttr4, wxCANVAS_RIGHT, prevline, wxCANVAS_Y, _T("relative to previous text right bottom") ,
                          60, wxFont( 18,wxSCRIPT, wxSLANT, wxBOLD ) );
        ttr4->SetFill( *a2dTRANSPARENT_FILL );
        ttr4->SetStroke(wxColour(220,10,100));
        //textgroup->Append( ttr4 );
        top->Append( ttr4 );

        ttr4=new wxRelativeTextDc( ttr4, wxCANVAS_RIGHT, prevline, wxCANVAS_Y, _T("relative to previous text right bottom") ,
                          45, wxFont( 18,wxSCRIPT, wxSLANT, wxBOLD ) );
        ttr4->SetFill( *a2dTRANSPARENT_FILL );
        ttr4->SetStroke(wxColour(220,50,100));
        //textgroup->Append( ttr4 );
        top->Append( ttr4 );

        prevline = ttr4;

    }

    top->Append( textgroup );

    RelativeRectangle* rec55 = new RelativeRectangle(350,270,21);
    rec55->SetFill(wxColour(50,0,24));
    rec55->SetStroke(wxColour(252,154,252));

    top->Append( rec55 );

    RelativeRectangle* rec555 = new RelativeRectangle(350,270,21);
    rec555->SetFill(wxColour(70,0,24));
    rec555->SetStroke(wxColour(252,154,252));

    top->Append( rec555 );

    RelativeRectangle* rec5555 = new RelativeRectangle(650,70,21);
    rec5555->SetFill(wxColour(90,0,24));
    rec5555->SetStroke(wxColour(252,154,252));

    top->Append( rec5555 );

    wxRelativeTextDc* tt=new wxRelativeTextDc( rec5555, wxCANVAS_LEFT, rec5555, wxCANVAS_BOTTOM, _T("relative to previous text\nIn wxCanvasWorld Nou Nou\nEn nog andere onzin") ,
                      180, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    tt->SetFill( wxColour(10,200,24) );
    tt->SetStroke(wxColour(220,10,200));
    top->Append( tt );

    RelativeRectangle* rec7 = new RelativeRectangle(250,370,21);
    rec7->SetFill(wxColour(210,0,24));
    rec7->SetStroke(wxColour(252,154,252));
    top->Append( rec7 );


    a2dRect* rec66 = new a2dRect( 0, 0, 650,270,31);
    rec66->SetFill(wxColour(200,12,240));
    rec66->SetStroke(wxColour(252,54,252));
    rec7->Append( rec66 );


    wxRelativeTextDc* t1=new wxRelativeTextDc( rec66, wxCANVAS_LEFT,  rec66, wxCANVAS_BOTTOM, _T("relative to previous text\nAnd the next object also") ,
                      50, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    t1->SetFill( wxColour(10,200,124) );
    t1->SetStroke(wxColour(220,10,20));
    rec7->Append( t1 );

    wxRelativeTextDc* t2=new wxRelativeTextDc( t1, wxCANVAS_BOTTOM,  t1, wxCANVAS_BOTTOM, _T("relative to previous text\nAnd the next object also") ,
                      20, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    t2->SetFill( wxColour(10,200,124) );
    t2->SetStroke(wxColour(220,10,20));
    rec7->Append( t2 );


    wxRelativeTextDc* ttt=new wxRelativeTextDc( rec7,  wxCANVAS_LEFT, rec7, wxCANVAS_BOTTOM, _T("relative to previous text\nAnd the next object also\nlets see\nif this really works") ,
                      80, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    ttt->SetFill( wxColour(10,200,124) );
    ttt->SetStroke(wxColour(220,10,20));
    top->Append( ttt );

    *settop = top;
}
*/


/*clip test*/
/*
void FillData0( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    top->SetName("topdrawing") );
    a2dNameReference* topr = new a2dNameReference(0,0,top);
    datatree->Append( topr );

    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap );

    a2dCanvasObject* group1 = new a2dCanvasObject();

    group1->SetLayer( wxLAYER_DEFAULT );
    group1->Translate(-130,-123);



    a2dCanvasObject* clipgroup = new a2dCanvasObject();

    a2dPolygonL* clippoly= new a2dPolygonL();
    clippoly->SetFill( wxColour(100,17,55),a2dFILL_VERTICAL_HATCH );
    clippoly->SetStroke( *wxRED );
    clippoly->AddPoint(-130,-143);
    clippoly->AddPoint(-230,180);
    clippoly->AddPoint(90,220);
    clippoly->AddPoint(80,-123);
    clipgroup->Prepend( clippoly );

    a2dPolygonL* clippoly2= new a2dPolygonL();
    clippoly2->SetFill( *a2dTRANSPARENT_FILL );
    clippoly2->SetStroke( *wxGREEN );
    clippoly2->AddPoint(38,-43);
    clippoly2->AddPoint(48,230);
    clippoly2->AddPoint(135,250);
    clippoly2->AddPoint(130,-33);
    clippoly2->SetFill(wxColour(21,215,6),a2dFILL_HORIZONTAL_HATCH );
    clippoly2->SetStroke(wxColour(1,3,205 ),0.0);
    clipgroup->Prepend( clippoly2 );

    a2dClipPathProperty* clipprop = new a2dClipPathProperty( clipgroup );
    group1->SetOrAddProperty( clipprop );
    //group1->SetRotation(30);

    group1->SetName("group1") );

    a2dArrow* arrow2 = new  a2dArrow( 0,0,40,0,20 );
    arrow2->SetFill(wxColour(219,215,6));
    arrow2->SetStroke(wxColour(1,3,205 ),5.0);

    a2dEndsLine* line = new a2dEndsLine( 10,-85,50,230);
    line->SetStroke(wxColour(255,161,5),5.0,a2dSTROKE_DOT_DASH );
    line->SetEnd(arrow2);
    group1->Prepend( line );

    group1->Prepend( new a2dImage( image, 4,38,32,32 ) );

    MyCircle* ciree = new  MyCircle( 0,0, 30 );
    ciree->SetFill(wxColour(219,215,6),a2dFILL_SOLID );
    ciree->SetStroke(wxColour(1,3,205 ),3.0);
    ciree->SetURL("./index.html") );
    group1->Append(ciree);

    a2dRect* rec3 = new a2dRect(20,-20,50,170,30);
    rec3->SetFill(wxColour(0,120,240));
    rec3->SetStroke(wxColour(252,54,252 ),3.0);
    group1->Prepend( rec3 );

    a2dRect* rec2 = new a2dRect(0,0,104,52);
    rec2->SetFill(wxColour(0,240,240));
    rec2->SetStroke(wxColour(210,40,50 ),1.0);
    group1->Prepend( rec2 );

    //make another group of a2dCanvasObjects
    a2dCanvasObject* group2 = new a2dCanvasObject();
    group2->Prepend( new a2dImage( image, 60,38,52,32 ) );
    a2dRect* rr = new a2dRect(10,20,104,52,30);
    rr->SetFill(wxColour(10,17,255),a2dFILL_HORIZONTAL_HATCH );
    rr->SetStroke(wxColour(9,115,64 ),4.0);
    group2->Prepend( rr );


    //this a reference to group2 put into group1
    a2dCanvasObjectReference* m_subref = new a2dCanvasObjectReference(60,50, group2);
    m_subref->SetRotation(35);
    m_subref->SetRotation(0);
    group1->Prepend( m_subref );


    a2dPolygon* poly= new a2dPolygon();
    poly->AddPoint( -30, -20 );
    poly->AddPoint( 100, 0 );
    poly->AddPoint( 100, 100 );
    poly->AddPoint( 50, 150 );
    poly->AddPoint( 0, 100 );

    poly->SetFill(wxColour(100,17,255),a2dFILL_CROSSDIAG_HATCH );
    poly->SetStroke(wxColour(9,115,64 ),4.0);
    group1->Prepend( poly );

    a2dVertexList* pointlist = new a2dVertexList();
    a2dLineSegment* point = new a2dLineSegment(0,0);
    pointlist->Append(point);
    point = new a2dLineSegment(-300,100);
    pointlist->Append(point);
    point = new a2dLineSegment(-100,100);
    pointlist->Append(point);
    point = new a2dLineSegment(-100,0);
    pointlist->Append(point);
    point = new a2dLineSegment(-200,50);
    pointlist->Append(point);

    a2dPolygonL* poly15= new a2dPolygonL(pointlist, true);
    wxBitmap gs_bmp36_mono;


    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T("pat36.bmp") );
    gs_bmp36_mono.LoadFile(path, wxBITMAP_TYPE_BMP);
    wxMask* mask36 = new wxMask(gs_bmp36_mono, *wxBLACK);
    // associate a monochrome mask with this bitmap
    gs_bmp36_mono.SetMask(mask36);

    a2dFill aa = a2dFill(gs_bmp36_mono);
    aa->SetColour(wxColour(250,78,216 ));
    aa->SetColour2(*wxRED);
    aa->SetStyle(a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT);
    poly15->SetFill(aa);
    poly15->SetStroke(*wxRED,4.0);
    group1->Prepend( poly15 );


    top->Append( group1 );

    //now make two references to group1 into root group of the canvas
    a2dCanvasObjectReference* m_ref = new a2dCanvasObjectReference(350,-200, group1);
    m_ref->Scale(2, 3.2);
    m_ref->SetRotation(180);
    //m_ref->SetScale(2,2);
    top->Append( m_ref );


    a2dCanvasObjectReference* m_ref2 = new a2dCanvasObjectReference(80,450, group1);
    m_ref2->SetRotation(-35);
    top->Prepend( m_ref2 );

    //this a reference to group2 put into group1
    a2dCanvasObjectReference* subref2 = new a2dCanvasObjectReference(20,130, group2);
    subref2->SetRotation(15);
    //group3->Prepend( subref2 );
    top->Prepend( subref2 );

    *settop = top;
}
*/
/*
void FillData0( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    top->SetName( _T("topdrawing") );
    datatree->Append( top );


    MyCircle* ciree = new  MyCircle( 0,0, 330 );
    ciree->SetFill(wxColour(219,215,6),a2dFILL_SOLID );
    ciree->SetStroke(wxColour(1,3,205 ),3.0);
    ciree->SetURL( _T("./index.html") );
    top->Append(ciree);


    *settop = top;;

}
*/


void FillData0( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{ 
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );

    //! todo a2dNameReference gives in object top, problem for hits.
    //a2dNameReference* topr = new a2dNameReference(0,0,top);
    //datatree->Append( topr );
    datatree->GetDrawing()->Append( top );

    a2dText* tt = new a2dText( _T( "Hello rotated text\nIn wxCanvasWorld Nou Nou\nEn nog andere onzin" ) , -500, 750,
                               a2dFont( 180.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ), 25.0 );
    tt->SetFill( *a2dTRANSPARENT_FILL );
    tt->SetStroke( wxColour( 220, 10, 200 ) );
    top->Append( tt );

    a2dRect* tr = new a2dRect( -400, -150, 400, 200 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 20.0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
    tr->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( tr );

    a2dSLine* ll = new a2dSLine( -500, 0, 400, 0 );
    ll->SetStroke( wxColour( 252, 0, 252 ), 0 );
    top->Append( ll );

    a2dSLine* lo = new a2dSLine( 0, -500, 0, 400 );
    lo->SetStroke( wxColour( 2, 252, 252 ), 0 );
    top->Append( lo );

    int i;
    for ( i = 10; i < 100; i += 10 )
    {
        a2dSLine* l = new a2dSLine( 10, -15, i, 300 );
        l->SetStroke( wxColour( 252, 0, 252 ), 10 );
        top->Append( l );
    }


    wxBitmap bitmap;
    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "image1.png" ) );

    a2dImage* im = new a2dImage( path, wxBITMAP_TYPE_PNG, -300, -265, 382, 332 );

    im->SetStroke( wxColour( 29, 255, 244 ), 15.0 );
    im->SetFill( wxColour( 0, 117, 0 ), a2dFILL_CROSS_HATCH );
    im->SetLayer( wxLAYER_DEFAULT );
    top->Append( im );

    a2dCanvasObjectReference* ref1 = new a2dCanvasObjectReference( 1500, -500,  im );
//    ref1->SetScale(2,3.2);
    ref1->SetRotation( 35 );
    top->Append( ref1 );

    a2dImage* imp = new a2dImage( path, wxBITMAP_TYPE_PNG, -300, -665, 182, 72 );
    imp->SetStroke( wxColour( 29, 255, 244 ), 2 );
    imp->SetFill( wxColour( 0, 117, 0 ), a2dFILL_CROSS_HATCH );
    //imp->Rotate( 30 );
    top->Append( imp );

    a2dOrigin* origin = new a2dOrigin( 15, 15 );
    origin->SetStroke( *wxRED, 3 );
    top->Append( origin );

    a2dVertexList* pointlist = new a2dVertexList();
    a2dLineSegment* point = new a2dLineSegment( 0, 0 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -300, 100 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -100, 100 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -100, 0 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -200, 50 );
    pointlist->push_back( point );

    a2dPolygonL* poly15 = new a2dPolygonL( pointlist, true );
    wxBitmap gs_bmp36_mono;
    path = a2dGlobals->GetImagePathList().FindValidPath( _T( "pat36.bmp" ) );
    gs_bmp36_mono.LoadFile( path, wxBITMAP_TYPE_BMP );
    wxMask* mask36 = new wxMask( gs_bmp36_mono, *wxBLACK );
    // associate a monochrome mask with this bitmap
    gs_bmp36_mono.SetMask( mask36 );

    a2dFill  aa = a2dFill( gs_bmp36_mono );
    aa.SetColour( wxColour( 250, 78, 216 ) );
    aa.SetColour2( *wxRED );
    aa.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
    poly15->SetFill( aa );
    poly15->SetStroke( *wxRED, 4.0 );
    poly15->SetLayer( wxLAYER_DEFAULT + 3 );
    top->Append( poly15 );

    a2dRect* tr2 = new a2dRect( -350, -300, 400, 200 );
    tr2->SetStroke( wxColour( 9, 115, 64 ), 10.0, a2dSTROKE_SOLID );
    tr2->SetFill( wxColour( 100, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    tr2->SetLayer( wxLAYER_DEFAULT + 4 );
    top->Append( tr2 );

#define animate
#ifdef animate
    AnimaterObject* ani = new AnimaterObject( 100, 200 );

    a2dEllipse* e2 = new a2dEllipse( 0, 0, 400, 200 );
    e2->SetStroke( wxColour( 209, 1, 64 ), 10.0, a2dSTROKE_SOLID );
    e2->SetFill( wxColour( 100, 1, 5 ), a2dFILL_SOLID );
    ani->Append( e2 );

    a2dEllipticArc* e3 = new a2dEllipticArc( 30, 50, 340, 400, 40, 100 );
    e3->SetStroke( wxColour( 9, 115, 254 ), 10.0, a2dSTROKE_SOLID );
    e3->SetFill( wxColour( 10, 251, 5 ), a2dFILL_SOLID );
    ani->Append( e3 );

    top->Prepend( ani );

    AnimaterObject* ani2 = new AnimaterObject( 100, -200 );
    a2dEllipticArc* e4 = new a2dEllipticArc( 30, 50, 140, 400, 40, 200 );
    e4->SetStroke( wxColour( 9, 215, 254 ), 10.0, a2dSTROKE_SOLID );
    e4->SetFill( wxColour( 10, 51, 235 ), a2dFILL_SOLID );
    ani2->Append( e4 );

    top->Prepend( ani2 );
#endif
    *settop = top;;
}



void FillData1( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    datatree->CreateCommandProcessor();

    datatree->GetDrawing()->GetCanvasCommandProcessor()->SetParentObject();
    a2dCanvasGlobals->GetHabitat()->SetStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0 ) );


    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dFont sharedfont = a2dFont( 200.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC );
    a2dFill fillshared = a2dFill( wxColour( 100, 17, 155 ) );

    a2dText* p = new a2dText( wxT( "Hello, word" ), 0, 0, sharedfont );
    double dHeight = p->GetTextHeight();
    p->SetFill( fillshared );
    p->SetStroke( wxColour( 198, 3, 105 ), 0.0 );
    top->Append( p );

    a2dText* pp = new a2dText( wxT( "Hello, word" ), 0, 200, sharedfont );
    pp->SetFill( fillshared );
    pp->SetTextHeight( 50 );
    top->Append( pp );
    dHeight = p->GetTextHeight();

    datatree->GetDrawing()->GetCanvasCommandProcessor()->SetParentObject( top );
    a2dCanvasGlobals->GetHabitat()->SetFill( a2dFill( wxColour( 100, 17, 155 ) ) );
    a2dCanvasGlobals->GetHabitat()->SetStroke( *a2dTRANSPARENT_STROKE );

    {
        a2dArc* aca = new a2dArc( -500, -1450, 200, 30, 270 );
        aca->SetObjectTip( _T( "i am an Arc" ) , 20, 100, 140 );
        aca->SetSpecificFlags( true, a2dCanvasOFlags::SubEdit );

        a2dRect* tr = new a2dRect( 0, 0, 100, 40 );
        tr->SetFill( wxColour( 1, 227, 225 ), a2dFILL_HORIZONTAL_HATCH );
        tr->SetStroke( wxColour( 198, 3, 105 ), 0.0 );
        tr->SetSpecificFlags( true, a2dCanvasOFlags::SubEditAsChild );
        aca->Append( tr );

        a2dCircle* cirp = new  a2dCircle( -100, 0, 120 );
        cirp->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
        cirp->SetStroke( wxColour( 1, 3, 205 ), 30.0 );
        cirp->SetSpecificFlags( true, a2dCanvasOFlags::SubEdit | a2dCanvasOFlags::SubEditAsChild );
        aca->Append( cirp );

        a2dRect* tr2level = new a2dRect( 0, 0, 50, 20 );
        tr2level->SetFill( wxColour( 1, 227, 225 ), a2dFILL_VERTICAL_HATCH );
        tr2level->SetStroke( wxColour( 18, 3, 15 ), 0.0 );
        tr2level->SetSpecificFlags( true, a2dCanvasOFlags::SubEditAsChild );
        cirp->Append( tr2level );

        a2dText* tt = new a2dText( _T( "Label text" ) , 0, 0,
                                   a2dFont( 20.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
        tt->SetFill( wxColour( 180, 10, 20 ) );
        tt->SetStroke( *wxGREEN );
        tt->SetName( _T( "__LABEL__" ) );
        aca->Append( tt );

    }

    a2dCanvasGlobals->GetHabitat()->SetStroke( a2dStroke( wxColour( 210, 217, 15 ), 30.0 ) );
    a2dCircle* cir = new a2dCircle( -500, -1150, 370 );
    cir->SetObjectTip( _T( "i am a circle" ) , 20, 100, 240 );

    a2dText* tt = new a2dText( _T( "Label text is named __LABEL__" ) , 0, 0,
                               a2dFont( 40.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 180, 10, 20 ) );
    tt->SetStroke( *wxGREEN );
    tt->SetName( _T( "__LABEL__" ) );
    tt->SetChildOnlyTranslate( true );
    cir->Append( tt );

    a2dRect* tr2 = new a2dRect( 0, 230, 900, 30 );
    tr2->SetFill( wxColour( 221, 27, 225 ), a2dFILL_SOLID );
    tr2->SetStroke( wxColour( 18, 116, 15 ), 0.0 );
    tr2->SetSpecificFlags( true, a2dCanvasOFlags::SubEditAsChild );
    cir->Append( tr2 );

    a2dRect* tr3 = new a2dRect( 0, 200, 900, 30 );
    tr3->SetFill( wxColour( 1, 227, 225 ), a2dFILL_SOLID );
    tr3->SetStroke( wxColour( 18, 3, 15 ), 0.0 );
    tr3->SetSpecificFlags( true, a2dCanvasOFlags::SubEditAsChild );
    tr3->SetChildOnlyTranslate( true );
    cir->Append( tr3 );
    top->Append( cir );

    /*
        {
            a2dText* object=new a2dText( _T("Label text is named __LABEL__") , 0, 0,
                              40,0, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
            object->SetLayer( 0 );
            object->SetStroke( a2dBLACK_STROKE );
            object->SetFill( a2dTRANSPARENT_FILL );
            object->SetIsProperty( true );

            a2dCanvasObjectPtrProperty* res = cir->SetOrAddProperty( wxT("aap"), object, true );
            res->SetCanRender( true );
        }
    */

    a2dCanvasGlobals->GetHabitat()->SetFill( a2dFill( wxColour( 0, 17, 250 ) ) );
    a2dRect* rec = new a2dRect( -100, -150, 370, 500 );
    rec->SetObjectTip( _T( "i am a rectangle" ) , 20, 100, 100 );
    top->Append( rec );

    /*
    #if wxUSE_UNICODE
        commands2->Execute( _T("cvgstring   {<?xml version=\"1.0\" standalone=\"yes\"?> \
                <cvg classname=\"a2dCanvasDocument\"> \
                <o classname=\"a2dEllipticArc\" id=\"66\" \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 400.000000 500.000000 )\" \
                    width=\"540.000000\" height=\"400.000000\" start=\"-140.000000\" end=\"150.000000\"  > \
                </o>  </cvg>}") );
    #else
        commands2->Execute( _T("cvgstring   {<?xml version=\"1.0\" standalone=\"yes\"?> \
                <cvg classname=\"a2dCanvasDocument\"> \
                <o classname=\"a2dRect\" id=\"36\" \
                    layer=\"7\" \
                    flags=\" selectable vector visible draggable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 110.000000 350.000000 )\" \
                     width=\"400.000000\" height=\"140.000000\"  > \
                </o> \
                <o classname=\"a2dEllipse\" id=\"64\"  \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 800.000000 500.000000 )\" \
                     width=\"540.000000\" height=\"400.000000\"  >  \
                </o>\
                <o classname=\"a2dEllipticArc\" id=\"66\" \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 400.000000 500.000000 )\" \
                    width=\"540.000000\" height=\"400.000000\" start=\"-140.000000\" end=\"150.000000\"  > \
                </o>  </cvg>}") );
    #endif

    #if wxUSE_UNICODE
        //try invalid document, which will be finished by commandprocessor
        commands2->Execute( _T("cvgstring {\
                <o classname=\"a2dEllipticArc\" id=\"66\" \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 1600.000000 500.000000 )\" \
                    width=\"540.000000\" height=\"400.000000\" start=\"-140.000000\" end=\"150.000000\"  > \
                </o>}"));
    #else
        //try invalid document, which will be finished by commandprocessor
        commands2->Execute( _T("cvgstring \
                {<o classname=\"a2dRect\" id=\"36\" \
                    layer=\"7\" \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"translate( 410.000000 350.000000 ) rotate( 30.000000 )\" \
                     width=\"400.000000\" height=\"140.000000\"  > \
                </o> \
                <o classname=\"a2dEllipse\" id=\"64\"  \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 1200.000000 500.000000 )\" \
                     width=\"540.000000\" height=\"400.000000\"  >  \
                </o>\
                <o classname=\"a2dEllipticArc\" id=\"66\" \
                    flags=\" selectable vector visible dragable showassociated filled\" hitflags=\" visible\" \
                    transform=\"matrix( 1.000000 0.000000 0.000000 1.000000 1600.000000 500.000000 )\" \
                    width=\"540.000000\" height=\"400.000000\" start=\"-140.000000\" end=\"150.000000\"  > \
                </o>}"));
    #endif
    */
    a2dCircle* cirEe = new  a2dCircle( 1000, 0, 300, 200 );
    cirEe->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cirEe->SetStroke( wxColour( 1, 3, 205 ), 30.0 );
    top->Append( cirEe );

    cirEe->SetSpecificFlags( true,  a2dCanvasOFlags::SubEdit );

    a2dPropertyIdBool* propid = ( a2dPropertyIdBool* ) cirEe->HasPropertyId( "mypropX" );
    if ( !propid )
    {
        propid = new a2dPropertyIdBool( "mypropX", false, a2dPropertyId::flag_userDefined );
        cirEe->AddPropertyId( propid );
    }
    propid->SetPropertyToObject( cirEe, true );
    a2dVisibleProperty* visprop = new a2dVisibleProperty( cirEe, propid, 100, 20, 0.0 );
    cirEe->Append( visprop );

    *settop = top;;
}




/*
void FillData1( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    top->SetName("topdrawing") );
    a2dNameReference* topr = new a2dNameReference(0,0,top);
    datatree->Append( topr );

    a2dRect *tr = new a2dRect(-500,400,300,200 );
    tr->SetStroke(wxColour(9,115,64 ),10.0);
    tr->SetFill(wxColour(100,117,5),a2dFILL_HORIZONTAL_HATCH);
    top->Append( tr );

    a2dRectC *r = new a2dRectC(-500,-300,300,140 );
    top->Append( r );

    a2dOrigin* origin= new a2dOrigin(15,15);
    origin->SetStroke(*wxRED,3);
    //top->Append( origin );

    a2dSLine* ll = new a2dSLine( -500,0,400,-100 );
    ll->SetStroke(wxColour(252,0,252 ),20.0);
    top->Append( ll );

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment(-400,500);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-500,400);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-100,500);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-200,100);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-300,375);
    pointlist22->Append(point22);
    a2dPolygonL* poly166= new a2dPolygonL(pointlist22, true);
    poly166->SetStroke(wxColour(9,115,64 ),4.0);
    top->Append( poly166 );

    wxVariant prop( _T("value of property") );
    wxCanvasVisibleVariantProperty* ppp = new wxCanvasVisibleVariantProperty( _T("Firstproperty") ,prop, -480, 1200,
                      wxFont( 18, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL ),80,10);
    ppp->GetTextObject()->SetFill( wxColour(219,215,6), a2dFILL_SOLID );
    top->SetOrAddProperty( ppp );

    a2dText* tt=new a2dText( _T("Hello rotated text in wxCanvasWorld") , -500, 750,
                      80,15, wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ) );
    tt->SetFill(wxColour(0,10,200));
    tt->SetStroke(*wxGREEN);
    top->Append( tt );

    a2dEndsLine* l = new a2dEndsLine( -600,-600,400,-700 );
    l->SetStroke(wxColour(252,0,52 ),20.0);
    top->Append( l );

    a2dCircle* cire = new  a2dCircle( 0,0, 30 );
    cire->SetFill(wxColour(219,215,6),a2dFILL_HORIZONTAL_HATCH );
    cire->SetStroke(wxColour(1,3,205 ),3.0);
    l->SetEnd(cire);

    a2dEndsLine* l3 = new a2dEndsLine( -500,-700,0,-800 );
    l3->SetStroke(wxColour(252,0,252 ),6.0);
    top->Append( l3 );

    a2dArrow* arrow = new  a2dArrow( 0,0,140,90,70 );
    arrow->SetFill(wxColour(219,215,6));
    arrow->SetStroke(wxColour(1,3,205 ),6.0);
    l3->SetEnd(arrow);

    a2dPolyline* poly2= new a2dPolyline();
    poly2->Add( 230, 220 );
    poly2->Add( 300, 200 );
    poly2->Add( 300, 300 );
    poly2->Add( 800, 300 );

    poly2->SetStroke(wxColour(200,0,64 ),20.0,a2dSTROKE_DOT_DASH);
    top->Prepend( poly2 );

    a2dPolygon* poly= new a2dPolygon();
    poly->Add( -430, -220 );
    poly->Add( 100, -200 );
    poly->Add( 100, -300 );
    poly->Add( 50, -350 );
    poly->Add( 0, -100 );

    //poly->SetFill(wxColour(19,215,6),a2dFILL_HORIZONTAL_HATCH ));
    //poly->SetStroke(wxColour(18,3,205 ),30.0,a2dSTROKE_SOLID));
    poly->SetLayer(6);
    top->Append( poly );

    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment(-400,100);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(-400,200);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(0,200);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(0,100);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(-200,175);
    pointlist2->Append(point2);

    a2dPolylineL* poly16= new a2dPolylineL(pointlist2, true);
    poly16->SetStroke(wxColour(9,115,64 ),4.0);
    top->Prepend( poly16 );
    poly16->SetEnd(arrow);
    poly16->SetBegin(arrow);

    a2dCircle* cir = new  a2dCircle( -500, -1000, 100 );
    cir->SetFill(wxColour(19,215,6),a2dFILL_HORIZONTAL_HATCH);
    cir->SetStroke(wxColour(198,3,105 ),30.0);
    top->Prepend( cir );

    a2dEllipse* elp = new  a2dEllipse( -500, -1500, 100,300 );
    elp->SetFill(wxColour(100,17,55),a2dFILL_VERTICAL_HATCH );
    elp->SetStroke(wxColour(2,255,6 ),10.0,a2dSTROKE_DOT);
    top->Prepend( elp );

    a2dEllipticArc* aelp = new  a2dEllipticArc( -500, -1900, 100,300, 30,270 );
    aelp->SetFill(wxColour(100,17,155));
    aelp->SetStroke(wxColour(1,215,6 ),10.0);
    top->Prepend( aelp );

    a2dArc* aca = new  a2dArc( -500, -2150, 200, 30,270 );
    aca->SetFill(wxColour(100,17,155));
    aca->SetStroke( a2dTRANSPARENT_STROKE );
    top->Prepend( aca );

//    a2dText* ct= new a2dText( _T("Hello i am a Freetype with background brush") , 0,0,
//                      a2dGlobals->GetFontPathList().FindValidPath("LiberationSerif-Regular.ttf"), 100 );
    a2dText* ct= new a2dText( _T("Hello i am a Freetype with background brush") , 0,0,
                      a2dGlobals->GetFontPathList().FindValidPath( _T("LiberationSans-Bold.ttf") , 100 );
    ct->SetFill(a2dFill(wxColour(0,245,246),wxColour(9,5,246),a2dFILL_SOLID ));
    ct->SkewX(-40);
    ct->Rotate(30);
    ct->Scale(2,3);
    ct->Translate(-500, -3000);
    top->Append( ct );

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject();
    group1->Translate(1220,630);
    group1->SetName( _T("group1") );


    a2dCanvasObjectReference* ref1 = new a2dCanvasObjectReference(500,-500,  tr );

    a2dCanvasObjectReference* ref2 = new a2dCanvasObjectReference(500,-500,  r );
    a2dCanvasObjectReference* ref3 = new a2dCanvasObjectReference(500,-500,  ll );
    a2dCanvasObjectReference* ref4 = new a2dCanvasObjectReference(500,-500,  origin );

    a2dCanvasObjectReference* ref6 = new a2dCanvasObjectReference(500,-500,  tt );

    a2dCanvasObjectReference* ref7 = new a2dCanvasObjectReference(500,-500,  l );
    a2dCanvasObjectReference* ref8 = new a2dCanvasObjectReference(500,-500,  cire );
    a2dCanvasObjectReference* ref9 = new a2dCanvasObjectReference(500,-500,  l3 );
    a2dCanvasObjectReference* ref10 = new a2dCanvasObjectReference(500,-500,  poly2 );
    a2dCanvasObjectReference* ref11 = new a2dCanvasObjectReference(500,-500,  poly );
    a2dCanvasObjectReference* ref12 = new a2dCanvasObjectReference(500,-500,  poly16 );
    a2dCanvasObjectReference* ref13 = new a2dCanvasObjectReference(500,-500,  cir );
    a2dCanvasObjectReference* ref14 = new a2dCanvasObjectReference(500,-500,  elp );
    a2dCanvasObjectReference* ref15 = new a2dCanvasObjectReference(500,-500,  aelp );

    a2dCanvasObjectReference* ref16 = new a2dCanvasObjectReference(500,-500,  aca );
    a2dCanvasObjectReference* ref17 = new a2dCanvasObjectReference(500,-500,  poly166 );
    a2dCanvasObjectReference* ref18 = new a2dCanvasObjectReference(500,-500,  ct );

    group1->Prepend( ref1 );
    group1->Prepend( ref2 );
    group1->Prepend( ref3 );
    group1->Prepend( ref4 );
    group1->Prepend( ref6 );
    group1->Prepend( ref7 );
    group1->Prepend( ref8 );
    group1->Prepend( ref9 );
    group1->Prepend( ref10 );
    group1->Prepend( ref11 );
    group1->Prepend( ref12 );
    group1->Prepend( ref13 );
    group1->Prepend( ref14 );
    group1->Prepend( ref15 );
    group1->Prepend( ref16 );
    group1->Prepend( ref17 );
    group1->Prepend( ref18 );

    top->Prepend(group1);
    *settop = top;;

}
*/

void FillData2( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dRect* tr = new a2dRect( -300, -400, 800, 800 );

    tr->SetFill( wxColour( 1, 227, 225 ), a2dFILL_HORIZONTAL_HATCH );
    tr->SetStroke( wxColour( 198, 3, 105 ), 0.0 );
    top->Append( tr );
    tr->GeneratePins( a2dPinClass::Standard, a2d_GeneratePinsForPinClass, 0, 0 );

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group2 = new a2dCanvasObject( 0, 2100 );
    group2->SetName( _T( "group2" ) );
    {
        a2dRect* tr = new a2dRect( 0, -100, 500, 300 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 7, 5 ), a2dFILL_VERTICAL_HATCH );
        group2->Append( tr );

        a2dSLine* ll = new a2dSLine( 50, 0, 240, 100 );
        ll->SetStroke( wxColour( 252, 0, 2 ), 20.0 );
        group2->Append( ll );

        a2dPin* pin1 = new a2dPin( group2, _T( "pin1" ) , a2dPinClass::Standard, 0, 0, 180 );
        group2->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group2, _T( "pin2" ) , a2dPinClass::Standard, 500, 80, 0 );
        group2->Append( pin2 );

        a2dPin* pin3 = new a2dPin( group2, _T( "pin3" ) , a2dPinClass::Standard, 150, -100, -90 );
        group2->Append( pin3 );
    }
    top->Prepend( group2 );


    //make a group of a2dCanvasObjects
    a2dCanvasObject* group22 = new a2dCanvasObject( 1110, 1400 );
    group22->SetName( _T( "group22" ) );
    {
        a2dRect* tr = new a2dRect( 0, -100, 800, 100 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 127, 5 ) );
        group22->Append( tr );

        a2dSLine* ll = new a2dSLine( 50, 0, 240, 100 );
        ll->SetStroke( wxColour( 252, 0, 200 ), 2.0 );
        group22->Append( ll );

        a2dPin* pin4 = new a2dPin( group22, _T( "pin4" ) , a2dPinClass::Standard, 240, 100, 0 );
        group22->Append( pin4 );

        a2dPin* pin1 = new a2dPin( group22, _T( "pin1" ) , a2dPinClass::Standard, 0, 0, 180 );
        group22->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group22, _T( "pin2" ) , a2dPinClass::Standard, 800, 10, 0 );
        group22->Append( pin2 );

        a2dPin* pin3 = new a2dPin( group22, _T( "pin3" ) , a2dPinClass::Standard, 150, -100, -90 );
        group22->Append( pin3 );
    }
    top->Prepend( group22 );

    a2dCanvasObject* group3 = new a2dCanvasObject( 500, 1100 );
    group3->SetName( _T( "group3" ) );
    group3->Rotate( 30 );
    {
        a2dRect* tr = new a2dRect( 0, -100, 500, 210 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 237, 5 ), a2dFILL_VERTICAL_HATCH );
        group3->Append( tr );
        a2dPin* pin1 = new a2dPin( group3, _T( "pin1" ) , a2dPinClass::Standard, 0, 50, 20 );
        group3->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group3, _T( "pin2" ) , a2dPinClass::Standard, 500, 80, 0 );
        group3->Append( pin2 );
    }
    top->Prepend( group3 );
    group3->ConnectWith( top, group2, _T( "pin1" ) );

    a2dCanvasObject* group4 = new a2dCanvasObject( 230, 500 );
    group4->SetName( _T( "group4" ) );
    {
        a2dRect* tr = new a2dRect( 0, -100, 200, 550 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 237, 5 ), a2dFILL_VERTICAL_HATCH );
        group4->Append( tr );
        a2dPin* pin1 = new a2dPin( group4, _T( "pin1" ) , a2dPinClass::Standard, 0, 30, 180 );
        group4->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group4, _T( "pin2" ) , a2dPinClass::Standard, 200, 250, 0 );
        group4->Append( pin2 );
    }
    top->Prepend( group4 );
    group4->ConnectWith( top, group3, _T( "pin2" ) );

    a2dCanvasObject* group5 = new a2dCanvasObject( 630, 500 );
    group5->SetName( _T( "group5" ) );
    {
        a2dRect* tr = new a2dRect( 0, -100, 200, 550 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 237, 5 ), a2dFILL_VERTICAL_HATCH );
        group5->Append( tr );
        a2dPin* pin1 = new a2dPin( group5, _T( "pin1" ) , a2dPinClass::Standard, 0, 30, 180 );
        group5->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group5, _T( "pin2" ) , a2dPinClass::Standard, 200, 250, 0 );
        group5->Append( pin2 );
    }
    top->Prepend( group5 );

    {
        a2dRect* rect = new a2dRect( -900, -100, 500, 350 );
        rect->SetStroke( wxColour( 49, 1, 64 ), 0 );
        rect->SetFill( wxColour( 100, 237, 235 ), a2dFILL_SOLID );
        a2dPin* pin1 = new a2dPin( rect, _T( "pin1" ) , a2dPinClass::Standard, 0, 130, 180 );
        rect->Append( pin1 );
        a2dPin* pin2 = new a2dPin( rect, _T( "pin2" ) , a2dPinClass::Standard, 400, 250, 0 );
        rect->Append( pin2 );
        top->Prepend( rect );
    }

    a2dWires* wg = new a2dWires( group2, _T( "pin2" ) );
    wg->SetStroke( wxColour( 10, 222, 215 ), 0.0, a2dSTROKE_SHORT_DASH );
    top->Prepend( wg );

    //wg->ConnectWith( top, group2, _T("pin1"));
    wg->ConnectWith( top, group2, _T( "pin3" ) );

    wg->ConnectWith( top, tr, _T( "pin1" ) );
    wg->ConnectWith( top, tr, _T( "pin4" ) );
    wg->ConnectWith( top, tr, _T( "pin5" ) );
    wg->ConnectWith( top, tr, _T( "pin6" ) );
    wg->ConnectWith( top, tr, _T( "pin7" ) );

    int i;
    for ( i = 1; i < 3; i += 1 )
    {
        a2dRectC* r = new a2dRectC( -300 + i / 3, -300 + i / 2, 30, 140 );
        top->Append( r );
    }

    *settop = top;;

}

void FillData3( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );
//    randomize();
    int i;
    for( i = 0; i < 10000; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        top->Append( r );
    }

    for( i = 0; i < 200; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
        r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ), double( rand() % 10 ) );
        r->SetLayer( 5 );
        top->Append( r );
    }

    *settop = top;

}

void FillData4( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dText* ppp = new a2dText( _T( "CASTELS made of sand slip into the sea eventually" ) , -180, 1000, a2dFont( 180.0, wxFONTFAMILY_SWISS ) );
    ppp->SetFill( *a2dTRANSPARENT_FILL );
    top->Append( ppp );

    /*
        int i;
        for(i=0; i<360; i=i+20)
        {
            a2dText* ppp = new a2dText("castels made of sand slip into the sea eventually") , -80, 1200,
                              wxFont( 18, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL ),80,i);
            ppp->SetFill( a2dTRANSPARENT_FILL );
            top->Append( ppp );
        }

        for(i=0; i<360; i=i+20)
        {
            a2dText* tt=new a2dText( _T("Hello rotated text in wxCanvasWorld") , -80, -700,
                              wxFont( 18,wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxNORMAL ),80,i);
            tt->SetFill(wxColour(0,10,200));
            tt->SetStroke(*wxGREEN);
            top->Append( tt );
        }

        for(i=0; i<360; i=i+20)
        {
            a2dText* ct= new a2dText( _T("Hello i am a Freetype with brush") , 2400, 1200,
                              a2dGlobals->GetFontPathList().FindValidPath("LiberationSerif-Regular.ttf") , 100 );
            ct->SetFill( wxColour(0,245,246),wxColour(9,5,246));
            ct->Scale(1,1.2);
            ct->SetRotation(i);
            top->Append( ct );
        }

        for(i=0; i<360; i=i+20)
        {
            a2dText* ct2= new a2dText( _T("Hello i am a Freetype") , 2400, -700,
                              a2dGlobals->GetFontPathList().FindValidPath("LiberationSerif-Regular.ttf") , 150 );
            ct2->SetFill(*a2dTRANSPARENT_FILL);
            ct2->Scale(1,1.2);
            ct2->SetRotation(i);

            top->Append( ct2 );
        }
    */
#if wxART2D_USE_FREETYPE

    a2dText* ct = new a2dText( _( "\"And on the eight day God said, \"Murphy, you're in charge.\"" ) , 100, -300,
                               a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 100.0 ) );
    a2dFont stfont = a2dFont( wxT( "nen.chr" ), 100.0, false, 10 );
    stfont.SetWeight( 10 );
    ct->SetFont( stfont );
    ct->SetFill( wxColour( 0, 245, 246 ), wxColour( 9, 5, 246 ), a2dFILL_GRADIENT_FILL_XY_LINEAR );
//    ct->Scale(2,4.2);
    ct->SetRotation( 30 );
    top->Append( ct );

    //create a vector path
    a2dVpath* vpath = new a2dVpath();
    wxString str = ct->GetText();
    ct->GetFont().GetVpath( str, *vpath, 100, -300 );
    a2dVectorPath* canpath = new a2dVectorPath( vpath );
    canpath->SetStroke( wxColour( 1, 3, 205 ), 6.0 );
    top->Append( canpath );

    a2dVpath* vpath2 = new a2dVpath();
    ct->GetFont().GetVpath( str, *vpath2, 0, 0, wxMINX | wxMAXY, true, ct->GetTransformMatrix() );
    a2dVectorPath* canpath2 = new a2dVectorPath( vpath2 );
    //canpath2->Translate( 100, -300 );
    canpath2->SetStroke( wxColour( 1, 123, 205 ), 3.0 );
    top->Append( canpath2 );

    a2dText* ctf = new a2dText( _( "\"And on the eight day God said, \"Murphy, you're in charge.\"" ) , 100, 100,
                                a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 100.0 ) );
    ctf->SetFill( wxColour( 0, 245, 246 ) );
//    ctf->Scale(2,4.2);
    ctf->SetRotation( 30 );
    top->Append( ctf );

    //create a vector path
    a2dVpath* vpath3 = new a2dVpath();
    str = ctf->GetText();
    ctf->GetFont().GetVpath( str, *vpath3, 100, 100 );
    a2dVectorPath* canpath3 = new a2dVectorPath( vpath3 );
    canpath3->SetStroke( wxColour( 1, 3, 205 ), 0 );
    top->Append( canpath3 );


    a2dText* ct3 = new a2dText( _T( "Hello i am a Freetype with background brush" ) , 100, -800,
                                a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 100.0 ) );
    ct3->SetFill( wxColour( 0, 245, 46 ) );
    ct3->Scale( 2, 3.2 );
    ct3->SetRotation( 30 );
    top->Append( ct3 );

    a2dText* ct2 = new a2dText( _T( "Hello i am a Freetype with TransParent brush" ) , 300, 200,
                                a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 150.0 ) );
    ct2->SetFill( *a2dTRANSPARENT_FILL );
    ct2->Scale( 2, 4.2 );
    ct2->SetRotation( -30 );

    top->Append( ct2 );

    a2dText* ct4 = new a2dText( _T( "Hello i am a Freetype with background brush" ) , 0, 0,
                                a2dFont( wxT( "LiberationSans-Bold.ttf" ), 100.0 ) );
    ct4->SetFill( wxColour( 0, 245, 246 ) );
    ct4->SkewX( -40 );
    ct4->Rotate( 30 );
    ct4->Scale( 2, 3 );
    ct4->Translate( -500, -100 );
    top->Append( ct4 );
#endif

    *settop = top;;
}

void FillData5( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dSLine* ll = new a2dSLine( -600, -400, 400, -700 );
    ll->SetStroke( wxColour( 252, 0, 252 ), 20.0 );
    top->Append( ll );

    //line begin and ends
    {
        a2dArrow* arrow = new  a2dArrow( 0, 0, 140, 90, 70 );
        arrow->SetFill( wxColour( 219, 215, 6 ) );
        arrow->SetStroke( wxColour( 1, 3, 205 ), 6.0 );
        arrow->SetLayer( wxLAYER_DEFAULT + 1 );

        a2dCircle* cire = new  a2dCircle( 0, 0, 60 );
        cire->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
        cire->SetStroke( wxColour( 1, 3, 205 ), 3.0 );
        cire->SetLayer( wxLAYER_DEFAULT + 1 );

        a2dEndsLine* l3 = new a2dEndsLine( -500, -1000, 400, -1500 );
        l3->SetStroke( wxColour( 252, 0, 252 ), 6.0 );
        l3->SetLayer( wxLAYER_DEFAULT + 1 );
        top->Append( l3 );
        l3->SetBegin( cire );
        l3->SetEnd( arrow );
    }

    a2dCanvasObject* assogroup = new a2dCanvasObject( -600, -450 );


    //Put a nice rectangles around the properties
    a2dRect* rec = new a2dRect( 0, -12, 740, 175 );
    rec->SetFill( *wxBLACK );
    rec->SetStroke( *wxRED );
    rec->SetHitFlags( a2dCANOBJ_EVENT_STROKE );

    assogroup->Append( rec );


    a2dStringProperty* ppp = new a2dStringProperty( PROPID_property1 , _T( "value of property" ) );
    ppp->SetToObject( assogroup );
    a2dVisibleProperty* pppvis = new a2dVisibleProperty( assogroup, PROPID_property1, 40, 0, true,
            a2dFont( 40.0, wxFONTFAMILY_SWISS ) );
    pppvis->SetStroke( *wxRED );
    pppvis->SetFill( *wxBLACK );
    assogroup->Append( pppvis );

    a2dBoolProperty* ppp2 = new a2dBoolProperty( PROPID_bool , true );
    ppp2->SetToObject( assogroup );
    a2dVisibleProperty* ppp2vis = new a2dVisibleProperty( assogroup, PROPID_bool, 40, 45, true,
            a2dFont( 30.0, wxFONTFAMILY_SWISS ) );
    ppp2vis->SetStroke( *wxCYAN );
    ppp2vis->SetFill( *wxBLACK );
    assogroup->Append( ppp2vis );

    a2dDoubleProperty* ppp3 = new a2dDoubleProperty( PROPID_doubleprop , 3.14 );
    ppp3->SetToObject( assogroup );
    a2dVisibleProperty* ppp3vis = new a2dVisibleProperty( assogroup, PROPID_doubleprop, 40, 105, true,
            a2dFont( 40.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    ppp3vis->SetStroke( *wxGREEN );
    ppp3vis->SetFill( *wxBLACK );
    assogroup->Append( ppp3vis );

    ll->Append( assogroup );

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment( -400, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -500, 400 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -100, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -200, 100 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -300, 375 );
    pointlist22->push_back( point22 );
    a2dPolygonL* poly166 = new a2dPolygonL( pointlist22, true );
    //a2dPolygonL* poly166= new a2dPolygonL(pointlist22,false);
    poly166->SetStroke( wxColour( 9, 115, 64 ), 14.0 );
    poly166->SetFill( *a2dTRANSPARENT_FILL );
    poly166->SetContourWidth( 100 );

    a2dStringProperty* variantprop = new a2dStringProperty( PROPID_prop1 , _T( "bigs in space" ) );
    a2dStringProperty* variantprop2 = new a2dStringProperty( PROPID_prop2 , _T( "some more eqaul then others" ) );
    a2dDoubleProperty* variantprop3 = new a2dDoubleProperty( PROPID_prop3 , 3.1425656 );
    variantprop->SetToObject( poly166 );
    variantprop2->SetToObject( poly166 );
    variantprop3->SetToObject( poly166 );

    poly166->Append( new a2dVisibleProperty( poly166, PROPID_prop1, -400.0, 500.0, ( bool )true ) );
    poly166->Append( new a2dVisibleProperty( poly166, PROPID_prop2, -400.0, 380.0, ( bool )true ) );
    poly166->Append( new a2dVisibleProperty( poly166, PROPID_prop3, -400.0, 260.0, ( bool )true ) );

    poly166->SetHitFlags( a2dCANOBJ_EVENT_STROKE );

    top->Prepend( poly166 );

    a2dVertexList* pointlist222 = new a2dVertexList();
    a2dLineSegment* point222 = new a2dLineSegment( -1400, 1500 );
    pointlist222->push_back( point222 );
    point222 = new a2dLineSegment( -1500, 1400 );
    pointlist222->push_back( point222 );
    point222 = new a2dLineSegment( -1100, 1500 );
    pointlist222->push_back( point222 );
    point222 = new a2dLineSegment( -370, 1100 );
    pointlist222->push_back( point222 );
    //point222 = new a2dArcSegment(-300,375,100,475);
    point222 = new a2dArcSegment( -300, 375, -600, 775 );
    pointlist222->push_back( point222 );
    point222 = new a2dLineSegment( -1300, 1375 );
    pointlist222->push_back( point222 );
    a2dPolygonL* poly1666 = new a2dPolygonL( pointlist222, false );
    poly1666->SetStroke( wxColour( 9, 115, 164 ), 14.0 );
    poly1666->SetFill( wxColour( 109, 115, 164 ) );

    top->Prepend( poly1666 );

    a2dImage* renderedimage = new a2dImage( poly1666, -1400, 2000, 300, 300 );
    renderedimage->SetStroke( wxColour( 0, 115, 164 ), 0.0 );
    renderedimage->SetFill( *a2dTRANSPARENT_FILL ); //else layer fill would be use
    top->Prepend( renderedimage );


    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( -30, -20 );
    poly->AddPoint( 100, 0 );
    poly->AddPoint( 100, 100 );
    poly->AddPoint( 50, 150 );
    poly->AddPoint( 0, 100 );

    poly->SetFill( wxColour( 100, 17, 255 ), a2dFILL_CROSSDIAG_HATCH );
    poly->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    top->Prepend( poly );

    a2dPolylineL* polyline = new a2dPolylineL();
    polyline->AddPoint( -230, -20 );
    polyline->AddPoint( 300, 0 );
    polyline->AddPoint( 300, 100 );
    polyline->AddPoint( 250, 150 );
    polyline->AddPoint( 200, 100 );
    polyline->SetStroke( wxColour( 9, 115, 64 ), 10.0 );
    top->Prepend( polyline );


    *settop = top;;
}

/*
void FillData6( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    top->SetName("topdrawing") );
    datatree->GetDrawing()->Append( top );

    a2dSLine* ll = new a2dSLine( -500,0,400,-100 );
    ll->SetStroke(wxColour(252,0,252 ),50.0);
    top->Append( ll );

    //make a group of a2dCanvasObjects
    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject();
    group1->Translate(1000,500);
    group1->SetName("group1") );

    a2dCanvasObjectReference* ref3 = new a2dCanvasObjectReference(500,-500,  ll );

    group1->Prepend( ref3 );

    group1->SetRotation(30);

    top->Prepend(group1);

    *settop = top;;
}
*/


void FillData6( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dDrawingFrame* frame =  new  a2dDrawingFrame( 450, 680, 400, 600 );
    frame->SetFill( wxColour( 21, 215, 126 ), a2dFILL_HORIZONTAL_HATCH );
    frame->SetStroke( wxColour( 1, 123, 205 ), 2.0 );
    top->Append( frame );

    a2dCircle* cirframe = new  a2dCircle( 120, 230, 160 );
    cirframe->SetFill( wxColour( 219, 215, 6 ), a2dFILL_VERTICAL_HATCH );
    cirframe->SetStroke( wxColour( 1, 3, 205 ), 23.0 );
    frame->Append( cirframe );

    a2dCircle* cirsube = new  a2dCircle( 0, 0, 60 );
    cirsube->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cirsube->SetStroke( wxColour( 1, 3, 205 ), 23.0 );

    a2dRect* tr = new a2dRect( -500, 800, 300, 200 );
    tr->SetFill( *a2dNullFILL );
    tr->SetStroke( *a2dNullSTROKE );
    tr->SkewX( -40 );
    tr->Scale( 2, 3.2 );
    tr->SetRotation( 20 );
    tr->Append( cirsube );

    top->Append( tr );

    a2dSLine* ll = new a2dSLine( -500, 0, 400, -100 );
    ll->SetStroke( wxColour( 252, 0, 252 ), 50 );
    top->Append( ll );

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment( -400, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -500, 400 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -100, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -200, 100 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -300, 375 );
    pointlist22->push_back( point22 );
    a2dPolygonL* poly166 = new a2dPolygonL( pointlist22, true );
    poly166->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    top->Append( poly166 );

    a2dPropertyIdString* propid = ( a2dPropertyIdString* ) poly166->HasPropertyId( "myprop1" );
    if ( !propid )
    {
        propid = new a2dPropertyIdString( "myprop1", wxT( "whatever" ), a2dPropertyId::flag_userDefined );
        poly166->AddPropertyId( propid );
    }
    propid->SetPropertyToObject( poly166, wxT( "whatever" ) );

    a2dVisibleProperty* pppvis = new a2dVisibleProperty( poly166, propid, 48, 12, true,
            a2dFont( 20.0, wxFONTFAMILY_SWISS ), 10.0 );
    //pppvis->SetFill(*wxRED,*wxBLACK,a2dFILL_SOLID );
    pppvis->SetFill( *wxBLACK );
    poly166->Append( pppvis );

    //line begin and ends
    a2dArrow* arrow = new  a2dArrow( 0, 0, 140, 90, 70 );
    arrow->SetFill( wxColour( 219, 215, 6 ) );
    arrow->SetStroke( wxColour( 1, 3, 205 ), 6.0 );

    a2dCircle* cire = new  a2dCircle( 0, 0, 60 );
    cire->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cire->SetStroke( wxColour( 1, 3, 205 ), 3.0 );

    a2dEndsLine* l = new a2dEndsLine( -600, -600, 400, -700 );
    l->SetStroke( wxColour( 252, 0, 52 ), 20 );
    top->Append( l );
    l->SetBegin( cire );
    l->SetEnd( arrow );

    a2dEndsLine* l3 = new a2dEndsLine( -500, -700, 0, -800 );
    l3->SetStroke( wxColour( 252, 0, 252 ), 6.0 );
    top->Append( l3 );

    l3->SetBegin( arrow );
    l3->SetEnd( arrow );

    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment( -400, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -400, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -200, 175 );
    pointlist2->push_back( point2 );

    a2dPolylineL* poly16 = new a2dPolylineL( pointlist2, true );
    poly16->SetStroke( wxColour( 9, 115, 64 ), 4 );
    top->Prepend( poly16 );


    a2dPropertyIdString* propid2 = ( a2dPropertyIdString* ) poly16->HasPropertyId( "myprop1" );
    if ( !propid2 )
    {
        propid2 = new a2dPropertyIdString( "myprop1", _T( "12" ), a2dPropertyId::flag_userDefined );
        poly16->AddPropertyId( propid2 );
    }
    propid2->SetPropertyToObject( poly16, wxT( "no way" ) );

    a2dVisibleProperty* ppvis = new a2dVisibleProperty( poly16, propid2, 48, 12, true,
            a2dFont( 50.0, wxFONTFAMILY_SWISS ), 10.0 );
    ppvis->SetFill( *wxRED, a2dFILL_SOLID );
    poly16->Append( ppvis );

    a2dCircle* cir = new  a2dCircle( -500, -1000, 100 );
    cir->SetFill( wxColour( 19, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cir->SetStroke( wxColour( 198, 3, 105 ), 30 );
    cir->Scale( 2, 3.2 );
    cir->SkewX( -40 );
    cir->SetRotation( 20 );
    top->Prepend( cir );


    a2dEllipse* elp = new  a2dEllipse( -500, -1500, 100, 300 );
    elp->SetFill( wxColour( 100, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    elp->SetStroke( wxColour( 2, 255, 6 ), 10, a2dSTROKE_DOT );
    elp->SkewX( -40 );
    //ct->TransLate(-500, -3000);
    elp->Scale( 2, 3.2 );
    elp->SetRotation( 20 );
    top->Prepend( elp );

    a2dEllipticArc* aelp = new  a2dEllipticArc( -500, -1900, 100, 300, 30, 270 );
    aelp->SetFill( wxColour( 100, 17, 155 ) );
    aelp->SetStroke( wxColour( 1, 215, 6 ), 10 );
    aelp->SkewX( 40 );
    aelp->Scale( 2, 3.2 );
    aelp->SetRotation( 20 );
    top->Prepend( aelp );

    a2dArc* aca = new  a2dArc( -500, -2000, 400, 30, 270 );
    aca->SetFill( wxColour( 100, 17, 155 ) );
    aca->SetStroke( wxColour( 255, 215, 6 ), 10 );
    //aca->Scale(4,3.2);
    //aca->SkewX(20);
    //aca->SetRotation(20);
    top->Prepend( aca );


    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject();
    group1->Translate( 1000, 500 );
    group1->SetName( _T( "group1" ) );

    group1->Prepend( aca );

    a2dCanvasObjectReference* ref1 = new a2dCanvasObjectReference( 500, -500,  tr );
    ref1->SetFill( wxColour( 100, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    ref1->SetStroke( wxColour( 9, 115, 64 ), 10 );

    a2dCanvasObjectReference* ref3 = new a2dCanvasObjectReference( 500, -500,  ll );
    a2dCanvasObjectReference* ref7 = new a2dCanvasObjectReference( 500, -500,  l );
    a2dCanvasObjectReference* ref9 = new a2dCanvasObjectReference( 500, -500,  l3 );
    a2dCanvasObjectReference* ref10 = new a2dCanvasObjectReference( 500, -500,  poly166 );
    a2dCanvasObjectReference* ref11 = new a2dCanvasObjectReference( 500, -500,  elp );
    ref11->SetRotation( 30 );
    ref11->SetFill( wxColour( 10, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    a2dCanvasObjectReference* ref12 = new a2dCanvasObjectReference( 500, -500,  poly16 );
    a2dCanvasObjectReference* ref13 = new a2dCanvasObjectReference( 500, -500,  cir );
    ref13->SetRotation( 30 );
    a2dCanvasObjectReference* ref15 = new a2dCanvasObjectReference( 500, -500,  aelp );
    ref15->SetRotation( 30 );
    a2dCanvasObjectReference* ref16 = new a2dCanvasObjectReference( 500, -500,  aca );
    ref16->SetRotation( 30 );



    group1->Prepend( ref1 );
    group1->Prepend( tr );
    group1->Prepend( ref3 );
    group1->Prepend( ref7 );
    group1->Prepend( ref9 );
    group1->Prepend( ref10 );
    group1->Prepend( ref11 );
    group1->Prepend( ref12 );
    group1->Prepend( ref13 );
    group1->Prepend( ref15 );
    group1->Prepend( ref16 );

    a2dCanvasObjectReference* ref111 = new a2dCanvasObjectReference( 500, -500,  tr );
    top->Prepend( ref111 );
    a2dCanvasObjectReference* ref33 = new a2dCanvasObjectReference( 500, -500,  ll );
    top->Prepend( ref33 );

    group1->SetRotation( 0 );

    top->Prepend( group1 );

    *settop = top;;
    //*settop = group1;
}
/*
void FillData6( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top=new a2dCanvasObject();
    top->SetName(_T("topdrawing") );
    datatree->GetDrawing()->Append( top );

    a2dCircle* cirsube = new  a2dCircle( 0,0, 60 );
    cirsube->SetFill(wxColour(219,215,6),a2dFILL_HORIZONTAL_HATCH );
    cirsube->SetStroke(wxColour(1,3,205 ),23.0);

    a2dRect *tr = new a2dRect(-500,800,300,200 );
    tr->SetFill( a2dNullFILL );
    tr->SetStroke( a2dNullSTROKE );
    tr->SkewX(-40);
    tr->Scale(2,3.2);
    tr->SetRotation(20);
    tr->Append( cirsube );

    a2dCanvasObjectPtrProperty* propr = new a2dCanvasObjectPtrProperty( PROPID_renderpre, new a2dRect(-100,-100,500,400 ) );
    propr->GetCanvasObject()->SetFill( wxColour(219,215,236) );
    propr->SetVisible( true );
    propr->SetCanRender( true );
    propr->SetPreRender( true );
    propr->SetSelectedOnly( true );
    propr->SetToObject( tr );
    top->Append( tr );

    a2dSLine* ll = new a2dSLine( -500,0,400,-100 );
    ll->SetStroke(wxColour(252,0,252 ),50);
    top->Append( ll );

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment(-400,500);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-500,400);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-100,500);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-200,100);
    pointlist22->Append(point22);
    point22 = new a2dLineSegment(-300,375);
    pointlist22->Append(point22);
    a2dPolygonL* poly166= new a2dPolygonL(pointlist22, true);
    poly166->SetStroke(wxColour(9,115,64 ),4.0);
    top->Append( poly166 );

    a2dStringProperty* variantprop = new a2dStringProperty(PROPID_prop1 , _T("Firstproperty") );
    variantprop->SetToObject( poly166 );
    a2dVisibleProperty* pppvis = new a2dVisibleProperty( poly166, PROPID_prop1, 48, 12, true,
                      wxFont( 18, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL ),20,10);
    //pppvis->SetFill(*wxRED,*wxBLACK,a2dFILL_SOLID );
    pppvis->SetFill(*wxBLACK);
    poly166->Append( pppvis );

    //line begin and ends
    a2dArrow* arrow = new  a2dArrow( 0,0,140,90,70 );
    arrow->SetFill(wxColour(219,215,6));
    arrow->SetStroke(wxColour(1,3,205 ),6.0);

    a2dCircle* cire = new  a2dCircle( 0,0, 60 );
    cire->SetFill(wxColour(219,215,6),a2dFILL_HORIZONTAL_HATCH );
    cire->SetStroke(wxColour(1,3,205 ),3.0);

    a2dEndsLine* l = new a2dEndsLine( -600,-600,400,-700 );
    l->SetStroke(wxColour(252,0,52 ),20);
    top->Append( l );
    l->SetBegin(cire);
    l->SetEnd(arrow);

    a2dEndsLine* l3 = new a2dEndsLine( -500,-700,0,-800 );
    l3->SetStroke(wxColour(252,0,252 ),6.0);
    top->Append( l3 );

    l3->SetBegin(arrow);
    l3->SetEnd(arrow);

    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment(-400,100);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(-400,200);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(0,200);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(0,100);
    pointlist2->Append(point2);
    point2 = new a2dLineSegment(-200,175);
    pointlist2->Append(point2);

    a2dPolylineL* poly16= new a2dPolylineL(pointlist2, true);
    poly16->SetStroke(wxColour(9,115,64 ),4);
    top->Prepend( poly16 );

    a2dStringProperty* prop = new a2dStringProperty(PROPID_polyline , _T("12") );
    prop->SetToObject( poly16 );
    a2dVisibleProperty* ppvis = new a2dVisibleProperty( poly16, PROPID_polyline, 48, 12, true,
                      wxFont( 18, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL ),50,10);
    ppvis->SetFill(*wxRED,*wxBLACK,a2dFILL_SOLID );
    poly16->Append( ppvis );

    a2dCircle* cir = new  a2dCircle( -500, -1000, 100 );
    cir->SetFill(wxColour(19,215,6),a2dFILL_HORIZONTAL_HATCH );
    cir->SetStroke(wxColour(198,3,105 ),30);
    cir->Scale(2,3.2);
    cir->SkewX(-40);
    cir->SetRotation(20);
    top->Prepend( cir );


    a2dEllipse* elp = new  a2dEllipse( -500, -1500, 100,300 );
    elp->SetFill(wxColour(100,17,55),a2dFILL_VERTICAL_HATCH );
    elp->SetStroke(wxColour(2,255,6 ),10,a2dSTROKE_DOT);
    elp->SkewX(-40);
    //ct->TransLate(-500, -3000);
    elp->Scale(2,3.2);
    elp->SetRotation(20);
    top->Prepend( elp );

    a2dEllipticArc* aelp = new  a2dEllipticArc( -500, -1900, 100,300, 30,270 );
    aelp->SetFill(wxColour(100,17,155));
    aelp->SetStroke(wxColour(1,215,6 ),10);
    aelp->SkewX(40);
    aelp->Scale(2,3.2);
    aelp->SetRotation(20);
    top->Prepend( aelp );

    a2dArc* aca = new  a2dArc( -500, -2000, 400, 30,270 );
    aca->SetFill(wxColour(100,17,155));
    aca->SetStroke(wxColour(255,215,6 ),10);
    //aca->Scale(4,3.2);
    //aca->SkewX(20);
    //aca->SetRotation(20);
    top->Prepend( aca );


    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject();
    group1->Translate(1000,500);
    group1->SetName(_T("group1") );

    group1->Prepend( aca );


    //a2dCanvasObjectReference* ref111 = new a2dCanvasObjectReference(500,-500,  tr );
    //top->Prepend(ref111);
    //a2dCanvasObjectReference* ref33 = new a2dCanvasObjectReference(500,-500,  ll );
    //top->Prepend(ref33);

    group1->SetRotation(0);

    top->Prepend(group1);

    *settop = top;;
    // *settop = group1;
}
*/
void FillData7( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );


    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject();
    group1->SetName( _T( "group1" ) );

    {
        a2dRect* tr = new a2dRect( 0, -80, 300, 200 );
        tr->SetStroke( wxColour( 9, 115, 64 ), 0 );
        tr->SetFill( wxColour( 100, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
        group1->Append( tr );

        //a2dSLine* ll = new a2dSLine( -50,0,40,100 );
        //ll->SetStroke(wxColour(252,0,252 ),20.0);
        //group1->Append( ll );

        a2dCircle* cir = new  a2dCircle( -100, 100, 100 );
        cir->SetFill( wxColour( 19, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
        cir->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
        group1->Prepend( cir );

        a2dPin* pin1 = new a2dPin( group1, _T( "pin1" ) , a2dPinClass::Standard, 0, 0, 160 );
        group1->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group1, _T( "pin2" ) , a2dPinClass::Standard, 300, 80, 0 );
        group1->Append( pin2 );
    }

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group2 = new a2dCanvasObject();
    group2->SetName( _T( "group1" ) );

    {
        a2dRect* tr = new a2dRect( 0, -100, 500, 300 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 7, 5 ), a2dFILL_VERTICAL_HATCH );
        group2->Append( tr );

        //a2dSLine* ll = new a2dSLine( 50,0,240,100 );
        //ll->SetStroke(wxColour(252,0,2 ),20.0);
        //group2->Append( ll );

        a2dPin* pin1 = new a2dPin( group2, _T( "pin1" ) , a2dPinClass::Standard, 0, 0, 180 );
        group2->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group2, _T( "pin2" ) , a2dPinClass::Standard, 500, 80, 0 );
        group2->Append( pin2 );

        a2dPin* pin3 = new a2dPin( group2, _T( "pin3" ) , a2dPinClass::Standard, 150, -100, -90 );
        group2->Append( pin3 );
    }

    top->Prepend( group2 );

    top->Prepend( group1 );


    *settop = top;;
}



/* to test something simpler
void FillData7( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top=new a2dCanvasObject(0,0);
    top->SetName("topdrawing") );
    datatree->GetDrawing()->Append( top );


    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject(0,0);
    group1->SetName("group1") );

    {
        a2dRect *tr = new a2dRect(0,-80,300,200 );
        tr->SetStroke(wxColour(9,115,64 ),0);
        tr->SetFill(wxColour(100,117,5),a2dFILL_HORIZONTAL_HATCH ));
        group1->Append( tr );

        //a2dSLine* ll = new a2dSLine( -50,0,40,100 );
        //ll->SetStroke(wxColour(252,0,252 ),20.0);
        //group1->Append( ll );

        a2dCircle* cir = new  a2dCircle( -100, 100, 100 );
        cir->SetFill(wxColour(19,215,6),a2dFILL_HORIZONTAL_HATCH ));
        cir->SetStroke(wxColour(198,3,105 ),30.0);
        group1->Prepend( cir );

        a2dPin* pin1 = new a2dPin("pin1") , 0, 0, 160 );
        group1->Append( pin1 );

        a2dPin* pin2 = new a2dPin("pin2") , 300, 80, 0 );
        group1->Append( pin2 );
    }

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group2 = new a2dCanvasObject(0,0);
    group2->SetName("group1") );

    {
        a2dRect *tr = new a2dRect(0,-100,500,300 );
        tr->SetStroke(wxColour(9,1,64 ),0);
        tr->SetFill(wxColour(100,7,5),a2dFILL_VERTICAL_HATCH ));
        group2->Append( tr );

        //a2dSLine* ll = new a2dSLine( 50,0,240,100 );
        //ll->SetStroke(wxColour(252,0,2 ),20.0);
        //group2->Append( ll );

        a2dPin* pin1 = new a2dPin("pin1") , 0, 0, 180 );
        group2->Append( pin1 );

        a2dPin* pin2 = new a2dPin("pin2") , 500, 80, 0 );
        group2->Append( pin2 );
    }

    wxCanvasGraph* g1 = new wxCanvasGraph( 200, 300,0, group1);
    top->Prepend(g1);

    wxCanvasGraph* g4 = new wxCanvasGraph( 500, 1780,0, group2);
    top->Prepend(g4);

    wxCanvasWireGraph* wg = new wxCanvasWireGraph(g1, _T("pin2") );
    top->Prepend(wg);

    wg->ConnectWith(g4, _T("pin2") );

    datatree->AppendEventHandler( m_eventhandler );
    *settop = top;;
}
*/

void FillData8( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dRect* r = new a2dRect( 100, 100, 300, 140, 50 );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    r = new a2dRect( 150, 150, 400, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    r = new a2dRect( 200, 200, 300, 140 );
    r->SetStroke( wxColour( 9, 215, 64 ), 10.0 );
    r->SetFill( wxColour( 0, 217, 200 ), a2dFILL_CROSS_HATCH );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    r = new a2dRect( 100, 200, 800, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 1 );
    top->Append( r );

    r = new a2dRect( 150, 250, 300, 140 );
    r->SetStroke( wxColour( 255, 5, 25 ), 30.0 );
    r->SetFill( wxColour( 244, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    r->SetLayer( wxLAYER_DEFAULT + 1 );
    top->Append( r );

    r = new a2dRect( 200, 300, 400, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 1 );
    top->Append( r );

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group = new a2dCanvasObject();
    group->Translate( 300, 400 );

    a2dRect* rr = new a2dRect( 0, 0, 400, 140 );
    group->Append( rr );
    rr->SetStroke( wxColour( 255, 255, 25 ), 30.0 );
    rr->SetFill( wxColour( 44, 0, 55 ) );
    rr = new a2dRect( 50, 50, 400, 140 );
    rr->SetStroke( wxColour( 255, 5, 85 ), 13.0 );
    rr->SetFill( wxColour( 44, 0, 125 ), a2dFILL_CROSS_HATCH );
    group->Append( rr );
    //layer 8 for the whole group (so object will de drawn on layer 8
    //and not their own layer)
    group->SetLayer( wxLAYER_DEFAULT + 3 );
    top->Append( group );


    a2dCanvasObjectReference* refgroup = new a2dCanvasObjectReference( 500, 600, group );
    //layer 4 for the whole ref (so objects will de drawn on layer 4
    //and not their own layer)
    refgroup->SetLayer( wxLAYER_DEFAULT + 4 );
    top->Prepend( refgroup );

    *settop = top;;
}

void FillData9( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{
    FillData10( datatree, settop );
    datatree->GetDrawing()->GetRootObject()->SetSelectable( false ); //top not
    ( *settop )->SetSelectable( false );
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::SELECTED );
    setflags.Start( datatree->GetDrawing()->GetRootObject(), true );
    a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::FILLED );
    setflags.Start( datatree->GetDrawing()->GetRootObject(), false );
}

void FillData10( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );
    int i;


//    m_sm1 = new a2dImage( image, 0,70,32,32 );
//    top->Append( m_sm1 );

    a2dOrigin* origin = new a2dOrigin( 15, 15 );
    origin->SetStroke( *wxRED, 3 );
    top->Append( origin );

    a2dRectC* r = new a2dRectC( 10, 250, 300, 140, 0, -50 );
    r->SetStroke( wxColour( 9, 115, 64 ), 10.0 );
    r->SetFill( wxColour( 100, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    //r->SetLayer(wxLAYER_DEFAULT+1);
    top->Append( r );

    r = new a2dRectC( 110, 350, 400, 140, 0, 100 );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    for ( i = 10; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill( *wxRED );
        r->SetStroke( *a2dTRANSPARENT_STROKE );
        top->Append( r );
    }


//    m_sm2 = new a2dImage( image, 0,140,24,24 );
//    top->Append( m_sm2 );

    for ( i = 15; i < 300; i += 10 )
    {
        a2dRect* rec = new a2dRect( i, 50, 3, 140 );
        rec->SetFill( wxColour( 0, 10 + i, 2 + i ) );
        rec->SetStroke( *a2dTRANSPARENT_STROKE );
        rec->SetDraggable( false );
        top->Append( rec );
    }



    a2dSLine* ll = new a2dSLine( -600, 400, 400, 300 );
    ll->SetStroke( wxColour( 252, 0, 22 ), 20.0 );




    a2dStringProperty* prop = new a2dStringProperty( PROPID_prop1 , _T( "Firstproperty" ) );
    //prop->SetToObject( ll );
    PROPID_prop1->SetPropertyToObject( ll, prop );
    a2dVisibleProperty* pppvis = new a2dVisibleProperty( ll, PROPID_prop1, 10, 62, true,
            a2dFont( 20.0, wxFONTFAMILY_SWISS ), 10.0 );
    //pppvis->SetFill(*wxRED,*wxBLACK,a2dFILL_SOLID );
    pppvis->SetFill( *wxBLACK );
    ll->Append( pppvis );

    top->Append( ll );



    //make a group of a2dCanvasObjects
    a2dCanvasObject* groupprop = new a2dCanvasObject( -400, 500 );
    groupprop->Translate( 30, 20 );
    groupprop->SetURI( wxURI( _T( "properties.html#props" ) ) );
    groupprop->SetName( _T( "properties on line" ) );
    {
        a2dPropertyIdString* propid = ( a2dPropertyIdString* ) groupprop->HasPropertyId( PROPID_prop2 );
        if ( !propid )
        {
            groupprop->AddPropertyId( PROPID_prop2 );
        }

        a2dStringProperty* pppn = new a2dStringProperty( PROPID_prop2 , "property 1" );
        a2dVisibleProperty* pppnvis = new a2dVisibleProperty( groupprop, PROPID_prop2, 40, 0, true,
                a2dFont( 40.0, wxFONTFAMILY_SWISS ) );
        pppnvis->SetStroke( *wxRED );
        pppnvis->SetFill( *wxBLACK );
        groupprop->Append( pppnvis );
        //pppn->SetToObject( groupprop );
        PROPID_prop2->SetPropertyToObject( groupprop, pppn );

        a2dPropertyIdBool* propid2 = ( a2dPropertyIdBool* ) groupprop->HasPropertyId( PROPID_bool );
        if ( !propid2 )
        {
            groupprop->AddPropertyId( PROPID_bool );
        }

        a2dBoolProperty* pppn2 = new a2dBoolProperty( PROPID_bool , true );
        a2dVisibleProperty* pppn2vis = new a2dVisibleProperty( groupprop, PROPID_bool, 40, 65, true,
                a2dFont( 30.0, wxFONTFAMILY_SWISS ) );
        pppn2vis->SetStroke( *wxCYAN );
        pppn2vis->SetFill( *wxBLACK );
        groupprop->Append( pppn2vis );
        //pppn2->SetToObject( groupprop );
        PROPID_bool->SetPropertyToObject( groupprop, pppn2 );

        propid = ( a2dPropertyIdString* ) groupprop->HasPropertyId( PROPID_pythagoras );
        if ( !propid )
        {
            groupprop->AddPropertyId( PROPID_pythagoras );
        }

        a2dStringProperty* pppn3 = new a2dStringProperty( PROPID_pythagoras , _T( "3.14" ) );
        a2dVisibleProperty* pppn3vis = new a2dVisibleProperty( groupprop, PROPID_pythagoras, 40, 105, true,
                a2dFont( 40.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
        pppn3vis->SetStroke( *wxGREEN );
        pppn3vis->SetFill( *wxBLACK );
        groupprop->Append( pppn3vis );
        //pppn3->SetToObject( groupprop );
        PROPID_pythagoras->SetPropertyToObject( groupprop, pppn3 );

        //Put a nice rectangle around the properties
        a2dRect* rec = new a2dRect( 0, -12, 840, 200 );
        rec->SetFill( *wxBLACK );
        rec->SetStroke( *wxRED );
        groupprop->Prepend( rec );
    }

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment( -400, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -500, 400 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -100, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -200, 100 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -300, 375 );
    pointlist22->push_back( point22 );
    a2dPolygonL* poly166 = new a2dPolygonL( pointlist22, true );
    poly166->SetStroke( wxColour( 9, 115, 64 ), 10.0 );

    poly166->Append( groupprop );
    top->Prepend( poly166 );

    a2dText* t = new a2dText( _T( "Hello wxCanvasWorld" ) , -480, 450, a2dFont( 80.0, wxFONTFAMILY_SWISS ) );
    t->SetFill( wxColour( 0, 255, 255 ) );
    t->SetStroke( wxColour( 120, 120, 120 ) );
    top->Append( t );


    wxString a = _T( "\
<html>\
<head><title>VFS Demo</title></head>\
<body BGCOLOR=\"#FFFFFF\" text=\"#00FFFF\">\
<h1>Virtual File</h1> <h3><b>Virtual File Systems demonstration</b></h3>\
<p><FONT COLOR=\"#FF0000\">\
 Hello. This sample demonstrates VFS. Try the link (and have a look\
at status bar before clicking on them)\
</FONT></p>\
<b>Enter top level Node...</b>\
</body>\
</html>\
" );
    /*
        wxCanvasHTML* ht=new wxCanvasHTML( a, -480, -250,500,900,false);
        ht->SetFill(wxColour(0,120,2));
        ht->SetStroke(wxColour(120,120,120));
        top->Append( ht );
    */
//    wxCanvasHTML* ht1=new wxCanvasHTML( -2480, -250,200,4450, _T("index.html") );
//    ht1->SetFill(wxColour(0,120,2));
//    ht1->SetStroke(wxColour(120,120,120));
//    top->Append( ht1 );


//    wxCanvasHTML* ht2=new wxCanvasHTML( -2480, -250,1000,3500, _T("index2.html") );
//    ht2->SetFill(wxColour(0,120,2),a2dFILL_SOLID));
//    ht2->SetStroke(*wxGREY_PEN);
//    top->Append( ht2 );


    a2dText* tt = new a2dText( _T( "Multiple line\n Hello rotated \ntext in wxCanvasWorld" ) , -80, 450,
                               a2dFont( 80.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ), 25.0 );
    tt->SetFill(  *a2dTRANSPARENT_FILL  );
    tt->SetStroke( *wxGREEN );
    top->Append( tt );


#if wxART2D_USE_FREETYPE
    a2dText* ct = new a2dText( _T( "Hello wat is dit" ) , 200, 0,
                               a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 100.0 ) );
    ct->SetFill( wxColour( 219, 215, 6 ) );

    top->Append( ct );



    top->Append( new a2dText( _T( "How are you?" ) , 180, 10,
                              a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 8.0 ) ) );

    top->Append( new a2dText( _T( "How are you?" ) , 180, 20,
                              a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 10.0 ) ) );

    top->Append( new a2dText( _T( "How are you?" ) , 180, 30,
                              a2dFont( wxT( "LiberationSerif-Regular.ttf" ), 12.0 ) ) );
    a2dImage* m_sm3 = new a2dImage( image, 0, 210, 32, 32 );
    top->Append( m_sm3 );
#endif

    for ( i = 10; i < 300; i += 10 )
    {
        a2dSLine* l = new a2dSLine( 10, -15, i, 300 );
        l->SetStroke( wxColour( 252, 0, 252 ), 0 );
        top->Append( l );
    }

//    m_sm4 = new a2dImage( image, 0,270,64,32 );
//    m_sm4->SetDragMode(DRAG_RECTANGLE);
//    top->Append( m_sm4 );


    a2dEndsLine* l = new a2dEndsLine( -600, 600, 400, -600 );
    l->SetStroke( wxColour( 252, 0, 252 ), 20.0 );
    top->Append( l );

    a2dCircle* cire = new  a2dCircle( 0, 0, 30 );
    cire->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cire->SetStroke( wxColour( 1, 3, 205 ), 30.0 );
    l->SetEnd( cire );

    a2dCircle* cirEe = new  a2dCircle( 1000, 0, 300, 200 );
    cirEe->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cirEe->SetStroke( wxColour( 1, 3, 205 ), 30.0 );
    top->Append( cirEe );

    a2dEndsLine* l3 = new a2dEndsLine( -700, 700, 0, -600 );
    l3->SetStroke( wxColour( 252, 0, 252 ), 6.0 );
    top->Append( l3 );

    a2dArrow* arrow = new  a2dArrow( 0, 0, 140, 90, 70 );
    arrow->SetFill( wxColour( 219, 215, 6 ) );
    arrow->SetStroke( wxColour( 1, 3, 205 ), 6.0 );
    l3->SetEnd( arrow );

//    m_canvas->Append( new a2dSLine( 10,-1500e6,50,300000e6, 0,255,0 ) );
//    m_canvas->Append( new a2dSLine( 10,-150000,50,300000, 0,255,0 ) );


    //make a group of a2dCanvasObjects
//    a2dCanvasObject* group1 = new a2dCanvasObject();
    MyCanvasObjectGroup* group1 = new MyCanvasObjectGroup( 0, 0 );
    group1->Translate( -130, -123 );
    group1->SetURI( wxURI( _T( "groupswitheventhandling.html#groups" ) ) );

    a2dPolygonL* clippoly = new a2dPolygonL();
    clippoly->SetFill( wxColour( 100, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    clippoly->SetStroke( *wxRED );
    clippoly->AddPoint( -130, -143 );
    clippoly->AddPoint( -230, 180 );
    clippoly->AddPoint( 90, 220 );
    clippoly->AddPoint( 80, -123 );

    a2dPolygonL* clippoly2 = new a2dPolygonL();
    clippoly2->SetFill( *a2dTRANSPARENT_FILL );
    clippoly2->SetStroke( *wxGREEN );
    clippoly2->AddPoint( 38, -43 );
    clippoly2->AddPoint( 48, 230 );
    clippoly2->AddPoint( 135, 250 );
    clippoly2->AddPoint( 130, -33 );
    clippoly2->SetFill( wxColour( 21, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    clippoly2->SetStroke( wxColour( 1, 3, 205 ), 0.0 );

    group1->AddPropertyId( PROPID_clip1 );
    group1->AddPropertyId( PROPID_clip2 );
    group1->AddPropertyId( PROPID_clip3 );
    group1->AddPropertyId( PROPID_clip4 );

    //I want to see it also.
    a2dClipPathProperty* clipprop1 = new a2dClipPathProperty( PROPID_clip1, clippoly );
    clipprop1->SetVisible( true );
    clipprop1->SetToObject( group1 );

    a2dClipPathProperty* clipprop2 = new a2dClipPathProperty( PROPID_clip2, clippoly2 );
    clipprop2->SetVisible( true );
    clipprop2->SetToObject( group1 );

    group1->SetName( _T( "group with clipping" ) );

    a2dArrow* arrow2 = new  a2dArrow( 0, 0, 40, 0, 20 );
    arrow2->SetFill( wxColour( 219, 215, 6 ) );
    arrow2->SetStroke( wxColour( 1, 3, 205 ), 5.0 );

    a2dEndsLine* line = new a2dEndsLine( 10, -85, 50, 230 );
    line->SetStroke( wxColour( 255, 161, 5 ), 5.0, a2dSTROKE_DOT_DASH );
    line->SetEnd( arrow2 );
    group1->Prepend( line );

    group1->Prepend( new a2dImage( image, 4, 38, 32, 32 ) );

    MyCircle* ciree = new  MyCircle( 0, 0, 30 );
    ciree->SetFill( wxColour( 219, 215, 6 ), a2dFILL_SOLID );
    ciree->SetStroke( wxColour( 1, 3, 205 ), 3.0 );
    ciree->SetURI( wxURI( _T( "./index.html" ) ) );
    group1->Append( ciree );


    a2dRect* rec3 = new a2dRect( 20, -20, 50, 170, 30 );
    rec3->SetFill( wxColour( 0, 120, 240 ) );
    rec3->SetStroke( wxColour( 252, 54, 252 ), 3.0 );
    group1->Prepend( rec3 );

    a2dRect* rec2 = new a2dRect( 0, 0, 104, 52 );
    rec2->SetFill( wxColour( 0, 240, 240 ) );
    rec2->SetStroke( wxColour( 210, 40, 50 ), 1.0 );
    group1->Prepend( rec2 );



    a2dPolylineL* poly2 = new a2dPolylineL();
    poly2->AddPoint( 230, 220 );
    poly2->AddPoint( 300, 200 );
    poly2->AddPoint( 300, 300 );

    poly2->SetStroke( wxColour( 200, 0, 64 ), 4.0, a2dSTROKE_DOT_DASH );
    top->Prepend( poly2 );



    //make another group of a2dCanvasObjects
    a2dCanvasObject* group2 = new a2dCanvasObject();
    group2->Prepend( new a2dImage( image, 60, 38, 52, 32 ) );
    a2dRect* rr = new a2dRect( 10, 20, 104, 52, 30 );
    rr->SetFill( wxColour( 10, 17, 255 ), a2dFILL_HORIZONTAL_HATCH );
    rr->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    group2->Prepend( rr );


    //this a reference to group2 put into group1
    a2dCanvasObjectReference* m_subref = new a2dCanvasObjectReference( 60, 50, group2 );
    m_subref->SetRotation( 35 );
    m_subref->SetRotation( 0 );
    group1->Prepend( m_subref );


    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( -30, -20 );
    poly->AddPoint( 100, 0 );
    poly->AddPoint( 100, 100 );
    poly->AddPoint( 50, 150 );
    poly->AddPoint( 0, 100 );

    poly->SetFill( wxColour( 100, 17, 255 ), a2dFILL_CROSSDIAG_HATCH );
    poly->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    group1->Prepend( poly );

    a2dVertexList* pointlist = new a2dVertexList();
    a2dLineSegment* point = new a2dLineSegment( 0, 0 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -300, 100 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -100, 100 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -100, 0 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -200, 50 );
    pointlist->push_back( point );

    a2dPolygonL* poly15 = new a2dPolygonL( pointlist, true );
    wxBitmap gs_bmp36_mono;


    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "pat36.bmp" ) );
    gs_bmp36_mono.LoadFile( path, wxBITMAP_TYPE_BMP );
    wxMask* mask36 = new wxMask( gs_bmp36_mono, *wxBLACK );
    // associate a monochrome mask with this bitmap
    gs_bmp36_mono.SetMask( mask36 );

    a2dFill aa = a2dFill( gs_bmp36_mono );
    aa.SetColour( wxColour( 250, 78, 216 ) );
    aa.SetColour2( *wxRED );
    aa.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
    poly15->SetFill( aa );
    poly15->SetStroke( *wxRED, 4.0 );
    group1->Prepend( poly15 );

    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment( -400, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -400, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -200, 175 );
    pointlist2->push_back( point2 );

    a2dPolylineL* poly16 = new a2dPolylineL( pointlist2 );
    a2dStroke apen = a2dStroke( wxColour( 9, 115, 64 ), 40.0 );

//  wxJOIN_BEVEL =     120,
//  wxJOIN_MITER,
//  wxJOIN_ROUND,
//
//  wxCAP_ROUND =      130,
//  wxCAP_PROJECTING,
//  wxCAP_BUTT

    apen.SetJoin( wxJOIN_MITER );
    apen.SetCap( wxCAP_PROJECTING );

    poly16->SetStroke( apen );
    poly16->SetBegin( arrow );
    poly16->SetEnd( arrow );
    top->Prepend( poly16 );

    a2dPolygonL* poly17 = new a2dPolygonL();
    poly17->AddPoint( 50, 305 );
    poly17->AddPoint( -200, 200 );
    poly17->AddPoint( 0, 500 );
    poly17->AddPoint( 300, 200 );
    poly17->AddPoint( -300, -300 );

    poly17->SetFill( wxColour( 100, 17, 255 ) );
    //poly17->SetStroke(wxColour(10,17,25),16,wxLONG_DASH  ));
    poly17->SetStroke( wxColour( 10, 17, 25 ), 16.0 );
    a2dFill aaa = a2dFill( *wxBLUE, *wxRED, 50, 100, 50, 100, 450 );
    poly17->SetFill( aaa );

    a2dPolygonL* clippoly3 = new a2dPolygonL();
    clippoly3->SetFill( wxColour( 100, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    clippoly3->SetStroke( *wxRED );
    clippoly3->AddPoint( -260, 250 );
    clippoly3->AddPoint( 250, 450 );
    clippoly3->AddPoint( 270, -140 );
    clippoly3->AddPoint( -280, -150 );
    clippoly3->SetPreRenderAsChild( false );
    a2dClipPathProperty* clipprop3 = new a2dClipPathProperty( PROPID_clip3, clippoly3 );
    clipprop3->SetToObject( poly17 );
    //I want to see it also.
    clipprop3->SetVisible( true );
    top->Prepend( poly17 );



    a2dCanvasObjectReference* m_refc = new a2dCanvasObjectReference( 0, -200, poly17 );
    //m_refc->SetRotation(90);
    top->Prepend( m_refc );
    a2dCanvasObjectReference* m_refd = new a2dCanvasObjectReference( 200, -50, poly17 );
    m_refd->SetRotation( 0 );
    top->Append( m_refd );

    //now make two references to group1 into root group of the canvas
    a2dCanvasObjectReference* m_ref = new a2dCanvasObjectReference( 350, -200, group1 );
    m_ref->Scale( 2, 3.2 );
    m_ref->SetRotation( 180 );
    //m_ref->SetScale(2,2);
    top->Append( m_ref );

    top->Append( group1 );

    //group3->Prepend( m_ref );

    a2dCanvasObjectReference* m_ref2 = new a2dCanvasObjectReference( 80, 450, group1 );
    m_ref2->SetRotation( -35 );
    top->Prepend( m_ref2 );

    //this a reference to group2 put into group1
    a2dCanvasObjectReference* subref2 = new a2dCanvasObjectReference( 20, 130, group2 );
    subref2->SetRotation( 15 );
    //group3->Prepend( subref2 );
    top->Prepend( subref2 );

    a2dEllipse* elp = new  a2dEllipse( -100, 250, 100, 300 );
    elp->SetFill( wxColour( 100, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    elp->SetStroke( wxColour( 2, 255, 6 ), 10.0, a2dSTROKE_DOT );
    top->Prepend( elp );

    a2dEllipticArc* aelp = new  a2dEllipticArc( -230, 250, 100, 300, 30, 270 );
    aelp->SetFill( wxColour( 100, 17, 155 ) );
    aelp->SetStroke( wxColour( 1, 215, 6 ), 10.0 );
    top->Prepend( aelp );

    a2dArc* aca = new  a2dArc( -530, 250, 200, 30, 270 );
    aca->SetFill( wxColour( 100, 17, 155 ) );
    aca->SetStroke( wxColour( 1, 215, 6 ), 10.0 );
    top->Prepend( aca );

    a2dArc* aca2 = new  a2dArc( -630, 350, -730, -200, -730, 250.0 );
    aca2->SetFill( *a2dTRANSPARENT_FILL );
    aca2->SetStroke( wxColour( 222, 255, 6 ), 10.0, a2dSTROKE_DOT );
    top->Prepend( aca2 );

    a2dArc* aca3 = new  a2dArc( -630, 50, -730, -500, -730, -50.0, true );
    aca3->SetFill( *a2dTRANSPARENT_FILL );
    aca3->SetStroke( wxColour( 222, 255, 6 ), 20.0, a2dSTROKE_SOLID );
    top->Prepend( aca3 );

    a2dEllipticArc* aca4 = new  a2dEllipticArc( -730, -350.0, 500, 300, 45, 300 );
    aca4->SetFill( *wxRED );
    aca4->SetStroke( wxColour( 2, 255, 236 ), 30.0, a2dSTROKE_SOLID );
    top->Prepend( aca4 );

    a2dEllipticArc* aca44 = new  a2dEllipticArc( -730, -750.0, 500, 300, 45, 300, true );
    aca44->SetStroke( wxColour( 2, 25, 136 ), 30.0, a2dSTROKE_SOLID );
    top->Prepend( aca44 );

    //HOW BAD DO THINGS GET
    int kk;
    for ( kk = 0; kk < 100; kk++ )
    {
//        Mya2dCanvasObjectReference* m_refx = new Mya2dCanvasObjectReference(180,50+kk*30, group1);
//        m_refx->SetRotation(-35);
//        top->Prepend( m_refx );
    }

    /*
        m_log = new wxTextCtrl( this, -1, _T("")) , wxPoint(0,0), wxSize(100,100), wxTE_MULTILINE );
        wxLog *old_log = wxLog::SetActiveTarget( new wxLogTextCtrl( m_log ) );
        delete old_log;

        wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

        topsizer->Add( m_canvas, 1, wxEXPAND );
        topsizer->Add( m_log, 0, wxEXPAND );

        SetAutoLayout( true );
        SetSizer( topsizer );
    */

    //fancy background
    a2dPolygonL* poly18 = new a2dPolygonL();
    poly18->AddPoint( -350, -350 );
    poly18->AddPoint( -350, 50 );
    poly18->AddPoint( 50, 50 );
    poly18->AddPoint( 50, -350 );

    //poly18->SetStroke(wxColour(10,17,25),16,wxTRANSPARENT  ));
    //poly18->SetFill(wxColour(19,215,6),a2dFILL_HORIZONTAL_HATCH ));
    //TODO if pen not set and gradient, VERY Slow dragging
    poly18->SetStroke( wxColour( 10, 17, 25 ), 16.0 );
    a2dFill aaaa = a2dFill( wxColour( 0, 155, 0 ) );
    poly18->SetFill( aaaa );

    a2dCanvasObjectReference* m_refb = new a2dCanvasObjectReference( 200, 0, poly18 );
    m_refb->SetObjectTip( _T( "make my day punk" ) , 0, 0, 40 );
//    m_refb->SetRotation(90);

    top->Prepend( m_refb );


    MyCircle* cir2 = new  MyCircle( -300.0, 350.0, 160.0 );
    cir2->SetFill( wxColour( 250, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cir2->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    cir2->SetURI( wxURI( _T( "./index.html" ) ) );

    a2dCanvasObject* objtip = new a2dText( _T( "object tip of the day" ) , 160 + 12, 0, a2dFont( 30.0, wxFONTFAMILY_SWISS ) );
    objtip->SetFill( wxColour( 29, 215, 6 ), a2dFILL_SOLID );
    objtip->SetVisible( false );
    a2dCanvasObject::PROPID_Objecttip->SetPropertyToObject( cir2, objtip  );

    top->Prepend( cir2 );

    a2dCanvasObjectReference* m_refcir = new a2dCanvasObjectReference( 500, -500, cir2 );
    //TODO something wrog with property placement it seems
    //m_refcir->SetRotation(0);
    m_refcir->SetRotation( 0 );

    top->Prepend( m_refcir );

    ///datatree->AppendEventHandler( m_eventhandler );

    a2dCircle* cir = new  a2dCircle( -100, -750, 200 );
    a2dFill caa = a2dFill( *wxGREEN, *wxRED, -60, 0, -60, 0, 300 );
    cir->SetFill( caa );
    cir->SetStroke( wxColour( 198, 3, 105 ) );
    top->Prepend( cir );

    a2dPolygonL* clippoly4 = new a2dPolygonL();
    clippoly4->SetFill( wxColour( 100, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    clippoly4->SetStroke( *wxRED );
    clippoly4->AddPoint( -200, -170 );
    clippoly4->AddPoint( 0, 200 );
    clippoly4->AddPoint( 200, -170 );
    a2dClipPathProperty* clipprop4 = new a2dClipPathProperty( PROPID_clip4, clippoly4 );
    clipprop4->SetToObject( cir );
    //I want to see it also.
    clipprop4->SetVisible( true );
    //and editable
    cir->Append( clippoly4 );



    a2dCanvasObject* cont = top->Find( _T( "group with clipping" ) );

    a2dStringProperty* pp = new a2dStringProperty( PROPID_prop1 , _T( "apenoot" ) );
    pp->SetToObject( cont );

    wxString contname = cont->GetName();
    cont->RemoveProperty( a2dCanvasObject::PROPID_Name );
    wxString stamp = cont->GetName();
    cont->SetName( _T( "group111" ) );
    cont->SetName( _T( "group222" ) );
    wxString aap = cont->GetName();
    cont->RemoveProperty( a2dCanvasObject::PROPID_Name );
    wxString aap2 = cont->GetName();
    cont->SetName( _T( "group with clipping" ) );
    *settop = top;
}

void FillData11( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );


    a2dRect* tr = new a2dRect( -400, -150, 700, 800 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 200.0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
    top->Append( tr );

    a2dCanvasObject* g = new a2dCanvasObject();
    g->Translate( -200, -1000 );
    top->Append( g );
    a2dRect* tr2 = new a2dRect( -400, -150, 700, 500 );
    tr2->SetStroke( wxColour( 229, 5, 64 ), 200.0 );
    tr2->SetFill( wxColour( 0, 117, 245 ) );
    g->Append( tr2 );


    //root group always at 0,0
    a2dCanvasDocument* m_subtree = new a2dCanvasDocument();

    a2dRect* trs = new a2dRect( 200, 150, 670, 400 );
    trs->SetStroke( wxColour( 29, 255, 4 ), 20.0 );
    trs->SetFill( wxColour( 0, 17, 245 ) );
    m_subtree->GetDrawing()->Append( trs );
    a2dRect* trs2 = new a2dRect( 0, 0, 370, 800 );
    trs2->SetStroke( wxColour( 29, 5, 64 ), 20.0 );
    trs2->SetFill( wxColour( 200, 17, 24 ) );
    m_subtree->GetDrawing()->Prepend( trs2 );

    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );

    a2dImage* im = new a2dImage( image, 300, 400, 362, 362 );

    m_subtree->GetDrawing()->Append( im );


    a2dRenderImage* i = new a2dRenderImage( m_subtree->GetDrawing()->GetRootObject(), 300, 200, 500, 800 );
    {
        /*TODO set mask in a2dCanvasView???
                wxBitmap formask=i->GetBuffer();
                wxBitmap maskbitmap(formask.GetWidth(),formask.GetHeight());
                wxMemoryDC e;
                e.SelectObject(maskbitmap);
                e.Clear();
                e.SetBrush(*wxWHITE_BRUSH);
                e.SetPen(*wxBLACK,wxCANVAS_TRANSPARENT));

                e.DrawCircle(formask.GetWidth()/2,formask.GetHeight()/2,200);
                e.DrawRectangle(50,60,200,300);
                e.SelectObject(wxNullBitmap);
                wxMask* mask= new wxMask(maskbitmap,*wxBLACK);
                formask.SetMask(mask);
        */
    }

    i->SetStroke( wxColour( 209, 5, 4 ), 20.0 );
    i->SetFill( *wxRED, a2dFILL_CROSS_HATCH );
    i->SetScaleOnResize( false );
    i->SetMappingUpp( -200, -200, 3, 3 );
    a2dFill a = a2dFill( wxColour( 21, 4, 233 ) );
    a.SetColour2( wxColour( 17, 215, 243 ) );
    i->SetBackgroundFill( a );

    i->SetGridStroke( a2dStroke( wxColour( 2, 5, 6 ), 0, a2dSTROKE_DOT ) );
    //to show grid as lines
    //i->SetGridLines(true);
    i->SetGridSize( 0 );
    i->SetGridX( 50 );
    i->SetGridY( 100 );
    //This to turn it on
    //i->SetGrid(true);
    top->Append( i );


    a2dRenderImage* ii = new a2dRenderImage( m_subtree->GetDrawing()->GetRootObject(), 100, -400, 500, 800 );

    ii->SetStroke( wxColour( 209, 5, 244 ), 2.0 );
    ii->SetFill( *a2dTRANSPARENT_FILL );
    ii->SetScaleOnResize( false );
    ii->SetMappingUpp( -200, -200, 3, 3 );
    ii->SetYaxis( true );
    a2dFill B = a2dFill( wxColour( 1, 224, 233 ) );
    ii->SetBackgroundFill( B );

    top->Append( ii );

    *settop = top;;
}

void FillData12( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );


    a2dRect* r = new a2dRect( 110, 350, 400, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

#if wxART2D_USE_CURVES
    a2dCanvasXYDisplayGroup* curvegroup = new a2dCanvasXYDisplayGroup( 500, -600 );
    curvegroup->SetStroke( wxColour( 9, 227, 5 ), 2.0 );
    curvegroup->SetBoundaries( -100, -200, 350, 250 );
    curvegroup->GetAxisX()->SetTicTextHeight( 10 );
    curvegroup->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 9, 227, 5 ), 1.0 ) );
    curvegroup->GetAxisX()->SetTicTextStroke( a2dStroke( wxColour( 229, 12, 23 ), 1.0 ) );
    curvegroup->GetAxisX()->SetStroke( a2dStroke( *wxRED_PEN ) );
    curvegroup->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    curvegroup->SetFill( *wxLIGHT_GREY );
    curvegroup->SetAxisAreaFill( a2dFill( wxColour( 100, 207, 255 ) ) );

    top->Append( curvegroup );

    a2dFunctionCurve* curve = new  a2dFunctionCurve( _T( "y=140*(sin(x/10)/(x/10))" ) );
    curve->SetStroke( wxColour( 0, 27, 225 ), 0 );
    curve->SetBoundaries( -100, -100, 250, 400 );
    curve->SetXstep( 2 );
    curvegroup->AddCurve( curve );

    a2dFunctionCurve* curve2 = new  a2dFunctionCurve( _T( "y=140*cos(x/20) + 80*cos(x*2/20) + 40*cos(x*4/20)" ) );
    curve2->SetStroke( wxColour( 227, 2, 25 ), 1 );
    curve2->SetBoundaries( -100, -100, 350, 400 );
    curve2->SetXstep( 2 );
    curvegroup->AddCurve( curve2 );

    a2dCircle* bb = new a2dCircle( 100, 140, 10 );
    bb->SetFill( wxColour( 0, 120, 5 ) );
    bb->SetStroke( wxColour( 0, 217, 225 ), 2.0 );
    curvegroup->Prepend( bb );

    a2dText* tt = new a2dText( _T( "Two curves \n On the same plot" ) , -50, -240,
                               a2dFont( 20.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( *wxBLACK );
    curvegroup->Append( tt );

    a2dText* ttv = new a2dText( _T( "Vertical axis" ) , -170, 0,
                                a2dFont( 20.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ), 90.0 );
    ttv->SetFill( wxColour( 0, 10, 200 ) );
    ttv->SetStroke( wxColour( 0, 117, 125 ), 0 );
    curvegroup->Append( ttv );

    a2dVertexArray* pointlist = new a2dVertexArray();
    a2dLineSegment* point = new a2dLineSegment( -200, -50 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -175, 50 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -50, 100 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 0, 30 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 20, 88 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 100, 300 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 200, 50 );
    pointlist->push_back( point );

    a2dVertexCurve* vcurve = new a2dVertexCurve( pointlist );
    vcurve->SetBoundaries( -100, -100, 200, 400 );
    curvegroup->AddCurve( vcurve );

    a2dMarker* mark1 = new a2dMarker( _T( "%3.0f=%3.0f" ) , curve, 20 );
    mark1->SetFill( wxColour( 0, 230, 235 ) );
    mark1->SetStroke( wxColour( 227, 2, 25 ), 0 );
    mark1->Set( 20, 10, 10 );
    curvegroup->AddMarker( mark1 );

    a2dMarker* mark2 = new a2dMarker( _T( "%3.0f=%3.0f" ) , curve2, 210, 20 );
    curvegroup->AddMarker( mark2 );

    a2dMarker* mark3 = new a2dMarker( _T( "%3.0f=%3.0f" ) , vcurve, 120, 40 );
    curvegroup->AddMarker( mark3 );

    /* not possible because of m_axisArea needed, maybe later will be back.
        //mouse test marker
        a2dFunctionCurve* curve4 = new  a2dFunctionCurve( _T("y=140*(sin(x/10)/(x/10))") );
        curve4->SetStroke(wxColour(0,27,225),0);
        curve4->SetBoundaries( -100, -100, 250, 400 );
        curve4->SetXstep( 2 );
        top->Append( curve4 );

        a2dMarker* mark4 = new a2dMarker( _T(" x%3.0f->y%3.0f") , curve4, 20,20 );
        top->Append( mark4 );
    */
    {
        wxBitmap bitmap2;
        wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "imagenPablo.bmp" ) );
        a2dImage* im = new a2dImage( path, wxBITMAP_TYPE_BMP, -500, -265, 382, 332 );
        top->Append( im );
    }

#endif //wxART2D_USE_CURVES



    a2dPolygonL* poly17 = new a2dPolygonL();
    poly17->AddPoint( 50, 305 );
    poly17->AddPoint( -200, 200 );
    poly17->AddPoint( 0, 500 );
    poly17->AddPoint( 300, 200 );
    poly17->AddPoint( -300, -300 );

    poly17->SetFill( wxColour( 100, 17, 255 ) );
    //poly17->SetStroke(wxColour(10,17,25),16.0);


    a2dEllipse* contourobject = new a2dEllipse( 0, 0, 20, 40 );
    contourobject->SetFill( wxColour( 0, 247, 2 ) );
    contourobject->SetStroke( wxColour( 177, 0, 5 ) );

    // a2dCanvasObjectStroke* objectstroke = new a2dCanvasObjectStroke( contourobject );
    //  objectstroke->SetDistance(150);
    // poly17->SetStroke( objectstroke );

    // poly17->SetContour( new a2dContourRandomizeXY( 30, 10) );

    /*
        a2dShadowStyleProperty* shadow = new a2dShadowStyleProperty( a2dCanvasObject::PROPID_shadowstyle, 0, 0 );
        shadow->SetFill( a2dFill( wxColour(100,207,255), *wxBLACK ) );
        shadow->SetStroke( *wxRED );
        shadow->SetExtrudeAngle( -30 );
        shadow->SetExtrudeDepth( 100 );
        //poly17->SetOrAddProperty( shadow );
    */
    a2dCanvasObjectReference* m_refc = new a2dCanvasObjectReference( 0, -200, poly17 );
    top->Prepend( m_refc );

    top->Prepend( poly17 );

    {
        a2dRectC* cube = new a2dRectC( 500, 600, 200, 100 );
        cube->SetFill(  *wxRED );
        cube->SetStroke( wxColour( 0, 217, 225 ), 2.0 );

        a2d3DShape* shadele = new a2d3DShape( cube,  50, 20 );
        shadele->SetExtrudeStrokeFromShape();
        shadele->SetExtrudeFillFromShape();
        top->Append( shadele );
    }

    {
        a2dRectC* cube2 = new a2dRectC( 400, 600, 200, 100 );
        cube2->SetFill( a2dFill( wxColour( 100, 207, 255 ) ) );

        a2d3DShape* shadele = new a2d3DShape( cube2,  350, 40 );
        shadele->SetExtrudeStrokeFromShape();
        shadele->SetExtrudeFillFromShape();
        shadele->SetStroke( wxColour( 199, 1, 2 ), 0 );
        top->Append( shadele );
    }

    {
        a2dRectC* cube = new a2dRectC( 800, -600, 200, 100 );
        cube->SetFill( wxColour( 230, 250, 5 ) );
        cube->SetStroke( wxColour( 0, 217, 225 ), 20.0 );

        a2d3DShape* shadele = new a2d3DShape( cube,  350, -60 );
        shadele->SetStroke( wxColour( 199, 1, 2 ), 0 );
        shadele->SetFill( wxColour( 2, 2, 5 ), a2dFILL_SOLID );
        top->Append( shadele );
    }

    {
        a2dEllipse* sele = new a2dEllipse( 800, 500, 540, 400 );
        sele->SetFill( wxColour( 230, 250, 5 ) );
        sele->SetStroke( wxColour( 0, 217, 225 ), 20.0 );

        a2dShadowStyleProperty* shadele = new a2dShadowStyleProperty( a2dCanvasObject::PROPID_Shadowstyle, 50 , 30 );
        shadele->SetStroke( wxColour( 99, 1, 2 ), 0 );
        shadele->SetFill( a2dFill( wxColour( 2, 2, 5 ) ) );

        shadele->SetToObject( sele );

        top->Append( sele );
    }

    {
        a2dEllipticArc* sele = new a2dEllipticArc( 400, 500, 540, 400, -140, 150 );
        sele->SetStroke( wxColour( 9, 100, 254 ), 10.0 );
        sele->SetFill( wxColour( 210, 21, 235 ) );

        a2dShadowStyleProperty* shadele = new a2dShadowStyleProperty( a2dCanvasObject::PROPID_Shadowstyle, 80 , 20 );
        shadele->SetStroke( wxColour( 99, 1, 2 ), 0 );
        shadele->SetFill(  wxColour( 2, 2, 5 ) );

        shadele->SetToObject( sele );

        top->Append( sele );
    }

    *settop = top;;

}



void FillData13( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{

    a2dRect* tr = new a2dRect( -500, 750, 900, 80 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
    datatree->GetDrawing()->Append( tr );

    a2dVectorPath* path = new a2dVectorPath();
    path->SetStroke( wxColour( 29, 235, 64 ), 0 );
    path->SetFill( wxColour( 230, 1, 2 ) );
    path->SetPathType( a2dPATH_END_ROUND );
    //path->SetPathType( a2dPATH_END_SQAURE_EXT );
    //path->SetPathType( a2dPATH_END_SQAURE );

    path->SetContourWidth( 20 );
    path->Add( new a2dVpathSegment( 0, 0, a2dPATHSEG_MOVETO ) );
    path->Add( new a2dVpathSegment( 100, 0, a2dPATHSEG_LINETO ) );
    path->Add( new a2dVpathSegment( 200, 200, a2dPATHSEG_LINETO ) );
    path->Add( new a2dVpathSegment( 0, 100, a2dPATHSEG_LINETO ) );
    path->Add( new a2dVpathSegment( 50, 50, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    path->Add( new a2dVpathSegment( 80, 80, a2dPATHSEG_MOVETO ) );
    path->Add( new a2dVpathSegment( 180, 80, a2dPATHSEG_LINETO ) );
    path->Add( new a2dVpathSegment( 280, 280, a2dPATHSEG_LINETO ) );
    path->Add( new a2dVpathSegment( 80, 180, a2dPATHSEG_LINETO ) );
    path->Add( new a2dVpathSegment( 130, 130, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    datatree->GetDrawing()->Append( path );



    a2dVectorPath* path2 = new a2dVectorPath();
    path2->SetStroke( wxColour( 29, 235, 64 ), 0 );
    path2->SetFill( wxColour( 230, 1, 2 ) );

    path2->Add( new a2dVpathSegment( 0, 400, a2dPATHSEG_MOVETO ) );
    path2->Add( new a2dVpathSegment( 100, 400, a2dPATHSEG_LINETO ) );
    path2->Add( new a2dVpathSegment( 200, 600, a2dPATHSEG_LINETO ) );
    path2->Add( new a2dVpathSegment( 0, 700, a2dPATHSEG_MOVETO ) );
    path2->Add( new a2dVpathSegment( 0, 500, a2dPATHSEG_LINETO ) );
    path2->Add( new a2dVpathSegment( 50, 450, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    datatree->GetDrawing()->Append( path2 );

    a2dVectorPath* path3 = new a2dVectorPath();
    path3->SetStroke( wxColour( 29, 235, 234 ), 20.0 );
    path3->SetFill( wxColour( 230, 1, 2 ) );

    path3->Add( new a2dVpathSegment( 0, 600, a2dPATHSEG_MOVETO ) );
    path3->Add( new a2dVpathSegment( 100, 600, a2dPATHSEG_LINETO ) );
    path3->Add( new a2dVpathSegment( 200, 800, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );
    path3->Add( new a2dVpathSegment( 0, 900, a2dPATHSEG_MOVETO ) );
    path3->Add( new a2dVpathSegment( 0, 700, a2dPATHSEG_LINETO ) );
    path3->Add( new a2dVpathSegment( 50, 650, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    datatree->GetDrawing()->Append( path3 );


    a2dVectorPath* path4 = new a2dVectorPath();
    path4->SetStroke( wxColour( 29, 235, 64 ), 10.0 );
    path4->SetFill( wxColour( 230, 1, 2 ) );

    path4->Add( new a2dVpathSegment( 500, 600, a2dPATHSEG_MOVETO ) );
    path4->Add( new a2dVpathSegment( 600, 600, a2dPATHSEG_LINETO ) );
    path4->Add( new a2dVpathSegment( 750, 800, a2dPATHSEG_LINETO_NOSTROKE, a2dPATHSEG_END_CLOSED  ) );
    path4->Add( new a2dVpathSegment( 800, 900, a2dPATHSEG_MOVETO ) );
    path4->Add( new a2dVpathSegment( 800, 700, a2dPATHSEG_LINETO ) );
    path4->Add( new a2dVpathSegment( 900, 900, a2dPATHSEG_LINETO ) );
    path4->Add( new a2dVpathSegment( 850, 650, a2dPATHSEG_LINETO ) );
    path4->Add( new a2dVpathSegment( 1000, 950, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    datatree->GetDrawing()->Append( path4 );

    a2dVectorPath* path5 = new a2dVectorPath();
    path5->SetStroke( wxColour( 29, 235, 64 ), 7.0 );
    //a2dLinearGradientFill* aaa = a2dFill( *wxBLUE, *wxRED );
    //path5->SetFill(aaa);
    path5->SetFill( wxColour( 230, 1, 2 ) );

    path5->Add( new a2dVpathSegment( 0, 1000, a2dPATHSEG_MOVETO ) );
    path5->Add( new a2dVpathCBCurveSegment( 1000, 0, 500, 900, 700, 500, a2dPATHSEG_CBCURVETO ) );
    path5->Add( new a2dVpathCBCurveSegment( 0, -1000, 900, -750, 100, -750, a2dPATHSEG_CBCURVETO_NOSTROKE ) );
    path5->Add( new a2dVpathCBCurveSegment( -1000, 0, -100, -750, -800, -550, a2dPATHSEG_CBCURVETO ) );
    path5->Add( new a2dVpathQBCurveSegment( 0, 1000, -1000, 1000, a2dPATHSEG_QBCURVETO ) );
    path5->Add( new a2dVpathSegment( 0, 500, a2dPATHSEG_LINETO_NOSTROKE ) );
    path5->Add( new a2dVpathSegment( -500, 0, a2dPATHSEG_LINETO ) );
    path5->Add( new a2dVpathSegment( 0, -600, a2dPATHSEG_LINETO ) );
    path5->Add( new a2dVpathSegment( 500, 50, a2dPATHSEG_LINETO ) );
    path5->Add( new a2dVpathSegment( 0, 500, a2dPATHSEG_LINETO_NOSTROKE, a2dPATHSEG_END_CLOSED_NOSTROKE  ) );

    datatree->GetDrawing()->Append( path5 );

    a2dVectorPath* path6 = new a2dVectorPath();
    path6->SetStroke( wxColour( 29, 235, 245 ), 27.0 );
    //a2dLinearGradientFill* aaa = a2dFill( *wxBLUE, *wxRED );
    //path6->SetFill(aaa);
    path6->SetFill( wxColour( 130, 21, 72 ) );

    path6->Add( new a2dVpathSegment( 1000, 1500, a2dPATHSEG_MOVETO ) );
    path6->Add( new a2dVpathArcSegment( 1500, 1000, 1353, 1353, a2dPATHSEG_ARCTO ) );
    path6->Add( new a2dVpathArcSegment( 1000,  500, 1353, 646, a2dPATHSEG_ARCTO_NOSTROKE ) );
    path6->Add( new a2dVpathArcSegment(  500, 1000, 646, 646, a2dPATHSEG_ARCTO ) );
    path6->Add( new a2dVpathArcSegment( 1000, 1500, 646, 1353, a2dPATHSEG_ARCTO_NOSTROKE, a2dPATHSEG_END_CLOSED_NOSTROKE ) );
    a2dVpathSegment* t = new a2dVpathSegment( 1000, 2000, a2dPATHSEG_LINETO_NOSTROKE );
    path6->Add( t );
    t = new a2dVpathArcSegment( t, 1000, 1000, 90 , a2dPATHSEG_ARCTO );
    path6->Add( t );
    t = new a2dVpathArcSegment( t, 1000, 1000, 135 , a2dPATHSEG_ARCTO );
    path6->Add( t );
    t = new a2dVpathArcSegment( t, 1000, 1000, 45 , a2dPATHSEG_ARCTO );
    path6->Add( t );
    t = new a2dVpathArcSegment( t, 1000, 1000, 90 , a2dPATHSEG_ARCTO );
    path6->Add( t );

    t = new a2dVpathSegment( 1000, 2500, a2dPATHSEG_MOVETO );
    path6->Add( t );
    t = new a2dVpathArcSegment( t, 1000, 2000, -130 , a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED );
    path6->Add( t );

    t = new a2dVpathSegment( 1000, 3500, a2dPATHSEG_MOVETO );
    path6->Add( t );
    t = new a2dVpathArcSegment( t, 1000, 3000, 130 , a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED_NOSTROKE );
    path6->Add( t );

    datatree->GetDrawing()->Append( path6 );

}

void FillData14( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );
    int i, j;
    for( i = 0; i < 20; i++ )
    {
        for( j = 0; j < 20; j++ )
        {
            a2dRect* r = new a2dRect( i * 100, j * 100, 100, 100 );
            r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetLayer( 5 );
            top->Append( r );
        }
    }

    for( i = 0; i < 20; i++ )
    {
        for( j = 0; j < 20; j++ )
        {
            a2dRect* r = new a2dRect( i * 100, j * 100, 50, 50 );
            r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetLayer( 5 );
            top->Append( r );
        }
    }

    for( i = 0; i < 20; i++ )
    {
        for( j = 0; j < 20; j++ )
        {
            a2dRect* r = new a2dRect( i * 100, j * 100, 10, 10 );
            r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetLayer( 5 );
            top->Append( r );
        }
    }

    for( i = 0; i < 40; i++ )
    {
        for( j = 0; j < 40; j++ )
        {
            a2dRect* r = new a2dRect( i * 50, j * 50, 5, 5 );
            r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
            r->SetLayer( 5 );
            top->Append( r );
        }
    }

    *settop = top;;

}

void FillData15( a2dCanvasDocument* datatree, a2dCanvasObject** settop  )
{

    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    a2dCanvasObject* lib, *sym, *use;
    //top->Append( new a2dText(_T("Usage") , 1000, 400, 100 ) );
    top->Append( use = new a2dCanvasObject() );
    top->Append( lib = new a2dCanvasObject() );
    //top->Append( new a2dText(_T("Library") , -100, 400, 100 ) );
    lib->Append( sym = new a2dCanvasObject );
    lib->Append( new a2dText( _T( "Library" ) , 0, 150, a2dFont( 100.0, wxFONTFAMILY_SWISS ) ) );

    sym->Append( new a2dSLine( -100, -100, 100, 100 ) );
    sym->Append( new a2dSLine( 100, -100, -100, 100 ) );
    sym->Append( new a2dText( _T( "Symbol" ) , 0, -120, a2dFont( 40.0, wxFONTFAMILY_SWISS ) ) );

    use->Append( new a2dCanvasObjectReference( 1000, 0, sym ) );
    use->Append( new a2dCanvasObjectReference( 1500, 0, sym ) );
    use->Append( new a2dText( _T( "Usage" ) , 1200, 120, a2dFont( 100.0, wxFONTFAMILY_SWISS ) ) );

    *settop = top;;
}

void FillDataAll( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( _T( "topdrawing" ) );
    datatree->GetDrawing()->Append( top );

    //root group always at 0,0
    a2dCanvasDocument* subtree = new a2dCanvasDocument();

    FillData10( subtree, settop );
    a2dLayers* setup = new a2dLayers();


    for ( int i = 0; i < 10; i++ )
    {
        wxString buf;
        buf.Printf( _T( "layer %d" ) , i );
        a2dLayerInfo* n = new a2dLayerInfo( i, buf );
        setup->Append( n );
        n->SetOrder( i );
        n->SetInMapping( i );
        n->SetOutMapping( i );
    }

    wxString buf;
    buf.Printf( _T( "layer default %d " ) , wxLAYER_DEFAULT );
    setup->SetName( wxLAYER_DEFAULT, buf );
    setup->SetStroke( wxLAYER_DEFAULT, a2dStroke( wxColour( 250, 0, 240 ), 1.0, a2dSTROKE_SOLID ) );
    setup->SetFill( wxLAYER_DEFAULT, a2dFill( *wxBLACK, a2dFILL_SOLID ) );
    //setup->SetVisible(wxLAYER_DEFAULT,false);
    setup->SetOrder( wxLAYER_DEFAULT, -1 );

    setup->SetStroke( 5, a2dStroke( wxColour( 9, 115, 64 ), 20, a2dSTROKE_SOLID ) );
    setup->SetFill( 5, a2dFill( wxColour( 10, 217, 2 ), a2dFILL_CROSSDIAG_HATCH ) );

    setup->SetStroke( 6, a2dStroke( wxColour( 9, 115, 64 ), 0, a2dSTROKE_SOLID ) );
    setup->SetFill( 6, a2dFill( wxColour( 100, 17, 255 ), a2dFILL_CROSSDIAG_HATCH ) );

    setup->SetStroke( 7, a2dStroke( wxColour( 9, 115, 64 ), 10.0, a2dSTROKE_DOT_DASH ) );
    setup->SetFill( 7, a2dFill( wxColour( 0, 120, 240 ), a2dFILL_SOLID ) );

    setup->SetStroke( 8, a2dStroke( wxColour( 109, 115, 64 ), 40.0, a2dSTROKE_DOT_DASH ) );
    setup->SetFill( 8, a2dFill( wxColour( 200, 120, 240 ), a2dFILL_SOLID ) );

    setup->SetStroke( 9, a2dStroke( wxColour( 255, 0, 255 ), 40.0, a2dSTROKE_SOLID ) );
    setup->SetFill( 9, a2dFill( wxColour( 0, 255, 0 ), a2dFILL_SOLID ) );


    subtree->SetLayerSetup( setup );

    a2dRenderImage* io = new a2dRenderImage( *settop, 700.0, 200.0, 400.0, 400.0 );
    io->SetStroke( wxColour( 209, 5, 4 ), 20.0 );
    io->SetFill( *a2dTRANSPARENT_FILL );
    io->SetScaleOnResize( false );
    io->SetMappingUpp( -800, -800, 5 , 5 );
    a2dFill B =  a2dFill( wxColour( 231, 4, 233 ) );
    io->SetBackgroundFill( B );

    io->SetGridStroke( a2dStroke( wxColour( 2, 5, 6 ), 0, a2dSTROKE_DOT ) );
    //to show grid as lines
    //i->SetGridLines(true);
    io->SetGridSize( 0 );
    io->SetGridX( 50 );
    io->SetGridY( 100 );
    //This to turn it on
    //io->SetGrid(true);


    top->Append( io );

    //root group always at 0,0
    a2dCanvasDocument* subtree2 = new a2dCanvasDocument();

    FillData5( subtree2, settop );
    a2dLayers* s2 = new a2dLayers();
    s2 = setup;
    subtree2->SetLayerSetup( s2 );

    a2dRenderImage* i2 = new a2dRenderImage( *settop, 300, 200, 200, 300 );
    i2->SetStroke( wxColour( 209, 5, 4 ), 2.0 );
    i2->SetFill( *a2dTRANSPARENT_FILL );
    i2->SetScaleOnResize( false );
    i2->SetMappingUpp( -800, -1800, 13, 13 );
    a2dFill bb = a2dFill( wxColour( 1, 234, 233 ) );
    i2->SetBackgroundFill( bb );

    i2->SetGridStroke( a2dStroke( wxColour( 2, 5, 6 ), 0, a2dSTROKE_DOT ) );
    //to show grid as lines
    //i->SetGridLines(true);
    i2->SetGridSize( 0 );
    i2->SetGridX( 50 );
    i2->SetGridY( 100 );
    //This to turn it on
    //i->->SetGrid(true);

    top->Append( i2 );

    //root group always at 0,0
    a2dCanvasDocument* subtree3 = new a2dCanvasDocument();

    FillData6( subtree3, settop );

    a2dRenderImage* i3 = new a2dRenderImage( *settop, 300, 800, 400, 400 );
    i3->SetStroke( wxColour( 209, 5, 4 ), 6.0 );
    i3->SetFill( *a2dTRANSPARENT_FILL );
    i3->SetScaleOnResize( false );
    i3->SetMappingUpp( -2200, -3400, 17, 17 );
    a2dFill bbb = a2dFill( wxColour( 1, 234, 233 ) );
    i3->SetBackgroundFill( bbb );

    i3->SetGridStroke( a2dStroke( wxColour( 2, 5, 6 ), 0, a2dSTROKE_DOT ) );
    //to show grid as lines
    //i->SetGridLines(true);
    i3->SetGridSize( 0 );
    i3->SetGridX( 50 );
    i3->SetGridY( 100 );
    //This to turn it on
    //i->SetGrid(true);

    top->Append( i3 );

    *settop = top;;
}


void FillData16( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    MyCircle* cir = new MyCircle( 0,0,40);
    datatree->GetDrawing()->GetRootObject()->Append( cir );

    const long WHATEVER_Slider = wxNewId();
    a2dWidgetSlider* sli = new a2dWidgetSlider( datatree->GetDrawing()->GetRootObject(), WHATEVER_Slider, 90, -40, 12, 80 );
    sli->SetHighLightStroke( a2dStroke( wxColour( 10, 222, 215 ) ) );
    sli->SetHighLightFill( a2dFill( wxColour( 255, 102, 102 ), a2dFILL_SOLID ) );
    sli->SetFill( wxColour( 195, 195, 195 ) );
    sli->SetStroke( wxColour( 195, 0, 0 ) );
    sli->SetName( wxT( "__CLOSE_BUTTON__" ) );
    //sli->SetDraggable( false );
    datatree->GetDrawing()->GetRootObject()->Append( sli );
    sli->ConnectEvent( wxEVT_SLIDER, cir );

    *settop = datatree->GetDrawing()->GetRootObject();
}


void FillData17( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( wxT( "topdrawing" ) );

    datatree->GetDrawing()->GetRootObject()->Append( top );

    a2dRect* r = new a2dRect( 100, 100, 300, 140, 50 );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    r = new a2dRect( 150, 150, 400, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    r = new a2dRect( 200, 200, 300, 140 );
    r->SetStroke( wxColour( 9, 215, 64 ), 10.0 );
    r->SetFill( wxColour( 0, 217, 200 ), a2dFILL_CROSS_HATCH );
    r->SetLayer( wxLAYER_DEFAULT + 2 );
    top->Append( r );

    r = new a2dRect( 100, 200, 800, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 1 );
    top->Append( r );

    r = new a2dRect( 150, 250, 300, 140 );
    r->SetStroke( wxColour( 255, 5, 25 ), 30.0 );
    r->SetFill( wxColour( 244, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    r->SetLayer( wxLAYER_DEFAULT + 1 );
    top->Append( r );

    r = new a2dRect( 200, 300, 400, 140 );
    r->SetLayer( wxLAYER_DEFAULT + 1 );
    top->Append( r );

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group = new a2dCanvasObject();
    group->Translate( 300, 400 );

    a2dRect* rr = new a2dRect( 0, 0, 400, 140 );
    group->Append( rr );
    rr->SetStroke( wxColour( 255, 255, 25 ), 30.0 );
    rr->SetFill( wxColour( 44, 0, 55 ) );
    rr = new a2dRect( 50, 50, 400, 140 );
    rr->SetStroke( wxColour( 255, 5, 85 ), 13.0 );
    rr->SetFill( wxColour( 44, 0, 125 ), a2dFILL_CROSS_HATCH );
    group->Append( rr );
    //layer 8 for the whole group (so object will de drawn on layer 8
    //and not their own layer)
    group->SetLayer( wxLAYER_DEFAULT + 3 );
    top->Append( group );


    a2dCanvasObjectReference* refgroup = new a2dCanvasObjectReference( 500, 600, group );
    //layer 4 for the whole ref (so objects will de drawn on layer 4
    //and not their own layer)
    refgroup->SetLayer( wxLAYER_DEFAULT + 4 );
    top->Prepend( refgroup );

    *settop = top;
}

void FillData18( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( wxT( "topdrawing" ) );

    datatree->GetDrawing()->GetRootObject()->Append( top );

    a2dCanvasObject* group1 = new a2dCanvasObject();

    group1->SetLayer( wxLAYER_DEFAULT );
    group1->Translate( -130, -123 );

    a2dRect* rec3 = new a2dRect( 20, -20, 50, 170, 30 );
    rec3->SetFill( wxColour( 0, 120, 240 ) );
    rec3->SetStroke( wxColour( 252, 54, 252 ), 3.0 );
    group1->Prepend( rec3 );

    a2dRect* rec4 = new a2dRect( 120, -120, 150, 70, 30 );
    rec4->SetFill( wxColour( 10, 0, 240 ) );
    rec4->SetStroke( wxColour( 252, 154, 252 ), 3.0 );
    group1->Prepend( rec4 );

    top->Append( group1 );


    wxString path2 = a2dGlobals->GetImagePathList().FindValidPath( wxT( "smile.xpm" ) );

    a2dImage* im2 = new a2dImage( path2 , wxBITMAP_TYPE_XPM, 200, 200, 1100, 1100 );
    im2->SetFill( *wxRED );
    im2->SetStroke( *wxBLUE, 10 );
    //im2->SetFill( *a2dTRANSPARENT_FILL );
    //im2->SetStroke( *a2dTRANSPARENT_STROKE );
    top->Append(  im2 );

    /*
        for(int s=0; s<7600; s+=20)
            for(int mz=0; mz<10240; mz+=20)
        {
          unsigned char r = (s+mz)%255;
          a2dRect* rec4 = new a2dRect(mz, s, 20, 20);
          rec4->SetFill(wxColour(r, 255-r, ~r));
          top->Append( rec4 );
        }
    */
    wxString path = a2dGlobals->GetImagePathList().FindValidPath( wxT( "smaller.png" ) );

    int i;
    for ( i = 0; i < 12; i++ )
    {
        int j;
        for ( j = 0; j < 17; j++ )
        {
            a2dImage* im = new a2dImage( path, wxBITMAP_TYPE_PNG, i * 100, j * 100, 100, 100 );
            //im->SetFill( *a2dTRANSPARENT_FILL );
            im->SetFill( a2dFill(  wxColour( rand() % 254, rand() % 254, rand() % 254, rand() % 250 ) ) );
            //im->SetFill( a2dFill(  wxColour( 0,0,0, rand() % 255 ) ) );
            //im->SetFill( a2dFill(  wxColour( 1,1,1, rand() % 255 ) ) );

            top->Append( im );
        }
    }

    *settop = top;
}

/*
class MyCanvasControl: public a2dControl
{
public:
    MyCanvasControl(wxTextCtrl* but);

    void OnMouseEvent(wxMouseEvent &event);

    wxString m_text;
    wxString m_answer;

    DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(MyCanvasControl,a2dControl)
    EVT_MOUSE_EVENTS( MyCanvasControl::OnMouseEvent )
//    EVT_CHAR(MyButton::OnChar)
//    EVT_KEY_DOWN(MyButton::OnKeyDown)
//

END_EVENT_TABLE()

MyCanvasControl::MyCanvasControl(wxTextCtrl* but):
a2dControl( 100, 600,but)
{
    m_text="are you happy?:";
    m_answer="of course";
}

void MyCanvasControl::OnMouseEvent(wxMouseEvent &event)
{
    if (event.LeftDown())
    {
        wxString text = wxGetTextFromUser(m_text,m_answer);
        if (text=="no")
        {
                m_text="i hate you!:";
                m_answer="yes";
        }
    }
    event.Skip();
}


void MyCanvasControl::OnChar(wxKeyEvent &event)
{
    event.Skip();
}
void MyCanvasControl::OnKeyDown(wxKeyEvent &event)
{
    event.Skip();
}
void MyCanvasControl::OnKeyUp(wxKeyEvent &event)
{
    event.Skip();
}
*/

void FillData19( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( wxT( "topdrawing" ) );

    datatree->GetDrawing()->GetRootObject()->Append( top );

    a2dRect* tr = new a2dRect( -500, 400, 300, 200 );
    tr->SetStroke( wxColour( 9, 115, 64 ), 10.0 );
    tr->SetFill( wxColour( 100, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    top->Append( tr );

    a2dRectC* r = new a2dRectC( -500, -300, 300, 140 );
    top->Append( r );

    a2dOrigin* origin = new a2dOrigin( 15, 15 );
    origin->SetStroke( *wxRED, 3 );
    //top->Append( origin );

    a2dSLine* ll = new a2dSLine( -500, 0, 400, -100 );
    ll->SetStroke( wxColour( 252, 0, 252 ), 20.0 );
    top->Append( ll );

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment( -400, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -500, 400 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -100, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -200, 100 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -300, 375 );
    pointlist22->push_back( point22 );
    a2dPolygonL* poly166 = new a2dPolygonL( pointlist22, true );
    poly166->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    top->Append( poly166 );

    a2dText* tt = new a2dText( wxT( "Hello rotated text in a2dCanvas World" ), -500, 750,
                               a2dFont( 80.0, wxFONTFAMILY_SWISS ), 15.0 );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( *wxGREEN );
    top->Append( tt );

    a2dEndsLine* l = new a2dEndsLine( -600, -600, 400, -700 );
    l->SetStroke( wxColour( 252, 0, 52 ), 20.0 );
    top->Append( l );

    a2dCircle* cire = new  a2dCircle( 0, 0, 30 );
    cire->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cire->SetStroke( wxColour( 1, 3, 205 ), 3.0 );
    l->SetEnd( cire );

    a2dEndsLine* l3 = new a2dEndsLine( -500, -700, 0, -800 );
    l3->SetStroke( wxColour( 252, 0, 252 ), 6.0 );
    top->Append( l3 );

    a2dArrow* arrow = new  a2dArrow( 0, 0, 140, 90, 70 );
    arrow->SetFill( wxColour( 219, 215, 6 ) );
    arrow->SetStroke( wxColour( 1, 3, 205 ), 6.0 );
    l3->SetEnd( arrow );

    a2dPolylineL* poly2 = new a2dPolylineL();
    poly2->AddPoint( 230, 220 );
    poly2->AddPoint( 300, 200 );
    poly2->AddPoint( 300, 300 );
    poly2->AddPoint( 800, 300 );

    poly2->SetStroke( wxColour( 200, 0, 64 ), 20.0, a2dSTROKE_DOT_DASH );
    top->Prepend( poly2 );

    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( -430, -220 );
    poly->AddPoint( 100, -200 );
    poly->AddPoint( 100, -300 );
    poly->AddPoint( 50, -350 );
    poly->AddPoint( 0, -100 );

    //poly->SetFill(wxColour(19,215,6),a2dFILL_HORIZONTAL_HATCH ));
    //poly->SetStroke(wxColour(18,3,205 ),30.0,a2dSTROKE_SOLID));
    poly->SetLayer( 6 );
    top->Append( poly );

    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment( -400, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -400, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -200, 175 );
    pointlist2->push_back( point2 );

    a2dPolylineL* poly16 = new a2dPolylineL( pointlist2, true );
    poly16->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    top->Prepend( poly16 );
    poly16->SetEnd( arrow );
    poly16->SetBegin( arrow );

    a2dCircle* cir = new  a2dCircle( -500, -1000, 100 );
    cir->SetFill( wxColour( 19, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cir->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    top->Prepend( cir );

    a2dEllipse* elp = new  a2dEllipse( -500, -1500, 100, 300 );
    elp->SetFill( wxColour( 100, 17, 55 ), a2dFILL_VERTICAL_HATCH );
    elp->SetStroke( wxColour( 2, 255, 6 ), 10.0, a2dSTROKE_DOT );
    top->Prepend( elp );

    a2dEllipticArc* aelp = new  a2dEllipticArc( -500, -1900, 100, 300, 30, 270 );
    aelp->SetFill( wxColour( 100, 17, 155 ) );
    aelp->SetStroke( wxColour( 1, 215, 6 ), 10.0 );
    top->Prepend( aelp );

    a2dArc* aca = new  a2dArc( -500, -2150, 200, 30, 270 );
    aca->SetFill( wxColour( 100, 17, 155 ) );
    aca->SetStroke( *a2dTRANSPARENT_STROKE );
    top->Prepend( aca );

#if wxART2D_USE_FREETYPE
    a2dText* ct = new a2dText( wxT( "Hello i am a Freetype with background brush" ), 0, 0,
                               a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Bold.ttf" ) ), 100 ) );
    ct->SetFill( a2dFill( wxColour( 0, 245, 246 ) ) );
    ct->SetBackGround();
    ct->SkewX( -40 );
    ct->Rotate( 30 );
    ct->Scale( 2, 3 );
    ct->Translate( -500, -3000 );
    top->Append( ct );
#endif

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group1 = new a2dCanvasObject();
    group1->Translate( 1220, 630 );
    group1->SetName( wxT( "group1" ) );

    a2dCanvasObjectReference* ref1 = new a2dCanvasObjectReference( 500, -500,  tr );

    a2dCanvasObjectReference* ref2 = new a2dCanvasObjectReference( 500, -500,  r );
    a2dCanvasObjectReference* ref3 = new a2dCanvasObjectReference( 500, -500,  ll );
    a2dCanvasObjectReference* ref4 = new a2dCanvasObjectReference( 500, -500,  origin );

    a2dCanvasObjectReference* ref6 = new a2dCanvasObjectReference( 500, -500,  tt );

    a2dCanvasObjectReference* ref7 = new a2dCanvasObjectReference( 500, -500,  l );
    a2dCanvasObjectReference* ref8 = new a2dCanvasObjectReference( 500, -500,  cire );
    a2dCanvasObjectReference* ref9 = new a2dCanvasObjectReference( 500, -500,  l3 );
    a2dCanvasObjectReference* ref10 = new a2dCanvasObjectReference( 500, -500,  poly2 );
    a2dCanvasObjectReference* ref11 = new a2dCanvasObjectReference( 500, -500,  poly );
    a2dCanvasObjectReference* ref12 = new a2dCanvasObjectReference( 500, -500,  poly16 );
    a2dCanvasObjectReference* ref13 = new a2dCanvasObjectReference( 500, -500,  cir );
    a2dCanvasObjectReference* ref14 = new a2dCanvasObjectReference( 500, -500,  elp );
    a2dCanvasObjectReference* ref15 = new a2dCanvasObjectReference( 500, -500,  aelp );

    a2dCanvasObjectReference* ref16 = new a2dCanvasObjectReference( 500, -500,  aca );
    a2dCanvasObjectReference* ref17 = new a2dCanvasObjectReference( 500, -500,  poly166 );
#if wxART2D_USE_FREETYPE
    a2dCanvasObjectReference* ref18 = new a2dCanvasObjectReference( 500, -500,  ct );
    group1->Prepend( ref18 );
#endif

    group1->Prepend( ref1 );
    group1->Prepend( ref2 );
    group1->Prepend( ref3 );
    group1->Prepend( ref4 );
    group1->Prepend( ref6 );
    group1->Prepend( ref7 );
    group1->Prepend( ref8 );
    group1->Prepend( ref9 );
    group1->Prepend( ref10 );
    group1->Prepend( ref11 );
    group1->Prepend( ref12 );
    group1->Prepend( ref13 );
    group1->Prepend( ref14 );
    group1->Prepend( ref15 );
    group1->Prepend( ref16 );
    group1->Prepend( ref17 );

    top->Prepend( group1 );



    //wxTextCtrl* but = new wxTextCtrl( this, -1, "apenoot \n schei uit", wxPoint(0,0), wxSize(100,100), wxTE_MULTILINE );
    //top->Prepend( new MyCanvasControl( but ) );

    *settop = top;

}


void FillData20( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dRectWindow *trw = new a2dRectWindow( datatree->GetDrawing()->GetRootObject(), 20,30,300,400 );
    trw->SetTitle( wxT("A window title") );
    trw->SetStroke(wxColour(22,5,64 ),0);
    //trw->SetFill(wxColour(0,11,245));
    trw->SetResizeOnChildBox( true );
    trw->SetSubEdit( true );
    trw->GetCanvas()->SetFill( wxColour(215,215,215) );
    //trw->GetCanvas()->SetStyle( a2dWindowMM::SUNKEN );

    const long WHATEVER = wxNewId();
    a2dWidgetButton* buti = new a2dWidgetButton( trw, WHATEVER, 140, 280, 1200, 40 );
    buti->SetHighLightStroke( a2dStroke( wxColour(10,222,215) ) );
    buti->SetHighLightFill( a2dFill( wxColour(255,102,102), a2dFILL_SOLID ) );
    buti->SetFill( wxColour( 195,195,195) );
    buti->SetStroke( wxColour( 195,0,0) );
    buti->SetName( wxT("__CLOSE_BUTTON__") );
    buti->SetDraggable( false );
    trw->GetCanvas()->Append( buti );

    datatree->GetDrawing()->GetRootObject()->Append( trw );


    const long WHATEVER2 = wxNewId();
    a2dWidgetButton* buti2 = new a2dWidgetButton( trw, WHATEVER2, 14, 80, 120, 40 );
    buti2->SetHighLightStroke( a2dStroke( wxColour(10,222,215) ) );
    buti2->SetHighLightFill( a2dFill( wxColour(255,102,102), a2dFILL_SOLID ) );
    buti2->SetFill( wxColour( 195,195,195) );
    buti2->SetStroke( wxColour( 195,0,0) );
    buti2->SetName( wxT("__CLOSE_BUTTON2__") );
    buti2->SetDraggable( false );
    trw->GetCanvas()->Append( buti2 );

    datatree->GetDrawing()->GetRootObject()->Append( trw );
    *settop = datatree->GetDrawing()->GetRootObject();
}

void FillData21( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dRectWindow* trw = new a2dRectWindow( datatree->GetDrawing()->GetRootObject(), 120, 30, 300, 400 );
    trw->SetTitle( wxT( "A window title" ) );
    trw->SetStroke( wxColour( 22, 5, 64 ), 0 );
    //trw->SetFill(wxColour(0,11,245));
    trw->SetResizeOnChildBox( true );
    trw->SetSubEdit( true );
    trw->GetCanvas()->SetFill( wxColour( 215, 215, 215 ) );
    //trw->GetCanvas()->SetStyle( a2dWindowMM::SUNKEN );

    const long WHATEVER = wxNewId();
    a2dWidgetButton* buti = new a2dWidgetButton( trw, WHATEVER, 140, 280, 120, 40 );
    buti->SetHighLightStroke( a2dStroke( wxColour( 10, 222, 215 ) ) );
    buti->SetHighLightFill( a2dFill( wxColour( 255, 102, 102 ), a2dFILL_SOLID ) );
    buti->SetFill( wxColour( 195, 195, 195 ) );
    buti->SetStroke( wxColour( 195, 0, 0 ) );
    buti->SetName( wxT( "__CLOSE_BUTTON__" ) );
    buti->SetDraggable( false );
    trw->GetCanvas()->Append( buti );

    a2dWindowMM* vas = new a2dWindowMM( 340, 250, 170, 80 );
    vas->SetBorder( 20 );
    vas->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    vas->SetSubEdit( true );
    vas->SetSubEditAsChild( true );
    trw->GetCanvas()->Append( vas );

    a2dWindowMM* vas2 = new a2dWindowMM( 440, 250, 100, 30 );
    vas2->SetBorder( 20 );
    vas2->SetFill( wxColour( 219, 215, 236 ) );
    vas2->SetSubEdit( true );
    vas2->SetSubEditAsChild( true );
    vas2->SetStyle( a2dWindowMM::SUNKEN );
    trw->GetCanvas()->Append( vas2 );

    a2dText* ppn3 = new a2dText( wxT( "first Nested text" ), 100, 140, *a2dDEFAULT_CANVASFONT );
    ppn3->SetFill( a2dFill( wxColour( 10, 217, 250 ) ) );
    ppn3->SetStroke( wxColour( 222, 135, 164 ), 1 );
    ppn3->SetTextHeight( 80 );
    ppn3->SetSubEditAsChild( true );
    ppn3->SetDrawFrame();
    trw->GetCanvas()->Append( ppn3 );

    a2dCircle* cirp22 = new  a2dCircle( 400, 200, 80 );
    cirp22->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp22->SetSubEditAsChild( true );
    cirp22->SetName( wxT( "mies" ) );
    trw->GetCanvas()->Append( cirp22 );

    a2dCircle* cirp2 = new  a2dCircle( 300, 200, 50 );
    cirp2->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp2->SetSubEditAsChild( true );
    cirp2->SetName( wxT( "aap" ) );
    trw->GetCanvas()->Append( cirp2 );

    a2dCircle* cirp = new  a2dCircle( 150, 240, 120 );
    cirp->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cirp->SetName( wxT( "noot" ) );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp->SetSubEditAsChild( true );
    trw->GetCanvas()->Append( cirp );

    a2dText* ppn4 = new a2dText( wxT( "second Nested text" ), 30, 40, *a2dDEFAULT_CANVASFONT );
    ppn4->SetFill( a2dFill( wxColour( 10, 117, 155 ) ) );
    ppn4->SetStroke( wxColour( 22, 215, 14 ), 1 );
    ppn4->SetTextHeight( 50 );
    ppn4->SetSubEditAsChild( true );
    trw->GetCanvas()->Append( ppn4 );

    datatree->GetDrawing()->GetRootObject()->Append( trw );

    *settop = datatree->GetDrawing()->GetRootObject();
}


//! hit test for nested object at border of parent object (enter and leave events)
void FillData22( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    a2dRect* r1 = new a2dRect( 0, 0, 500, 500 );
    datatree->GetDrawing()->GetRootObject()->Append( r1 );

    a2dCircle* cirpX22 = new  a2dCircle( 200, 200, 200 );
    cirpX22->SetFill( wxColour( 29, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirpX22->SetSubEditAsChild( true );
    cirpX22->SetName( wxT( "mies2" ) );
    datatree->GetDrawing()->Append( cirpX22 );


    /*
    a2dRect* trw = new a2dRect( 20, 30, 400, 400 );
    trw->SetStroke( wxColour( 22, 5, 64 ), 0 );
    //trw->SetFill(wxColour(0,11,245));
    trw->SetResizeOnChildBox( true );
    trw->SetSubEdit( true );
    trw->SetFill( wxColour( 215, 215, 215 ) );
    //trw->GetCanvas()->SetStyle( a2dWindowMM::SUNKEN );

    a2dCircle* cirp22 = new  a2dCircle( 200, 200, 80 );
    cirp22->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp22->SetSubEditAsChild( true );
    cirp22->SetName( wxT( "mies" ) );
    trw->Append( cirp22 );

    a2dEllipse* cirp2 = new  a2dEllipse( 100, 200, 200, 100 );
    cirp2->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp2->SetSubEditAsChild( true );
    cirp2->SetName( wxT( "aap" ) );
    trw->Append( cirp2 );

    a2dCircle* cirp3 = new  a2dCircle( -50, 0, 50 );
    cirp3->SetFill( wxColour( 219, 215, 6 ) );
    cirp3->SetSubEditAsChild( true );
    cirp3->SetName( wxT( "deep" ) );
    cirp2->Append( cirp3 );

    datatree->GetDrawing()->GetRootObject()->Append( trw );
    */
    *settop = datatree->GetDrawing()->GetRootObject();
}

void FillData23( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    datatree->GetDrawing()->GetRootObject()->Translate( 200, 600 );
    datatree->GetDrawing()->GetRootObject()->Rotate( -50 );

    a2dRect* trw = new a2dRect( 20, 30, 300, 400 );
    trw->SetStroke( wxColour( 22, 5, 64 ), 0 );
    trw->SetFill( wxColour( 0, 11, 245 ) );
    trw->SetResizeOnChildBox( true );
    trw->SetSubEdit( true );
    trw->Translate( 30, 40 );
    trw->Rotate( 30 );


    a2dText* ppn3 = new a2dText( wxT( "first Nested text" ), 100, 140, *a2dDEFAULT_CANVASFONT );
    ppn3->SetFill( a2dFill( wxColour( 10, 217, 250 ) ) );
    ppn3->SetStroke( wxColour( 222, 135, 164 ), 1 );
    ppn3->SetTextHeight( 80 );
    ppn3->SetSubEditAsChild( true );
    ppn3->SetDrawFrame();
    trw->Append( ppn3 );

    a2dCircle* cirp22 = new  a2dCircle( 400, 200, 80 );
    cirp22->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp22->SetSubEditAsChild( true );
    cirp22->SetName( wxT( "mies" ) );
    trw->Append( cirp22 );

    a2dCircle* cirp2 = new  a2dCircle( 300, 200, 50 );
    cirp2->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp2->SetSubEditAsChild( true );
    cirp2->SetName( wxT( "aap" ) );
    trw->Append( cirp2 );

    a2dCircle* cirp = new  a2dCircle( 150, 240, 120 );
    cirp->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cirp->SetName( wxT( "noot" ) );
    //cirp->SetStroke(wxColour(1,3,205 ),30.0);
    cirp->SetSubEditAsChild( true );
    trw->Append( cirp );

    a2dText* ppn4 = new a2dText( wxT( "second Nested text" ), 30, 40, *a2dDEFAULT_CANVASFONT );
    ppn4->SetFill( a2dFill( wxColour( 10, 117, 155 ) ) );
    ppn4->SetStroke( wxColour( 22, 215, 14 ), 1 );
    ppn4->SetTextHeight( 50 );
    ppn4->SetSubEditAsChild( true );
    trw->Append( ppn4 );

    datatree->GetDrawing()->GetRootObject()->Append( trw );

    *settop = datatree->GetDrawing()->GetRootObject();
}

void FillData24( a2dCanvasDocument* datatree, a2dCanvasObject** settop )
{
    // put some object on it by filling the document.
    a2dText* tt = new a2dText( wxT( "Hello rotated text in a2dCanvas World" ), -10, 20,
                               a2dFont( 80.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ), 0 );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );
    datatree->GetDrawing()->GetRootObject()->Append( tt );

    //a2dDEFAULT_CANVASFONT = a2dFont( 200.0, wxDECORATIVE, wxFONTSTYLE_ITALIC );
    a2dFill fillshared = a2dFill( wxColour( 100, 17, 155 ) );

    a2dText* p = new a2dText( wxT( "Hello, world with shared font" ), 0, 100, *a2dDEFAULT_CANVASFONT  );
    double dHeight = p->GetTextHeight();
    p->SetFill( fillshared );
    p->SetStroke( wxColour( 198, 3, 105 ), 0.0 );
    datatree->GetDrawing()->GetRootObject()->Append( p );

    a2dText* pp = new a2dText( wxT( "Hello, world with shared font" ), 0, 300, *a2dDEFAULT_CANVASFONT );
    pp->SetFill( fillshared );
    pp->SetTextHeight( 150 );
    datatree->GetDrawing()->GetRootObject()->Append( pp );
    dHeight = p->GetTextHeight();

    a2dScaledEndLine* yy = new a2dScaledEndLine( 400, 290, 1400, 590, 40 );
    yy->SetEnd( 4, 2, 2, true );
    yy->SetBegin( 6, 4, 3 );
    datatree->GetDrawing()->GetRootObject()->Append( yy );

    a2dScaledEndLine* yyy = new a2dScaledEndLine( 200, 890, 1400, 890, 80 );
    yyy->SetEnd( 7, 7, 6 );
    yyy->SetBegin( 6, 4, 3 );
    datatree->GetDrawing()->GetRootObject()->Append( yyy );

    wxString path = a2dGlobals->GetImagePathList().FindValidPath( wxT( "image2_transp.png" ) );
    a2dImageMM* im = new a2dImageMM( path, wxBITMAP_TYPE_ANY, 100, 140, 300, 200 );
    im->SetFill( *a2dTRANSPARENT_FILL );
    im->SetDrawPatternOnTop( true );
    datatree->GetDrawing()->GetRootObject()->Append( im );

    a2dRectMM* tr = new a2dRectMM( 220, 130, 300, 400 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
    tr->SetResizeOnChildBox( true );
    tr->SetSubEdit( true );
    datatree->GetDrawing()->GetRootObject()->Append( tr );

    a2dText* ppn = new a2dText( wxT( "Nested text" ), 0, 0, *a2dDEFAULT_CANVASFONT );
    ppn->SetFill( a2dFill( wxColour( 100, 17, 155 ) ) );
    ppn->SetTextHeight( 150 );
    ppn->SetSubEditAsChild( true );
    tr->Append( ppn );

    a2dCircle* cirp = new  a2dCircle( 150, 240, 120 );
    cirp->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cirp->SetStroke( wxColour( 1, 3, 205 ), 30.0 );
    cirp->SetSubEditAsChild( true );
    tr->Append( cirp );

    a2dRect* tr2 = new a2dRect( 22, 13, 300, 170 );
    tr2->SetStroke( wxColour( 22, 135, 164 ), 0.1 );
    tr2->SetFill( wxColour( 123, 117, 25 ) );
    tr2->SetSubEditAsChild( true );
    tr->Append( tr2 );

    a2dText* ppn2 = new a2dText( wxT( "second Nested text" ), 100, 140, *a2dDEFAULT_CANVASFONT );
    ppn2->SetFill( a2dFill( wxColour( 10, 117, 155 ) ) );
    ppn2->SetStroke( wxColour( 222, 135, 164 ), 1 );
    ppn2->SetTextHeight( 50 );
    ppn2->SetSubEditAsChild( true );
    tr->Append( ppn2 );

    a2dText* ct = new a2dText( wxT( "Hello i am a Freetype with background brush" ), 0, 0,
                               a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 100 ) );

    ct->SetFill( a2dFill( wxColour( 0, 245, 246 ), a2dFILL_SOLID ) );
    ct->SkewX( -40 );
    ct->Rotate( 30 );
    ct->Scale( 1.3, 1.2 );
    ct->Translate( -50, -30 );
    datatree->GetDrawing()->GetRootObject()->Append( ct );

    wxString path2 = a2dGlobals->GetImagePathList().FindValidPath( wxT( "image2_transp.png" ) );
    a2dImage* im2 = new a2dImage( path2, wxBITMAP_TYPE_PNG, 0, 0, 200, 300 );
    im2->SetFill( *a2dTRANSPARENT_FILL );
    im2->SetDrawPatternOnTop( true );
    datatree->GetDrawing()->GetRootObject()->Append( im2 );

    wxImage rgba;
    path = a2dGlobals->GetImagePathList().FindValidPath( _T( "image2_transp.png" ) );
    //path = a2dGlobals->GetImagePathList().FindValidPath( _T("markerl.png") );
    rgba.LoadFile( path );
    a2dRgbaImage* imrgba = new a2dRgbaImage(  660, -560, rgba, 255 );
    imrgba->SetStroke( wxColour( 229, 5, 64 ), 5 );
    imrgba->SetFill( *a2dTRANSPARENT_FILL );
    datatree->GetDrawing()->GetRootObject()->Append( imrgba );

    a2dRect* tr22 = new a2dRect( 0, 0, 100, 100 );

    tr22->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr22->SetFill( wxColour( 0, 117, 245 ) );

    datatree->GetDrawing()->GetRootObject()->Append( tr22 );

    *settop = datatree->GetDrawing()->GetRootObject();
}
