/*! \file wx/canvas/styledialg.h
    \brief dialog for defining style on an a2dCanvasObject
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: styledialg.h,v 1.11 2008/08/19 23:17:12 titato Exp $
*/

#ifndef __WDR_styledialg_H__
#define __WDR_styledialg_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/statline.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>
#include <wx/odcombo.h>

#include <wx/colordlg.h>

#include <wx/canvas/strucdlg.h>
#include <wx/artbase/stylebase.h>
#include <wx/canvas/layerinf.h>

// Declare window functions

#define ID_STYLENAME 10000
#define ID_BLUE 10002
#define ID_SLIDER_RED 10007
#define ID_SLIDER_GREEN 10008
#define ID_SLIDER_BLUE 10009
#define ID_SLIDER_ALPHA 10021
#define ID_RED 10004
#define ID_GREEN 10005
#define ID_ALPHA 10020
#define ID_FILLCOLOR 10006
#define ID_STROKECOLOR 10019
#define ID_TEXT 10010
#define ID_PIXELWIDTH 10011
#define ID_PIXELSTROKE 10012
#define ID_STIPPLEBITMAP 10013
#define ID_FILLSTYLE 10015
#define ID_STROKESTYLE 10016
#define ID_COLOURS 10017
#define ID_MODEL_BUTTON_OK 10018

//! general style dialog to edit a2dCanvasObject style
/*!
    The a2dFill, a2dStroke an a2dContour can be modified interactive with this dialog.
    It sends the new style in idle time as commands to the global a2dCentralCanvasCommandProcessor,
    and they end up in the a2dCanvasCommandProcessor of the current document.
    If there is a current a2dCanvasObject is set for the last, the new style will be set to that object.
*/
class A2DCANVASDLLEXP a2dStyleDialog: public wxDialog
{
    DECLARE_EVENT_TABLE()

public:

    a2dStyleDialog( a2dHabitat* habitat, wxWindow* parent, long style = wxDEFAULT_DIALOG_STYLE, bool modal = false, bool setCentralStyle = true );

    ~a2dStyleDialog();

    void SetColor( wxSpinEvent& spinevent );

    void ColorSelect(  wxCommandEvent& event );

    void OnSpinCtrlText( wxCommandEvent& event );

    void OnSliderUpdate( wxScrollEvent& event );

    void OnColourBitmap( wxCommandEvent& event );

    void OnStippleBitmap( wxCommandEvent& event );

    void OnFillStyle( wxCommandEvent& event );

    void OnStrokeStyle( wxCommandEvent& event );

    void OnPixelWidth( wxCommandEvent& event );

    void OnPixelWidthSpin( wxSpinEvent& event );

    void OnPixelStroke( wxCommandEvent& event );

    //! Close window
    void OnCloseWindow( wxCloseEvent& event );

    void OnIdle( wxIdleEvent& idleEvent );

    void SentStyle( bool fill = true, bool stroke = true );

    a2dFill& GetFill() { return m_fill; }
    void SetFill( const a2dFill& fill );

    a2dStroke& GetStroke() { return m_stroke; }
    void SetStroke( const a2dStroke& stroke );

protected:

    void OnUndoEvent( a2dCommandProcessorEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    void OnSize( wxSizeEvent& WXUNUSED( event ) );

    void OnComEvent( a2dComEvent& event );

    void CmOk( wxCommandEvent& event );

    a2dFill m_fill;
    a2dFill m_oneColFill;
    a2dFill m_hatchTwoColFill;
    a2dFill m_PattFill;
    a2dFill m_linGrad;
    a2dFill m_radGrad;
    a2dFill m_dropGrad;

    a2dStroke   m_stroke;
    a2dStroke   m_oneColStroke;
    a2dStroke   m_pattStroke;

    wxColour m_colorFill1;

    wxColour m_colorFill2;

    wxColour m_colorStroke;

    //! the color that is under modification
    wxColour* m_color;

    wxBitmap m_fillstippleb;

    wxBitmapButton* m_colourfillshow;
    wxBitmapButton* m_colourstrokeshow;
    wxBitmapButton* m_fillstipple;

    wxSlider* m_sliderred;

    wxSlider* m_slidergreen;

    wxSlider* m_sliderblue;

    wxSlider* m_slideralpha;

    wxSpinCtrl* m_spinred;

    wxSpinCtrl* m_spingreen;

    wxSpinCtrl* m_spinblue;

    wxSpinCtrl* m_spinalpha;

    //! colour data for colour dialog
    wxColourData*   m_cdata;

    //! color dialog for color bitmap button sample
    wxColourDialog* m_cd;

    FillPatterns* m_fillpatterns;

    wxBitmap     m_fillpatternmono;

    wxChoice* m_fillstyles;

    wxSpinCtrl* m_pixelwidth;

    wxCheckBox* m_pixelstroke;

    wxChoice* m_strokestyles;

    bool m_skipspinevent;

    //! When canvas event, this is set to prevent sending commands.
    bool m_doSentCommand;

    bool m_modal;

    bool m_setCentralStyle;

    bool m_needsUpdate;

    bool m_fillCommand;
    bool m_strokeCommand;

    a2dHabitat* m_habitat;
};


//===========================================================================
// a2dCustomColors
//===========================================================================

class a2dCustomColors
{
public:
	a2dCustomColors();

	void Init();

	void Set(const wxColourData &colData);
	void Get(wxColourData *pColData) const;

	void Set(int i, const wxColour &color);
	wxColour Get(int i) const;

	bool operator==(const a2dCustomColors& rhs);
	bool operator!=(const a2dCustomColors& rhs);

private:
	wxColour m_Color[wxColourData::NUM_CUSTOM];
};


//===========================================================================
// a2dColorComboBox - combobox for selection of a RGB color (or transparent):
//===========================================================================

class a2dColorComboBox : public wxOwnerDrawnComboBox
{
	static const wxArrayString TChoices(bool bAllowTransparent);
public:
	//! pCustCol is an optional pointer to custom colors, which will be modified directly!
	a2dColorComboBox(wxWindow *parent, wxWindowID id, bool bAllowTransparent = true, a2dCustomColors *pCustCol = 0, const wxSize &size = wxDefaultSize);
	virtual ~a2dColorComboBox() {}

	void SetColor(const wxColour &color);
	bool IsColorSelected() const { return (GetSelection() > -1) ? true : false; }
	wxColour GetColor() const { return m_Color; }

	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
	virtual wxCoord OnMeasureItem(size_t item) const;

private:
	void SelectColor();

	//========================================================
	//	Event handler:
	//--------------------------------------------------------
	void OnSelected(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	bool m_bAllowTransparent;
	a2dCustomColors *m_pCustomColors;
	wxColour m_Color;
};


//===========================================================================
// a2dStrokeStyleComboBox - combobox for selection of stroke styles:
//===========================================================================

class A2DCANVASDLLEXP a2dStrokeStyleComboBox : public wxOwnerDrawnComboBox
{
	static const wxArrayString TChoices(bool bAllowLayerStyle);
public:
	a2dStrokeStyleComboBox(wxWindow *parent, wxWindowID id, bool bAllowLayerStyle = false, const wxSize &size = wxDefaultSize);
	virtual ~a2dStrokeStyleComboBox() {}

	void SetStyle(const a2dStrokeStyle &style);
	bool IsStyleSelected() const { return (GetSelection() > -1) ? true : false; }
	a2dStrokeStyle GetStyle() const { return m_Style; }

	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
	virtual wxCoord OnMeasureItem(size_t item) const;

private:
	//========================================================
	//	Event handler:
	//--------------------------------------------------------
	void OnSelected(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	bool m_bAllowLayerStyle;
	a2dStrokeStyle m_Style;
};


//===========================================================================
// a2dFillComboBox - combobox for selection of a fill (or transparent):
//===========================================================================

class A2DCANVASDLLEXP a2dFillComboBox : public wxOwnerDrawnComboBox
{
	static const wxArrayString TChoices(bool bAllowLayerStyle);
public:
	//! pCustCol is an optional pointer to custom colors, which will be modified directly!
	a2dFillComboBox(wxWindow *parent, wxWindowID id, bool bAllowLayerStyle = false, a2dCustomColors *pCustCol = 0, const wxSize &size = wxDefaultSize);
	virtual ~a2dFillComboBox() {}

	void SetFill(const a2dFill &fill);
	bool IsFillSelected() const { return (GetSelection() > -1) ? true : false; }
	a2dFill GetFill() const { return m_Fill; }

	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
	virtual wxCoord OnMeasureItem(size_t item) const;

private:
	void SelectColor();

	//========================================================
	//	Event handler:
	//--------------------------------------------------------
	void OnSelected(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	bool m_bAllowLayerStyle;
	a2dCustomColors *m_pCustomColors;
	a2dFill m_Fill;
};


//===========================================================================
// a2dDialogStyle - dialog for defining style on an a2dCanvasObject:
//===========================================================================
class wxBitmapToggleButton; // forward declaration

/*! simple style dialog
	This style dialog works on a group of selected objects, and what they have in common is shown in the style dialog.
	What is different is blank in the dialog, but can still be set.
*/
class A2DCANVASDLLEXP a2dDialogStyle : public wxDialog
{
public:
	// bGlobal ... global settings (show info in title bar)
	// bFill ... enable fill combobox
	// bAllowLayerStyle ... allows selection for using layer style
	a2dDialogStyle(wxWindow *parent, bool bGlobal, bool bFill, bool bAllowLayerStyle = false);

	// Number that defines the physical dimension (in millimeters) of document units.
	void SetUnitsScale(double scale) { m_UnitsScale = scale; }

	void SetCustomColors(const a2dCustomColors& colors) { m_CustomColors = colors; }
	const a2dCustomColors& GetCustomColors() const { return m_CustomColors; }

	const a2dExtStroke& GetExtStroke() const { return m_ExtStroke; }
	void SetExtStroke(const a2dExtStroke& extStroke) { m_ExtStroke = extStroke; }

	const a2dExtFill& GetExtFill() const { return m_ExtFill; }
	void SetExtFill(const a2dExtFill& extFill) { m_ExtFill = extFill; }

private:
	void CreateControls(bool bAllowLayerStyle);
	void UpdateStrokeData(bool bGetFromDlg);
	void SetJoin(wxPenJoin join);
	void SetCap(wxPenCap cap);

	//========================================================
	//	Event handler:
	//--------------------------------------------------------
	void OnInitDialog(wxInitDialogEvent& event);
	void OnStrokeStyle(wxCommandEvent& event);
	void OnJoin(wxCommandEvent& event);
	void OnCap(wxCommandEvent& event);
	void OnOk(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	bool m_bFill;
	double m_UnitsScale;
	a2dCustomColors m_CustomColors;
	a2dExtStroke m_ExtStroke;
	a2dExtFill m_ExtFill;

	a2dStrokeStyleComboBox *m_pSrokeStyleBox;
	wxBitmapToggleButton *m_pJoinMiterBt;
	wxBitmapToggleButton *m_pJoinBevelBt;
	wxBitmapToggleButton *m_pJoinRoundBt;
	a2dColorComboBox *m_pStrokeColorBox;
	wxBitmapToggleButton *m_pCapButtBt;
	wxBitmapToggleButton *m_pCapProjectingBt;
	wxBitmapToggleButton *m_pCapRoundBt;
	wxSpinCtrlDouble *m_pStrokeWidthCtrl;
	a2dFillComboBox *m_pFillBox;
};

extern wxUint32 a2dSetExtFill( a2dExtFill& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, a2dLayers* layersetup = NULL );
extern wxUint32 a2dSetExtStroke( a2dExtStroke& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, a2dLayers* layersetup = NULL );

#endif

