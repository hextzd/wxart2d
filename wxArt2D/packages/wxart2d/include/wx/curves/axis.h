/*! \file wx/curves/axis.h
    \brief classes for plotting curve and pie data, and editing them.

    Data stored in a derived a2dCanvasObject, can be plotted as a curve of pie chart.
    One can add markers on the curves, and several curves can be plot as a group in one plot.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: axis.h,v 1.13 2008/09/05 19:01:12 titato Exp $
*/

#ifndef __WXAXIS_H__
#define __WXAXIS_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/curves/meta.h"

class A2DCURVESDLLEXP a2dCurveAxis;

//! base class for axis tic formatter
/*!
    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dTicFormatter : public a2dCanvasObject
{

public:
    //! default constructor
    a2dTicFormatter();

    //! destructor
    virtual ~a2dTicFormatter();

    a2dTicFormatter( const a2dTicFormatter& other, CloneOptions options, a2dRefMap* refs );

    //! Set the parent axis that owns this formatter. Used by a2dCurveAxis::SetFormatter()
    void SetAxis( a2dCurveAxis* axis ) { m_axis = axis; }

    //! Set how tic text is formated/displayed.
    virtual void SetTicFormat( const wxString& format ) { m_format = format; }

    //! Default implementation. Returns printf(m_format, dTicValue)
    virtual wxString GetTicText( double dTicValue ) const;

    //! Default tic format
    const wxString& GetTicFormat() const { return m_format;}

    DECLARE_DYNAMIC_CLASS( a2dTicFormatter )

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! parent of formatter
    a2dCurveAxis* m_axis;

    // format string for tic text
    wxString m_format;
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dTicFormatter>;
#endif

class A2DCURVESDLLEXP a2dTimeTicFormatter : public a2dTicFormatter
{

public:
    //!default constructor
    a2dTimeTicFormatter();

    //!destructor
    virtual ~a2dTimeTicFormatter();

    //! Set how tic text is formated/displayed.
    void SetTicFormat( const wxString& format );

    virtual wxString GetTicText( double dTicValue );

    virtual bool CanRender() { return FALSE; }

    virtual void DoRender( a2dIterC& WXUNUSED( ic ), OVERLAP WXUNUSED( clipparent ) ) {}

    DECLARE_DYNAMIC_CLASS( a2dTimeTicFormatter )

};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dTimeTicFormatter>;
#endif

//! base class for axis in a2dCanvasXYDisplayGroup
/*!
    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dCurveAxis: public a2dCurveObject
{
public:

    a2dCurveAxis( double length = 0, bool yaxis = false );

    virtual ~a2dCurveAxis();

    a2dCurveAxis( const a2dCurveAxis& other, CloneOptions options, a2dRefMap* refs );

    //! define which part is displayed on this axis
    void SetBoundaries( double min, double max );

    /*!
        defines how the boundary values are displayed as tic labels this axis.
        i.e this allows a linear translation of aquired data to real messurement
        units (0-100 <==> 0-2500 rpm)

    */
    void SetTicBoundaries( double min, double max );

    double GetTicMin( void ) const { return m_ticmin;}

    double GetTicMax( void ) const { return m_ticmax;}

    //! get real minimum
    /*!
        this returns the lowest boundary value ( min(m_min,m_max) )
    */
    double GetAxisMin() const { return wxMin( m_min, m_max ); }

    //! get real maximum
    /*!
        this returns the highest boundry value ( max(m_min,m_max) )
    */
    double GetAxisMax() const { return wxMax( m_min, m_max ); }

    //! get minimum
    /*!
        this returns the boundary value set as min.
        if m_yaxis is set, this is the figure displayed at lowest Y, else X.
    */
    double GetBoundaryMin() const { return m_min; }

    //! get maximum
    /*!
        this returns the boundary value set as max.
        if m_yaxis is set, this is the figure displayed at highest Y, else X
    */
    double GetBoundaryMax() const { return m_max; }

    //! set length in world coordinates ( not axis coordinates )
    /*!
        the pyhsical extend of this axis on screen
    */
    void SetLength( double length );

    //! get length in world coordinates.
    double GetLength() const { return m_length; }

    //! return true is this axis is Yaxis
    bool IsYAxis() const { return m_yaxis; }

    //! set position of axis ( zero is default )
    /*!
        This value is in the data coordinates system of the a2dCanvasXYDisplayGroup, which uses the axis.
    */
    void SetPosition( double position ) { m_position = position; SetPending( true ); }

    //! get position of axis ( zero is default )
    double GetPosition() const { return m_position; }

    //! show tics
    void SetShowTics( bool showtics ) { m_showtics = showtics; SetPending( true ); }

    //! distance of tic lines in X/Y (curve coords)
    void SetTic ( double d ) { m_tic = d; SetPending( true ); }

    //! get distance of tic lines in X/Y (curve coords)
    double GetTic() const { return ( m_tic == 0 ) ?  ( m_max - m_min ) / 10 : m_tic; }

    double GetTicStart() const;

    //! height of tic lines
    void SetTicHeight ( double height ) { m_ticheight = height; SetPending( true ); }

    double GetTicHeight() const { return ( m_ticheight == 0 ) ?  m_length / 100 : m_ticheight; }

    //! height of tics text set to font
    void SetTicTextHeight ( double height ) { m_font.SetSize( height ); SetPending( true ); }

    //! return size of font
    double GetTicTextHeight() const { return m_font.GetSize(); }

    //! tics display on side of drawing
    void SetSideTic( bool sidetic ) { m_sidetic = sidetic; SetPending( true ); }

    //! Set how tic text is formated/displayed.
    void SetTicFormat( const wxString& format ) { m_pTicFormatter->SetTicFormat( format ); SetPending( true ); }

    //! Set how common tic text is formated/displayed for all (Y-axes).
    void SetCommonTicFormat( const wxString& format ) { m_commonTicFormat = format; SetPending( true ); }

    //! Set a stroke for tics
    void SetTicStroke( const a2dStroke& stroke );

    //! Set a stroke for tics text
    void SetTicTextStroke( const a2dStroke& stroke );

    //! Font for tics.
    void SetFontTic( const a2dFont& font ) { m_font = font;}

    //! Set a stroke for axis
    void SetAxisStroke( const a2dStroke& stroke );

    //!Set units for this axis
    void SetUnits( const wxString& units ) { m_units = units; SetPending( true ); }

    void SetUnitsStroke( const a2dStroke& stroke );

    void SetUnitsStroke( const wxColour& color, double width = 0, a2dStrokeStyle style = a2dSTROKE_SOLID );

    const wxString& GetUnits() const { return m_units; }

    void SetInvertTic( bool inverttic );

    void SetAutoSizedTic( bool autosized ) { if ( m_autosizedtic != autosized ) { m_autosizedtic = autosized; SetPending( true ); } }

    virtual void AddLevel( const wxString& levelName, double val, a2dCanvasObject* object );

    a2dCanvasObject* GetLevel( const wxString& levelName );

    bool GetLevelValue( const wxString& levelName, double& val );

    void MirrorLevels();

    double ConvertWorld2Axis( double RelativeWorldValue );

    double ConvertAxis2World( double AxisValue );

    //! Replace current tic formatter
    void SetFormatter( a2dTicFormatter* pFormatter );

    //! Get current tic formatter
    a2dTicFormatter* GetFormatter() { return m_pTicFormatter; }

    DECLARE_DYNAMIC_CLASS( a2dCurveAxis )

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool HasLevelMarkers() const;

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    void DoUpdateViewDependentObjects( a2dIterC& ic );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

protected:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCurveAxis( const a2dCurveAxis& other );

    //! length in parent world coordinates
    double m_length;

    //! minimum on axis (internal boundaries)
    double m_min;
    //! maximum on axis (internal boundaries)
    double m_max;

    //! tic scale minimum on axis
    double m_ticmin;
    //! tic scale maximum on axis
    double m_ticmax;

    //! position of axis
    double m_position;

    //! is this an Y axis
    bool m_yaxis;

    //! if true tics are displayed
    bool m_showtics;

    // distance of tics
    double m_tic;

    // tic height
    double m_ticheight;

    // use side tic
    bool m_sidetic;

    // use another side for tics
    bool m_inverttic;

    // use to set auto size for tics
    bool m_autosizedtic;

    //! tic font
    a2dFont m_font;

    // units for this axis
    wxString m_units;

    a2dStroke m_stroketic;
    a2dStroke m_stroketictext;
    a2dStroke m_strokeaxis;

    a2dStroke m_strokeunits;

    // current formatter
    a2dSmrtPtr<a2dTicFormatter> m_pTicFormatter;

    // common format string for all Y-axes
    wxString m_commonTicFormat;

    mutable a2dBoundingBox m_untransbboxtictext;
    /*
        static const a2dPropertyIdRefObject PROPID_stroketic;
        static const a2dPropertyIdRefObject PROPID_stroketictext;
        static const a2dPropertyIdRefObject PROPID_strokeaxis;
    */
};


#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCurveAxis>;
#endif

//! base class for axis in a2dCanvasXYDisplayGroup
/*!
    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dCurveAxisLin: public a2dCurveAxis
{
public:

    a2dCurveAxisLin( double length = 0, bool yaxis = false );

    virtual ~a2dCurveAxisLin();

    a2dCurveAxisLin( const a2dCurveAxisLin& other, CloneOptions options, a2dRefMap* refs );

    DECLARE_DYNAMIC_CLASS( a2dCurveAxisLin )

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCurveAxisLin( const a2dCurveAxisLin& other );

};


#if 0
//! base class for axis area in a2dCanvasXYDisplayGroup
/*!
    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dCurveAxisArea: public a2dCurveObject
{
public:

    a2dCurveAxisArea();

    virtual ~a2dCurveAxisArea();

    a2dCurveAxisArea( const a2dCurveAxisArea& other, CloneOptions options, a2dRefMap* refs );

    a2dCurveAxis* GetAxis( const wxString axisname );
    a2dCurveAxis* GetAxis( int nIndex );

    //! set length in world coordinates ( not axis coordinates )
    /*!
        the pyhsical extend of this axis on screen
    */
    void SetLength( double length );

    //! get length in world coordinates.
    double GetLength() { return m_length; }

    DECLARE_CLASS( a2dCurveAxisArea )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

protected:

    //! length in parent world coordinates
    double m_length;

};
#endif // ifdef 0

#endif

