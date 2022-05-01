/*! \file wx/curves/plotbox.h
    \brief Higher level of data plotting, adding title and legends.
    \author Sirin Nitinawarat

    Copyright: 2000-2004 (c) Robert Roebling

    Licence: wxWidgets Licence

    RCS-ID: $Id: plotbox.h,v 1.24 2008/09/05 19:01:12 titato Exp $
*/

#ifndef __WXPLOTBOX_H__
#define __WXPLOTBOX_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/curves/meta.h"

//! plot a group of curves on a given area in world coordinates.
/*
    The curves are displayed in the a2dCanvasXYDisplayGroup its
    internal coordinates, which are used within the axis area.

    Next to displaying the curves, the title and axis labels can be set.
*/
class A2DCURVESDLLEXP a2dPlot : public a2dCanvasXYDisplayGroup
{
    DECLARE_EVENT_TABLE()

public:

    a2dPlot( double x = 0, double y = 0 );

    a2dPlot( const a2dPlot& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dPlot();

    //! set title above plot, empty string removes it
    void SetTitle( const wxString& title );

    //! set title above plot using a text object, NULL removes it
    void SetTitleTextDc( a2dText* title );

    //! set markerShow under plot using a a2dMarkerShow object, NULL removes it
    void SetMarkerShow( a2dMarkerShow* showm );

    //! set X label on axis, empty string removes it
    void SetXLabel( const wxString& xlabel, const wxColour& color = wxNullColour );

    //! set X label on axis as text object, NULL removes it
    void SetXLabelTextDc( a2dText* xLabelTextDc );

    //! set Y label on axis, empty string removes it
    void SetYLabel( const wxString& ylabel, const wxColour& color = wxNullColour )
    {
        SetY1Label( ylabel, color );
    }
    //! set Y label on axis as text object, NULL removes it
    void SetYLabelTextDc( a2dText* yLabelTextDc )
    {
        SetY1LabelTextDc( yLabelTextDc );
    }

    //! set Y label on axis, empty string removes it
    void SetY1Label( const wxString& ylabel, const wxColour& color = wxNullColour );

    //! set Y label on axis as text object, NULL removes it
    void SetY1LabelTextDc( a2dText* yLabelTextDc );

    //! set Y label on axis, empty string removes it
    void SetY2Label( const wxString& ylabel, const wxColour& color = wxNullColour );

    //! set Y label on axis as text object, NULL removes it
    void SetY2LabelTextDc( a2dText* yLabelTextDc );

    //! if true text objects title xlabel and ylabel will be set in position automatically
    void SetAutoPlace( bool autoPlace ) { m_autoPlace = autoPlace; }

    //! if true text ylabel will be set according curves area name
    void SetAutoYAxesNames( bool autoYNames )
    {
        m_autoYNames = autoYNames; SetPending( true );
    }

    //! extra distance around plot
    void SetPadding( double leftPadding, double rightPadding, double topPadding, double bottomPadding );

    DECLARE_DYNAMIC_CLASS( a2dPlot )

protected:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    void SetPosYLabel( a2dText* yLabelTextDc, double x, double y, double tHeight, bool left );

private:

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! padding.
    double m_leftPadding;
    //! padding.
    double m_bottomPadding;
    //! padding.
    double m_topPadding;
    //! padding.
    double m_rightPadding;

    bool m_autoPlace;

    bool m_autoYNames;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPlot( const a2dPlot& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dPlot>;
#endif

//! legend for a group of curves.
/*!
    for all cuvres in a a2dCanvasXYDisplayGroup, display the name in a colums of Text
*/
class A2DCURVESDLLEXP a2dCurveGroupLegend : public a2dCanvasObject
{
public:

    a2dCurveGroupLegend( const wxString& format = _T( "name = %s" ) , a2dCanvasXYDisplayGroupAreas* curveGroup = 0,
                         const a2dFont& font = *a2dDEFAULT_CANVASFONT );

    a2dCurveGroupLegend( const a2dCurveGroupLegend& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dCurveGroupLegend();

    void SetCurveGroup( a2dCanvasXYDisplayGroupAreas* curveGroup ) { m_curveGroup = curveGroup; }

    a2dCanvasXYDisplayGroupAreas* GetCurveGroup() const { return m_curveGroup; }

    void SetFont( const a2dFont& font ) { m_font = font; SetPending( true );}

    //! Set Space in world coordinates between two lines
    inline void SetLineSpacing( double linespace )
    { m_linespace = linespace; SetPending( true ); }
    //! Get Space in world coordinates between two lines
    inline double GetLineSpacing() const { return m_linespace; }

    //! Height in world coordinates of one line
    inline double GetLineHeight() const { return m_font.GetLineHeight(); }

    DECLARE_DYNAMIC_CLASS( a2dCurveGroupLegend )

    DECLARE_EVENT_TABLE()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnChar( wxKeyEvent& event );

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    void DoAddPending( a2dIterC& ic );

    //void OnEnterObject(a2dCanvasObjectEvent &event);

    //void OnLeaveObject(a2dCanvasObjectEvent &event);

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dSmrtPtr<a2dCanvasXYDisplayGroupAreas> m_curveGroup;

    wxString m_format;

    //! legend font
    a2dFont m_font;

    //! space between the lines
    double   m_linespace;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCurveGroupLegend( const a2dCurveGroupLegend& other );
};



#endif


