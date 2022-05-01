/*! \file cantext.h
    \brief a2dText derived from a2dCanvasObject, holds multiline text
    \author Klaas Holwerda, Erik van der Wal

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cantext.h,v 1.29 2009/07/16 20:06:33 titato Exp $
*/

#ifndef __WXCANTEXT_H__
#define __WXCANTEXT_H__

#include "wx/canvas/canobj.h"

extern wxString TextAlignment2String( int alignment );
extern int AlignmentString2TextAlignment( const wxString& alignstring );

/*! \defgroup textrelated Text related classes
*/

//! a2dText offers text display and editing on a canvas.
/*!
    The text is filled with a rectangular background if a2dFill != *a2dTRANSPARENT_FILL
    The fill colour is the Colour of the a2dFill.
    The a2dStroke colour is the colour used for the text itself.

    The text can be aligned, see SetAlignment() for info.
    \par User editing keys:

<TABLE border=0 cellpadding=0 cellspacing=0>
<tr><td colspan=2><br><b>Cursor movement</b></td></tr><br>
<tr><td colspan=2>Text selection can be done by pressing the Shift key, simultaneously with one of the following keys.<br></td></tr><br>

<TR><TD class="memItemLeft" nowrap>Cursor up/down</TD><TD class="memItemRight"> Move cursor one line up/down.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Cursor Left / Ctrl + B</TD><TD class="memItemRight"> Move cursor one character to the left.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Cursor Right / Ctrl + F</TD><TD class="memItemRight"> Move cursor one character to the right.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Ctrl + Left / Alt + B</TD><TD class="memItemRight"> Move cursor to start of (previous) word.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Ctrl + Right / Alt + F</TD><TD class="memItemRight"> Move cursor to start of next word.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Home / Ctrl + A</TD><TD class="memItemRight"> Move to start of line</TD></TR>
<TR><TD class="memItemLeft" nowrap>End / Ctrl + E</TD><TD class="memItemRight"> Move to end of line</TD></TR>
<TR><TD class="memItemLeft" nowrap>Ctrl + N</TD><TD class="memItemRight"> Move to start of next line.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Ctrl + P</TD><TD class="memItemRight"> Move to start of previous line.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Ctrl + Home</TD><TD class="memItemRight"> Move to start of text.</TD></TR>
<TR><TD class="memItemLeft" nowrap>Ctrl + End</TD><TD class="memItemRight"> Move to end of text.</TD></TR>

<TR><TD colspan=2><br><b>Deletion</b></TD></TR><br>
<TR><TD colspan=2>If part of the text is selected, deleting it will take precedence over the following keys.<br></TD></TR><br>

<TR><TD class="memItemLeft" nowrap>Backspace / Ctrl + H</TD><TD class="memItemRight"> Delete previous character.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Delete / Ctrl + D</TD><TD class="memItemRight"> Delete next character.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + Backspace / Ctrl + W</TD><TD class="memItemRight"> Delete to start of (previous) word.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + Delete / Alt + D</TD><TD class="memItemRight"> Delete to start of next word.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + K</TD><TD class="memItemRight"> Delete to end of line.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + U</TD><TD class="memItemRight"> Delete current line</TD></TR><br>

<TR><TD colspan=2><br><b>Clipboard<br></b></TD></TR><br>

<TR><TD class="memItemLeft" nowrap>Ctrl + C / Ctrl + Insert</TD><TD class="memItemRight"> Copy text to clipboard.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + V / Shift + Insert</TD><TD class="memItemRight"> Paste text from clipboard.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + X / Shift + Delete</TD><TD class="memItemRight"> Cut text to clipboard.</TD></TR><br>

<TR><TD colspan=2><br><b>Text alignment</b></TD></TR><br>
<TR><TD colspan=2>Num lock should be on to use these keys.<br></TD></TR><br>

<TR><TD class="memItemLeft" nowrap>Ctrl + Keypad 1..9</TD><TD class="memItemRight"> Align the text to a different direction.</TD></TR><br>
<TR><TD class="memItemLeft" nowrap>Ctrl + Keypad 0</TD><TD class="memItemRight"> Toggle between bounding box and text alignment.</TD></TR><br>

</TABLE>

    Proper constructors for several cases.
    \code
        a2dText( text, x, y, a2dFont( fontfile, size ) );

        a2dText( text, x, y, a2dFont( wxfont, size, alignment), angle, up );


        a2dText( text, x, y, a2dFont( wxfont, size, alignment), 0.0, up );

        text->SetFont( a2dFont( wxfont, text->GetTextHeight(), text->GetAlignment() ) );

        text->SetFont( a2dFont( wxfont, size, text->GetAlignment() ) );
    \endcode

    \ingroup canvasobject
    \ingroup textrelated
*/

class A2DCANVASDLLEXP a2dText: public a2dCanvasObject
{
public:
    enum a2dTextFlags
    {
        //! Allows multiple lines of text
        a2dCANOBJTEXT_MULTILINE = 1 << 1,
        //! Todo: The text will not be user-editable, but is selectable
        a2dCANOBJTEXT_READONLY = 1 << 2,
        //! show the caret
        a2dCANOBJTEXT_CARETVISIBLE = 1 << 3,
        //! next line is higher in Y if set
        a2dCANOBJTEXT_NEXTLINEUP = 1 << 4,
        //! Text is always readible from the bottom right.
        a2dCANOBJTEXT_ENGINEERING = 1 << 5,
        //! disable background drawing
        a2dCANOBJTEXT_BACKGROUND = 1 << 6,
        //! allow draw a frame rect around a text
        a2dCANOBJTEXT_FRAME      = 1 << 7,
        a2dCANVASTEXT_DEFAULTFLAGS = a2dCANOBJTEXT_MULTILINE
                                     | a2dCANOBJTEXT_CARETVISIBLE
                                     | a2dCANOBJTEXT_NEXTLINEUP
    };

public:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dText )

    //! constructor
    /*!
        create text object at (0,0) text is set to "not specified" if not given
    */
    a2dText( const wxString& text = wxT( "not specified" ) );

    //! constructor
    /*!
        \param text text to display
        \param x x start
        \param y y start
        \param font a2dFont to use
        \param angle angle in degrees
        \param up if true next line is higher in Y else lower
        \param alignment Alignment of the text. (See SetAlignment)
    */
    a2dText( const wxString& text, double x, double y,
             const a2dFont& font = *a2dDEFAULT_CANVASFONT, double angle = 0.0,
             bool up = false, int alignment = a2dDEFAULT_ALIGNMENT );

protected:
    //! copy constructor
    a2dText( const a2dText& other, CloneOptions options, a2dRefMap* refs );

public:
    //! Destructor
    ~a2dText();

    inline a2dText* TClone( CloneOptions options, a2dRefMap* refs = NULL ) { return ( a2dText* ) Clone( options, refs ); }

public:

    //! get the number of lines in the text
    int GetLines() const;

    //! get lines into array, return number of lines
    int GetAsArray( wxArrayString& array );

    //! get the number of lines in the text
    wxString GetLine( int line ) const;

	wxString GetSelectedText() const;

    void DeleteSelectedText();

    //! set the text for the object '\n' in string means new line
    inline void SetText( const wxString& text )
    { m_text = text; m_utbbox_changed = true; SetPending( true ); }
    //! get the text of the object '\n' in string means new line
    inline wxString GetText() const { return m_text; }

    //! if selction is active, replace selection with given text, else insert text at carret position. 
	void InsertTextAtCarretOrSelection( const wxString& text );

	void InsertTextAtCarret( const wxString& text );

    //! set text height in world coordinates
    inline void SetTextHeight( double height )
    { m_font.SetSize( height ); m_utbbox_changed = true; SetPending( true ); }
    //! get text height in world coordinates
    inline double GetTextHeight() const { return m_font.GetSize(); }

    //! Height in world coordinates of one line
    inline double GetLineHeight() const { return m_font.GetLineHeight(); }

	//! Set the flag if text couldn't be loadad from file
	inline void SetWrongLoad( bool wrongLoad ) { m_wrongLoad = wrongLoad; }
	//! Get flag if text couldn't be loadad from file
	inline bool GetWrongLoad() const { return m_wrongLoad; }

    //! Set Space in world coordinates between two lines
    inline void SetLineSpacing( double linespace )
    { m_linespace = linespace; m_utbbox_changed = true; SetPending( true ); }
    //! Get Space in world coordinates between two lines
    inline double GetLineSpacing() const { return m_linespace; }

    //! Set the text flags
    inline void SetTextFlags( unsigned int textflags ) { m_textflags = textflags; m_utbbox_changed = true; }
    //! Get the text flags
    inline unsigned int GetTextFlags() const { return m_textflags; }

    //! Set if multiple lines of text are allowed.
    void SetMultiLine( bool multiline = true );
    //! True if multiple lines of text are allowed.
    inline bool GetMultiLine() const
    { return ( m_textflags & a2dCANOBJTEXT_MULTILINE ) != 0; }

    //! Set if text is readonly
    void SetReadOnly( bool readonly = true );
    //! True if text is readonly.
    inline bool GetReadOnly()
    { return ( m_textflags & a2dCANOBJTEXT_READONLY ) != 0; }

    //! next line is higher in Y if true or lower if false
    void SetNextLineDirection( bool up );
    //! next line is higher in Y if true or lower if false
    inline bool GetNextLineDirection() const
    { return ( m_textflags & a2dCANOBJTEXT_NEXTLINEUP ) != 0; }

    //! Keep text readible from the bottom right.
    void SetEngineeringText( bool engineering = true );
    //! Is text always readible from the bottom right.
    inline bool GetEngineeringText() const
    { return ( m_textflags & a2dCANOBJTEXT_ENGINEERING ) != 0; }

    //! Set if a background will be drawn.
    /*!
        When a layer style is used with a non transparent fill,
        overlapping text is disturbing in case background overlaps other text.
        By setting this flag false the background is set transparent, even if a private fill style is used.
        Of course one can set the object to use a transparent fill, but this takes memory.
    */
    void SetBackGround( bool background = true );
    //! \see SetBackGround()
    inline bool GetBackGround() const
    { return ( m_textflags & a2dCANOBJTEXT_BACKGROUND ) != 0; }

    //! Set if a frame rect will be drawn.
    /*!

    */
    void SetDrawFrame( bool frame = true );
    //! \see SetDrawFrame()
    inline bool GetDrawFrame() const
    { return ( m_textflags & a2dCANOBJTEXT_FRAME ) != 0; }

    //! set font for text
    /*! This class takes care of deleting the font
        \param font The font to set
        \param lineSpaceFactor The m_linespace will be set to font.GetSize() * lineSpaceFactor
    */
    void SetFont( const a2dFont& font, double lineSpaceFactor = 0.1 );
    //! get font for text
    /*! Returns a pointer to the current font. If you need a copy, use (a2dFont) GetFont()->Clone(flat_copy).
    */
    inline a2dFont GetFont() const { return m_font; }

    //! set position of caret (-1 means off)
    inline void SetCaret( int position )
    { m_caret = position; SetPending( true ); }
    //! Get position of caret (-1 means off)
    inline int GetCaret() const { return m_caret; }

    //! Set caret to the character closest to the position
    /*! \param ic iterative context
        \param x (mouse pointer) x-coordinate in absolute world coordinates
        \param y (mouse pointer) y-coordinate in absolute world coordinates
        \return True if mouse-position was within this object
    */
    bool SetCaret( a2dIterC& ic, double x, double y );

    //! set caret on or off
    void SetCaretShow( bool visible = true );
    //! return true if caret is visible
    inline bool GetCaretShow() const
    { return ( m_textflags & a2dCANOBJTEXT_CARETVISIBLE ) != 0; }

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
        if( m_alignment != alignment )
        {
            m_alignment = alignment; m_utbbox_changed = true; SetPending( true );
        }
    }
    //! Get the position of the anchor point w.r.t the text
    /*! \return The alignment setting.
    */
    inline int GetAlignment() const
    { return m_alignment; }

    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );

    bool AdjustPinLocation();

    virtual a2dCanvasObjectList* GetSnapVpath( a2dSnapToWhatMask snapToWhat );

    //! return text as a vector path, where text outline is converted to polyline or polygons.
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

#if wxART2D_USE_CVGIO
    //! Write object specific CVG data
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    //! Load object specific CVG data
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    static a2dPropertyIdFont* PROPID_Font;
    //! old Text property, to return text as it was before editing (use in Undo)
    static a2dPropertyIdString* PROPID_PreEditText;

    DECLARE_PROPERTIES()

protected:

    //! Clone this object and return a pointer to the new object.
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! Render derived object
    virtual void DoRender( a2dIterC& ic, OVERLAP clipparent );

    virtual bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    //! Get boundingbox without the affine matrix transform included.
    /*! \return The boundingbox.
    */
    virtual a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //! Set caret to the character closest to the position
    /*!
        \param xh (mouse pointer) x-coordinate in world coordinates
        \param yh (mouse pointer) y-coordinate in world coordinates
    */
    void SetCaretUntransformed( double xh, double yh );

    //! Calculate a new affine, so text is always readible from the bottom right.
    a2dAffineMatrix GetEngineeringTransform( a2dIterC& ic );

    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    virtual void DoEndEdit();

    void OnChar( wxKeyEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    const a2dFont& MX_GetFont() const { return m_font; }
    void MX_SetFont( const a2dFont& value ) { m_font = value; }

    //! font
    a2dFont m_font;

	a2dFontInfo m_fontinfo;

	bool m_wrongLoad;
	
    //! space between the lines
    double   m_linespace;

    //! the text to display
    wxString m_text;

    //! contains several text flags.
    unsigned int m_textflags;

    //! position of caret within text string
    int m_caret;

    //! X position of caret in world coordinate in line
    double m_XcaretinLine;

    //! Y position of caret in world coordinate in line
    double m_YcaretinLine;

    //! Untransformed bounding box.
    mutable a2dBoundingBox m_untransbbox;

    //! Untransformed bounding box changed
    mutable bool m_utbbox_changed;

    //! Selection starting position
    size_t m_selection_start_pos;
    //! Selection end position
    size_t m_selection_end_pos;

    //! The first editable character, usually 0, but may be different for a2dVisibleProperty
    size_t m_firsteditable;

    //! Alignment.
    int m_alignment;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dText( const a2dText& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dText>;
#endif

//
// enumerates used by the GDS text class
//
enum TEXT_PATHTYPE {TEXT_PATH_END_SQAURE, TEXT_PATH_END_ROUND, TEXT_PATH_END_SQAURE_EXT};

//
// enumerates used by the Presentation class
//
enum {FONT0, FONT1, FONT2, FONT3};
enum {TOP, MIDDLE, BOTTOM};
enum {LEFT, CENTER, RIGTH};

//! how a GDS-II object is placed relative to its XY position.
typedef struct pres_bitflags
{

#if wxBYTE_ORDER == wxBIG_ENDIAN
    unsigned    m_reserved  :    10;
    unsigned    m_font:      2;
    unsigned    m_vertical:      2;
    unsigned    m_horizontal: 2;
#else
    unsigned    m_horizontal: 2;
    unsigned    m_vertical:      2;
    unsigned    m_font:      2;
    unsigned    m_reserved  :    10;
#endif
} pres_bitflags;

//
// defaults for Presentation
//
#define DEFAULT_PRESENTATION_FONT           FONT0
#define DEFAULT_PRESENTATION_VERTICAL       TOP
#define DEFAULT_PRESENTATION_HORIZONTAL LEFT
#define DEFAULT_FONT_HEIGHT                 1


//! a2dTextGDS text based on wxDC text drawing.
/*!
    The text is filled with a background if a2dFill != *a2dTRANSPARENT_FILL
    The fill colour is the Colour of the a2dFill.
    The a2dStroke colour is the colour used for the text itself.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dTextGDS: public a2dText
{

public:

    //! constructor
    /*!
        create text object at (0,0) text is set to "not specified" if not given
    */
    a2dTextGDS( a2dFontType fontType = a2dFONT_STROKED, const wxString& text = wxT( "not specified" ), double height = 1.0, short int presentation = 0 );

    //! constructor
    a2dTextGDS( const a2dTextGDS& ori, CloneOptions options, a2dRefMap* refs );

    void SetFontGDS( char nw_font = DEFAULT_PRESENTATION_FONT );
    void SetVertical( char nw_vertical = DEFAULT_PRESENTATION_VERTICAL );
    void SetHorizontal( char nw_horizontal = DEFAULT_PRESENTATION_HORIZONTAL );
    short int GetFontGDS();
    short int GetVertical();
    short int GetHorizontal();

    void SetRotation( double rotation );
    double GetRotation();

    //! GDSII compatible to sub identify types of rectangles.
    /*! you can use it as a special tagged text */
    void SetTextType( wxUint16 type ) { m_texttype = type; }

    //! GDSII compatible to sub identify types of rectangles.
    /*! you can use it as a special tagged text */
    wxUint16 GetTextType() { return m_texttype; }

    void SetPresentationFlags( short int total );

    short int GetPresentationFlags( );

    void SetPathtype( TEXT_PATHTYPE type ) { m_pathtype = type; }

    TEXT_PATHTYPE GetPathtype() { return m_pathtype; }

    //! a factor to modify the internal text height, compared to the GDSII file height.
    static void SetFontScaleFactor( double fontScaleFactor ) { m_fontScaleFactor = fontScaleFactor; }

    //! a factor to modify the internal text height, compared to the GDSII file height.
    static double GetFontScaleFactor() { return m_fontScaleFactor; }

protected:

#if wxART2D_USE_CVGIO
    //! Write object specific CVG data
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    //! Load object specific CVG data
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //! Clone this object and return a pointer to the new object.
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! GDSII compatible to sub identify types of text.
    wxUint16 m_texttype;

    a2dFontType m_fontType;

    TEXT_PATHTYPE m_pathtype;

    float m_width;

    bool m_monoSpaced;

    static double m_fontScaleFactor;

    //! presentation flags \see pres_bitflags
    union
    {
        short int total;
        pres_bitflags bits;
    } m_presentationflags;

    DECLARE_DYNAMIC_CLASS( a2dTextGDS )
};


/*! a2dTextChanges - register a2dFont and other text properties 
on a group of a2dText objects.
The class is used to set only certain properties on a2dText objects, and leaf the rest as is.
For example to change the alignment of a group of selected a2dText objects, but leaf the font settings for the rest as is.

*/
class a2dTextChanges
{
public:
	a2dTextChanges();
	a2dTextChanges(const a2dFont& font, unsigned int textflags, int alignment );

	// Creates the list of available fonts once
	// Optional: filter fonts, if a filled filterlist and filtertype is passed
	static void InitialiseFontList(const wxArrayString &filterlist = wxArrayString(), const wxString filtertype = wxT( "" ));

    // Initialises with <font>.
	void Set(const a2dFont& font, unsigned int textflags, int alignment, bool wrongloadflag = false );

	// Compares with <font> and sets to invalid if unequal.
	void Mix(const a2dFont& font, unsigned int textflags, int alignment );

	// Returns the a2dFont member object if valid, or <font> instead.
	a2dFont GetFont(const a2dFont& font, wxArrayString valid_styles = wxArrayString(), wxArrayString valid_extras = wxArrayString() ) const;
	int GetAlignment( int Alignment ) const;
    unsigned int GetTextFlags( unsigned int textflags ) const;


	bool IsValid() const { return m_bFont; }
	// Returns the a2dFont member object.
	a2dFont GetFont() const { return m_Font; }
	void SetFont(a2dFont font)
	{
		m_Font = font;
		m_bFont = true;
	}

	//! Set the font type
	inline void SetType(const wxString& type)
	{
		a2dFontInfo fontinfo = m_Font.GetFontInfo();
		fontinfo.SetType( type );
        m_Font = a2dFont::CreateFont( fontinfo );

		m_bType = true;
	}

	//! Set the font style
	inline void SetStyle(const wxString& style)
	{
		a2dFontInfo fontinfo = m_Font.GetFontInfo();
		fontinfo.SetStyle( style );
        m_Font = a2dFont::CreateFont( fontinfo );

		m_bStyle = true;
	}

	//! Set the font name
	inline void SetName(const wxString& name)
	{
		a2dFontInfo fontinfo = m_Font.GetFontInfo();
		fontinfo.SetName( name );
        m_Font = a2dFont::CreateFont( fontinfo );

		m_bName = true;
	}

	//! Set the size
	inline void SetSize(double size)
	{
		a2dFontInfo fontinfo = m_Font.GetFontInfo();
		fontinfo.SetSize( size );
        m_Font = a2dFont::CreateFont( fontinfo );

		m_bSize = true;
	}

	 //! Set the text flags
    inline void SetTextFlags( unsigned int textflags ) 
	{ 
		m_textflags = textflags; 
		m_btextflags = true;
	}

	//! Set the position of the anchor point w.r.t the text.
    inline void SetAlignment( int alignment )
    {
        m_alignment = alignment;
		m_balignment = true;
    }
    
	bool IsTypeValid() const { return m_bType; }
	bool IsNameValid() const { return m_bName; }
	bool IsStyleValid() const { return m_bStyle; }
	bool IsSizeValid() const { return m_bSize; }
	bool IsAlignmentValid() const { return m_balignment; }
	bool IsTextflagsValid() const { return m_btextflags; }

	void SetStyleValidity( bool valid ) { m_bStyle = valid; }
	void SetNameValidity( bool valid ) { m_bName = valid; }

	//! Get the text flags
    inline unsigned int GetTextFlags() const { return m_textflags; }

    //! Get the position of the anchor point w.r.t the text
    /*! \return The alignment setting.
    */
    inline int GetAlignment() const
    { return m_alignment; }

	a2dFontInfoList *GetFontInfoList()
	{ return &m_fontlist; }

	wxArrayString GetCommonStyle()
	{ return m_commonstyle; }

private:
	a2dFont m_Font;
	bool m_bFont;

	//! Font flags
	bool m_bName;
	bool m_bType;
	bool m_bStyle;
	bool m_bSize;

    //! Font alignment
    int m_alignment;
	bool m_balignment;

    //! contains several text flags.
    unsigned int m_textflags;
    bool m_btextflags;

	//! contains the comlete font list
	static a2dFontInfoList m_fontlist;

	//! contains the common styles when objects are mixed
	wxArrayString m_commonstyle;

	//! for speed optimization, hold a list of already used fonts
	static a2dFontInfoList m_alreadyusedfonts;
};

extern wxUint32 SetTextChanges( a2dTextChanges& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, a2dLayers* layersetup = NULL );


#endif /* __WXCANTEXT_H__ */
