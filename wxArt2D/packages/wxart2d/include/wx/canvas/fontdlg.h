/*! \file wx/canvas/fontdlg.h
    \brief dialog for choosing fonts
    \author Erik van der Wal

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: fontdlg.h,v 1.7 2008/07/30 21:54:02 titato Exp $
*/

#ifndef __FONTDLG_H__
#define __FONTDLG_H__

#include "wx/artbase/stylebase.h"
#include "wx/canvas/cantext.h"
#include "wx/canvas/sttool.h"

class wxListBox;
class wxTextCtrl;
class wxCheckBox;
class a2dCanvas;
class a2dText;
class a2dSLine;

//! Display a dialog to edit the font search path.
/*! \class a2dFontSearchPathDialog  fontdlg.h editor/fontdlg.h
    \ingroup textrelated
*/
class A2DCANVASDLLEXP a2dFontSearchPathDialog : public wxDialog
{
public:
    //! Constructor.
    a2dFontSearchPathDialog( wxWindow* parent = NULL );
    //! Destructor.
    ~a2dFontSearchPathDialog();
protected:
    //! User interface event handler.
    void OnAdd( wxCommandEvent& event );
    //! User interface event handler.
    void OnDelete( wxCommandEvent& event );
    //! User interface event handler.
    void OnListBox( wxCommandEvent& event );
    //! User interface event handler.
    void OnPathButton( wxCommandEvent& event );
    //! User interface event handler.
    void OnText( wxCommandEvent& event );
    //! User interface event handler.
    void OnOK( wxCommandEvent& event );
protected:
    //! User interface object.
    wxListBox* m_listbox;
    //! User interface object.
    wxTextCtrl* m_textctrl;
    //! User interface object.
    wxButton* m_button;
private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dFontSearchPathDialog )
};

//! Display a font selection dialog.
/*! \class a2dTextPropDlg fontdlg.h editor/fontdlg.h
    \todo EW: Font scaling in the selection dialog. Now it is always scaled to fit in the window, but how to handle font sizes (eg. 0.000001 to 100000) in a consistent way? Maybe do the scaling in magnitudes of 10?
    \ingroup textrelated
*/
class A2DCANVASDLLEXP a2dTextPropDlg : public wxDialog
{
public:
    //! Constructor.
    a2dTextPropDlg( wxWindow* parent = NULL, const a2dFont& currentfont = *a2dDEFAULT_CANVASFONT, unsigned int textflags = a2dText::a2dCANVASTEXT_DEFAULTFLAGS, int alignment = a2dDEFAULT_ALIGNMENT );
    //! Destructor.
    virtual ~a2dTextPropDlg();

    //! Returns the font.
    const a2dFont& GetFontData() const;

    //! Set the position of the anchor point w.r.t the text.
    /*! \param alignment sets the direction of the anchor point. The direction can be set like e.g.
        wxMINX | wxMINY, meaning a specific corner of the boundingbox.
        Not specifying either a horizontal or vertical position will default to the centre. So only
        specifying wxMINX corresponds to the left and middle Y of bbox position.
        If Yaxis is up in a view, use wxBASELINE to have text origin start at (x,y).
        If Yaxis is down in a view, use wxBASELINE_CONTRA to have text origin start at (x,y).
        <!-- view picture using monospaced font -->
        \code
                 +-----^-----+ wxMAXY
                 |      #    |
                 |      #    |
                 | #### #### |
                 |#   # #   #|
                 +-####-#---#+ wxBASELINE
                 |    #      |
                 | ###       |
                 +-----v-----+ wxMINY
              wxMINX      wxMAXX
        \endcode
    */
    inline void SetAlignment( int alignment )
    {
        m_alignment = alignment;
        OnChangeFont();
    }
    //! Get the position of the anchor point w.r.t the text
    /*! \return The alignment setting.
    */
    inline int GetAlignment() const
    { return m_alignment; }

    //! Set the text flags
    inline void SetTextFlags( unsigned int textflags ) 
    { 
        m_textflags = textflags; 
        OnChangeFont();
    }
    //! Get the text flags
    inline unsigned int GetTextFlags() const { return m_textflags; }

    //! User interface event handler.
    void OnChangeFont();
    //! User interface event handler.
    void OnChangeFontType( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontName( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontStyle( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontSize( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontSizeList( wxCommandEvent& event );
    //! User interface event handler.
    void OnSearchPath( wxCommandEvent& event );
    //! User interface event handler.
    void OnAlignment( wxCommandEvent& event );
    //! User interface event handler.
    void OnSize( wxSizeEvent& event );

protected:
    //! Selected font.
    a2dFont m_font;
    //! Font enumeration list.
    a2dFontInfoList m_fonts;
    //! Scale at which the font is viewed.
    double m_viewscale;
    //! Font alignment
    int m_alignment;

    //! contains several text flags.
    unsigned int m_textflags;

    // used initially to select current font.
    a2dFontInfo m_currentinfo;

    //! User interface object.
    wxListBox* m_typelist;
    //! User interface object.
    wxListBox* m_fontlist;
    //! User interface object.
    wxListBox* m_stylelist;
    //! User interface object.
    wxTextCtrl* m_sizeedit;
    //! User interface object.
    wxListBox* m_sizelist;
    //! User interface object.
    wxCheckBox* m_aligncheck;

    wxCheckBox* m_textframe;
    wxCheckBox* m_textbackground;

    //! User interface object.
    a2dCanvas* m_canvas;
    //! Displayed text
    a2dText* m_canvastext;
    //! Horizontal line to indicate alignment
    a2dSLine* m_canvasline1;
    //! Vertical line to indicate alignment
    a2dSLine* m_canvasline2;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dTextPropDlg )
};

#include "wx/canvas/styledialg.h"

//===========================================================================
// a2dTextPropDlgExt - dialog for defining font and style on an a2dCanvasObject:
//===========================================================================
class wxBitmapToggleButton; // forward declaration

#define a2dDEFAULT_FONTTYPE wxT("Freetype font")

/*! Extended font dialog for selection of text objects

	Create the list of fonts (stored in a2dTextChanges) and filter them 
    <code>
	const int TNumFonts = 16;
	const wxString TFonts[TNumFonts] = { wxT("Arial"), wxT("Comic Sans MS"), wxT("Georgia"), wxT("Impact"), wxT("Lucida Sans"), wxT("Palatino Linotype"), wxT("Tahoma"), wxT("Times New Roman"), wxT("Trebuchet MS"), wxT("Verdana"), wxT("Liberation Sans"), wxT("Liberation Serif"), wxT("Liberation Mono"), wxT("DejaVu Sans"), wxT("DejaVu Sans Mono"), wxT("DejaVu Serif") };
	wxArrayString filterlist(TNumFonts, TFonts);
	
	a2dTextChanges::InitialiseFontList(filterlist, DEFAULT_FONTTYPE);		// create the list of available fonts
	//a2dTextChanges::InitialiseFontList();		// create the list of available fonts
    </code>
*/
class a2dTextPropDlgExt : public wxDialog
{
public:
	// bGlobal ... global settings (show info in title bar)
	// bFill ... enable fill combobox
	// fontlist ... list of all available fonts (from a2dFont::GetInfoList( a2dFontInfoList& list ))
	// bAllowLayerStyle ... allows selection for using layer style
	a2dTextPropDlgExt( a2dHabitat* habitat, wxWindow *parent, bool bGlobal, bool bFill, a2dFontInfoList *fontlist, bool bAllowLayerStyle = false);

	// Number that defines the physical dimension (in millimeters) of document units.
	void SetUnitsScale(double scale) { m_UnitsScale = scale; }

	void SetCustomColors(const a2dCustomColors& colors) { m_CustomColors = colors; }
	const a2dCustomColors& a2dGetCustomColors() const { return m_CustomColors; }

	const a2dExtStroke& GetExtStroke() const { return m_ExtStroke; }
	void SetExtStroke(const a2dExtStroke& extStroke) { m_ExtStroke = extStroke; }

	const a2dExtFill& GetExtFill() const { return m_ExtFill; }
	void SetExtFill(const a2dExtFill& extFill) { m_ExtFill = extFill; }

	const a2dTextChanges& GetExtFont() const { return m_ExtFont; }
	void SetExtFont(const a2dTextChanges& extFont) { m_ExtFont = extFont; }

	//! User interface event handler.
    void OnChangeFont();
    //! User interface event handler.
    void OnChangeFontType( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontName( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontStyle( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontSize( wxCommandEvent& event );
    //! User interface event handler.
    void OnChangeFontSizeList( wxCommandEvent& event );
    //! User interface event handler.
    void OnSearchPath( wxCommandEvent& event );
    //! User interface event handler.
    void OnAlignment( wxCommandEvent& event );
    //! User interface event handler.
	 void OnTextflags( wxCommandEvent& event );
    //! User interface event handler.
    void OnSize( wxSizeEvent& event );
	//! User interface event handler.
	void OnColourChange( wxCommandEvent& event );

private:
	void CreateControls(bool bAllowLayerStyle);
	void UpdateStrokeData(bool bGetFromDlg);
	void UpdateFontData();
	//// Sets the correct alignment button
	void SetAlignment(int align);
	// Returns a list of fonts for the given type
	wxArrayString GetFontList(wxString type);
	// Returns the longest style-string of all fonts (needed to initialize the size of style-listbox)
	wxString GetLongestStyle();
	void RepaintSample();

	//========================================================
	//	Event handler:
	//--------------------------------------------------------
	void OnInitDialog(wxInitDialogEvent& event);
	void OnOk(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	bool m_bFill;
	double m_UnitsScale;
	bool m_FontFound;
	wxStringToStringHashMap m_styles;
	a2dCustomColors m_CustomColors;
	a2dExtStroke m_ExtStroke;
	a2dExtFill m_ExtFill;
	a2dTextChanges m_ExtFont;

	wxBoxSizer* m_pTopSizer;
	a2dColorComboBox *m_pStrokeColorBox;
	a2dFillComboBox *m_pFillBox;
	wxBitmapToggleButton *m_pAlignTR_Bt;
	wxBitmapToggleButton *m_pAlignT_Bt;
	wxBitmapToggleButton *m_pAlignTL_Bt;
	wxBitmapToggleButton *m_pAlignR_Bt;
	wxBitmapToggleButton *m_pAlignC_Bt;
	wxBitmapToggleButton *m_pAlignL_Bt;
	wxBitmapToggleButton *m_pAlignBR_Bt;
	wxBitmapToggleButton *m_pAlignB_Bt;
	wxBitmapToggleButton *m_pAlignBL_Bt;

    //a2dFontInfoList m_fonts;
	a2dFontInfoList *m_fonts;
    //! Scale at which the font is viewed.
    double m_viewscale;

    //! User interface object.
    wxListBox* m_typelist;
    //! User interface object.
    wxListBox* m_fontlist;
    //! User interface object.
    wxListBox* m_stylelist;
    //! User interface object.
    wxTextCtrl* m_sizeedit;
    //! User interface object.
    wxListBox* m_sizelist;
    //! User interface object.
    wxCheckBox* m_aligncheck;
	 //! User interface object.
	wxCheckBox* m_textframe;
	 //! User interface object.
    wxCheckBox* m_textbackground;
    //! User interface object.
    a2dCanvas* m_canvas;
    //! Displayed text
    a2dText* m_canvastext;
    //! Horizontal line to indicate alignment
    a2dSLine* m_canvasline1;
    //! Vertical line to indicate alignment
    a2dSLine* m_canvasline2;

    a2dHabitat* m_habitat;
};

extern wxUint32 a2dSetTextChanges( a2dTextChanges& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, a2dLayers* layersetup = NULL );

#endif // __FONTDLG_H__
