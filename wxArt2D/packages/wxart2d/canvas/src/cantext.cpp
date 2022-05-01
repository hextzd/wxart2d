/*! \file canvas/src/cantext.cpp
    \author Klaas Holwerda, Erik van der Wal

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cantext.cpp,v 1.56 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/clipbrd.h>

#include "wx/canvas/cantext.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/canglob.h"

IMPLEMENT_DYNAMIC_CLASS( a2dText, a2dCanvasObject )


//! convert alignment enum to a string
wxString TextAlignment2String( int alignment )
{
     wxString alignstring;
    if ( alignment & wxMINX )
        alignstring += wxT( "wxMINX|" );
    if ( alignment & wxMAXY )
        alignstring += wxT( "wxMAXY|" );
    if ( alignment & wxMAXX )
        alignstring += wxT( "wxMAXX|" );
    if ( alignment & wxMINY )
        alignstring += wxT( "wxMINY|" );
		if ( alignment & wxMIDX )
        alignstring += wxT( "wxMIDX|" );
		if ( alignment & wxMIDY )
        alignstring += wxT( "wxMIDY|" );
    if ( alignment & wxBASELINE )
        alignstring += wxT( "wxBASELINE|" );
    if ( alignment & wxBASELINE_CONTRA )
        alignstring += wxT( "wxBASELINE_CONTRA|" );
    if ( !alignstring.empty() && alignstring.Last() == wxChar( '|' ) )
        alignstring.RemoveLast();

	return alignstring;
}

//! convert a string to alignment enum
int AlignmentString2TextAlignment( const wxString& alignstring )
{
    int alignment = 0;
    if ( alignstring.Find( wxT( "wxMINX" ) ) != -1 )
        alignment |= wxMINX;
    if ( alignstring.Find( wxT( "wxMAXX" ) ) != -1 )
        alignment |= wxMAXX;
    if ( alignstring.Find( wxT( "wxMINY" ) ) != -1 )
        alignment |= wxMINY;
    if ( alignstring.Find( wxT( "wxMAXY" ) ) != -1 )
        alignment |= wxMAXY;
	if ( alignstring.Find( wxT( "wxMIDX" ) ) != -1 )
        alignment |= wxMIDX;
	if ( alignstring.Find( wxT( "wxMIDY" ) ) != -1 )
        alignment |= wxMIDY;
	if ( alignstring.Find( wxT( "wxBASELINE_CONTRA" ) ) != -1 )
        alignment |= wxBASELINE_CONTRA;
    else if ( alignstring.Find( wxT( "wxBASELINE" ) ) != -1 )
        alignment |= wxBASELINE;

	return alignment;
}

//----------------------------------------------------------------------------
// a2dText
//----------------------------------------------------------------------------

#define CaretOnTheFLy
#define DRAGRECTWIDTH 5

a2dPropertyIdFont* a2dText::PROPID_Font = NULL;
a2dPropertyIdString* a2dText::PROPID_PreEditText = NULL;

INITIALIZE_PROPERTIES( a2dText, a2dCanvasObject )
{
    PROPID_Font = new a2dPropertyIdFont( wxT( "Font" ),
                                         a2dPropertyId::flag_transfer | a2dPropertyId::flag_clonedeep,
                                         *a2dNullFONT,
                                         static_cast < a2dPropertyIdFont::ConstGet >( &a2dText::MX_GetFont ),
                                         static_cast < a2dPropertyIdFont::ConstSet >( &a2dText::MX_SetFont ) );
    AddPropertyId( PROPID_Font );

    PROPID_PreEditText = new a2dPropertyIdString(  wxT( "PreEditText" ), a2dPropertyId::flag_temporary, wxT("") );
    AddPropertyId( PROPID_PreEditText );

    return true;
}

BEGIN_EVENT_TABLE( a2dText, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dText::OnCanvasObjectMouseEvent )
    EVT_CHAR( a2dText::OnChar )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dText::OnHandleEvent )
END_EVENT_TABLE()

a2dText::a2dText( const wxString& text )
{
    m_text = text;
    m_font = *a2dDEFAULT_CANVASFONT;
    m_alignment = a2dDEFAULT_ALIGNMENT;
    m_linespace = m_font.GetExternalLead();
    m_caret = -1;
    m_selection_start_pos = m_selection_end_pos = 0;
    m_firsteditable = 0;
    m_textflags = a2dCANVASTEXT_DEFAULTFLAGS;
    m_utbbox_changed = true;
	m_wrongLoad = false;
}

a2dText::a2dText( const wxString& text, double x, double y,
                  const a2dFont& font, double angle, bool up, int alignment )
{
    m_text = text;
    m_font = font;
    m_alignment = alignment;
    m_linespace = font.GetExternalLead(); //font.GetSize() / 10.0;
    m_caret = -1;
    m_selection_start_pos = m_selection_end_pos = 0;
    m_firsteditable = 0;
    m_textflags = a2dCANVASTEXT_DEFAULTFLAGS;
    SetNextLineDirection( up );
    Rotate( angle );
    m_lworld.Translate( x, y );
    m_utbbox_changed = true;
	m_wrongLoad = false;
}

a2dText::a2dText( const a2dText& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_text = other.m_text;
    m_linespace = other.m_linespace;
    m_font = other.m_font;
    m_alignment = other.m_alignment;
    m_textflags = other.m_textflags;
	m_wrongLoad = other.m_wrongLoad;
	m_fontinfo = other.m_fontinfo;

    m_caret = -1;
    m_selection_start_pos = m_selection_end_pos = 0;
    m_firsteditable = other.m_firsteditable;

    //in a clone pending flag and bbox state is taken over, and therefore this
    //box needs to be taken too, else it would not be calculated when an object
    //its bbox is already valid via this clone.
    m_untransbbox = other.m_untransbbox;
    m_utbbox_changed = other.m_utbbox_changed;
}

a2dText::~a2dText()
{
}

a2dObject* a2dText::DoClone( CloneOptions options, a2dRefMap* refs ) const
{ 
    return new a2dText( *this, options, refs ); 
}


a2dCanvasObjectList* a2dText::GetSnapVpath( a2dSnapToWhatMask snapToWhat )
{
    a2dVpath* segments = new a2dVpath();
    a2dVectorPath* snappath = new a2dVectorPath( segments );
    a2dCanvasObjectList* snappathlist = new a2dCanvasObjectList();
    snappathlist->push_back( snappath );

    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
            snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected
       )
    {
        if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_hasPins )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                a2dPin* pin = wxDynamicCast( obj, a2dPin );
                if ( pin && !pin->GetRelease( ) )
                {
                    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
                            ( snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected  && !pin->IsConnectedTo() )
                       )
                    {
                        a2dVpathSegment* seg = new a2dVpathSegment( pin->GetAbsX(), pin->GetAbsY(), a2dPATHSEG_MOVETO );
                        segments->Add( seg );
                    }
                }
            }
        }
    }
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectPos )
    {
        a2dVpathSegment* seg = new a2dVpathSegment( GetPosX(), GetPosY(), a2dPATHSEG_MOVETO );
        segments->Add( seg );
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes || snapToWhat & a2dRestrictionEngine::snapToBoundingBox )
    {
        a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );
        if ( untrans.GetWidth() == 0 || untrans.GetWidth() == 0 )
            untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN | a2dCANOBJ_BBOX_EDIT );

        double x, y, w, h;
        x = untrans.GetMinX();
        y = untrans.GetMinY();
        w = untrans.GetWidth();
        h = untrans.GetHeight();

        double tx, ty;
        m_lworld.TransformPoint( x , y, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x , y + h / 2, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w / 2 , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y + h / 2, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w / 2 , y, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
    }

    return snappathlist;
}

a2dCanvasObjectList* a2dText::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();

    //if ( GetEngineeringText() )
    //    ic.GetDrawer2D()->PushTransform( GetEngineeringTransform( ic ) );

    int up = GetNextLineDirection() ? 1 : -1;
    size_t linestart;
    size_t lineend = ( size_t ) 0;
    size_t textlength = m_text.Length();
    int line = 0;
    wxString linetext;
    while ( lineend < textlength )
    {
        linestart = lineend;
        while ( lineend < textlength && m_text[lineend] != wxChar( '\n' ) )
            lineend++;
        linetext = m_text.Mid( linestart, lineend - linestart );

        a2dVpath* segments = new a2dVpath();

        GetFont().GetVpath( linetext, *segments, 0.0, line * up * ( GetLineHeight() + m_linespace ), m_alignment, true, pworld );

        a2dVectorPath* canpath = new a2dVectorPath( segments );
        canpath->SetStroke( this->GetStroke() );
        canpath->SetFill( this->GetFill() );
        canpath->SetLayer( m_layer );
        if ( m_font.GetType() == a2dFONT_STROKED )
        {
            canpath->SetContourWidth( m_font.GetStrokeWidth() );
            canpath->SetPathType( a2dPATH_END_ROUND );
        }
        canpathlist->push_back( canpath );

        lineend++;
        line++;
    }

    return canpathlist;
}

int a2dText::GetAsArray( wxArrayString& array )
{
    int lines = 0;
    if ( GetMultiLine() )
    {
        wxString line;
        size_t i = 0;
        while ( i < m_text.Length() )
        {
            if ( m_text[i] == '\n' )
            {
                array.Add( line );
                line.Empty();
                lines++;
            }
            else
                line += m_text[i];
            i++;
        }
        if ( !line.IsEmpty() )
        {
            array.Add( line );
            lines++;
        }
    }
    else
    {
        array.Add( m_text );
        lines++;
    }
    return lines;
}

int a2dText::GetLines() const
{
    int lines = 1;
    if ( GetMultiLine() )
    {
        size_t i = 0;
        while ( i < m_text.Length() )
        {
            if ( m_text[i] == '\n' )
                lines++;
            i++;
        }
    }
    return lines;
}

wxString a2dText::GetLine( int n ) const
{
    wxString string;

    size_t i = 0;
    int newlines = 0;
    while ( i < m_text.Length() && newlines < n )
    {
        if ( m_text[i] == '\n' )
            newlines++;
        i++;
    }
    if ( i < m_text.Length() )
    {
        while ( i < m_text.Length() && m_text[i] != '\n' )
        {
            string += m_text[i];
            i++;
        }
    }
    return string;
}

void a2dText::SetMultiLine( bool multiline )
{
    if ( multiline )
        m_textflags |= a2dCANOBJTEXT_MULTILINE;
    else
        m_textflags &= ~a2dCANOBJTEXT_MULTILINE;
    m_utbbox_changed = true;
}

void a2dText::SetReadOnly( bool readonly )
{
    if ( readonly )
        m_textflags |= a2dCANOBJTEXT_READONLY;
    else
        m_textflags &= ~a2dCANOBJTEXT_READONLY;
}

void a2dText::SetBackGround( bool background )
{
    if ( background )
        m_textflags |= a2dCANOBJTEXT_BACKGROUND;
    else
        m_textflags &= ~a2dCANOBJTEXT_BACKGROUND;
}

void a2dText::SetDrawFrame( bool frame )
{
    if ( frame )
        m_textflags |= a2dCANOBJTEXT_FRAME;
    else
        m_textflags &= ~a2dCANOBJTEXT_FRAME;
}

wxString a2dText::GetSelectedText() const
{
    return m_text.Mid( m_selection_start_pos, m_selection_end_pos - m_selection_start_pos );
}

void a2dText::DeleteSelectedText()
{
	a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );

    if ( m_selection_start_pos != m_selection_end_pos )
    {
        m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( m_text.Length() - m_selection_end_pos );
        m_utbbox_changed = true;
        m_caret = m_selection_end_pos = m_selection_start_pos;

		SetPending( true );
		m_root->GetCommandProcessor()->Submit( new a2dCommand_ChangeText( original,  m_text, m_caret, m_font, m_linespace, m_textflags ) );
    }
}

void a2dText::InsertTextAtCarretOrSelection( const wxString& text )
{
    if ( m_selection_end_pos - m_selection_start_pos > 0 )
    {
        size_t textlength = m_text.Length();
	    m_text = m_text.Left( m_selection_start_pos ) + text + m_text.Right( textlength - m_selection_end_pos );
	    m_utbbox_changed = true;
        m_caret = m_selection_start_pos;
	    m_selection_start_pos = m_selection_end_pos = m_caret;
        SetPending( true );
        if ( m_flags.m_editingCopy )
	    {
            a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );
	        m_root->GetCommandProcessor()->Submit( new a2dCommand_ChangeText( original,  m_text, m_caret, m_font, m_linespace, m_textflags ) );
	    }	
    }
    else
        InsertTextAtCarret( text );
}

void a2dText::InsertTextAtCarret( const wxString& text )
{
    size_t textlength = m_text.Length();
	m_text = m_text.Left( m_caret ) + text + m_text.Right( textlength - m_caret );
	m_utbbox_changed = true;
	m_caret += text.Length();
	m_selection_start_pos = m_selection_end_pos = m_caret;
    SetPending( true );
    if ( m_flags.m_editingCopy )
	{
        a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );
	    m_root->GetCommandProcessor()->Submit( new a2dCommand_ChangeText( original,  m_text, m_caret, m_font, m_linespace, m_textflags ) );
	}	
}

// Set caret to closest character in string
bool a2dText::SetCaret( a2dIterC& ic, double x, double y )
{
    a2dIterCU cu( ic, m_lworld );

    a2dAffineMatrix inverse;
    if ( GetEngineeringText() )
    {
        inverse = ic.GetTransform() * GetEngineeringTransform( ic );
        inverse.Invert();
    }
    else
        inverse = ic.GetInverseTransform();

    double xh, yh;
    inverse.TransformPoint( x, y, xh, yh );

    SetCaretUntransformed( xh, yh );
    return true;
}

void a2dText::SetCaretUntransformed( double xh, double yh )
{
    m_caret = 0;
    int up = GetNextLineDirection() ? 1 : -1;
    a2dBoundingBox linebbox;
    size_t linestart;
    size_t lineend = ( size_t ) 0;
    size_t textlength = m_text.Length();
    int line = 0;
    wxString text;
    while ( lineend <= textlength )
    {
        linestart = lineend;
        while ( lineend < textlength && m_text[lineend] != wxChar( '\n' ) )
            lineend++;
        text = m_text.Mid( linestart, lineend - linestart );
        linebbox = m_font.GetTextExtent( text, m_alignment );
        linebbox.Translate( 0.0, line * up * ( GetLineHeight() + m_linespace ) );
        if ( ( yh >= linebbox.GetMinY() - m_linespace / 2.0 )
                && ( yh < linebbox.GetMaxY() + m_linespace / 2.0 )
                || line == 0 && up == 1 && yh < linebbox.GetMaxY() + m_linespace / 2.0
                || line == 0 && up == -1 && yh >= linebbox.GetMinY() - m_linespace / 2.0 )
        {
            if ( !text.IsEmpty() )
            {
                double oldx, newx;
                double kerning = 0.0;
                oldx = newx = linebbox.GetMinX();

                for( size_t i = 0; i < text.Length(); i++ )
                {
                    newx += m_font.GetWidth( text[i] );
                    if ( i > 0 )
                        kerning = m_font.GetKerning( text[i - 1], text[i] );
                    newx += kerning / 2.0;

                    if ( ( oldx + newx ) / 2.0 > xh )
                    {
                        m_caret = linestart + i;
                        if ( m_caret < m_firsteditable )
                            m_caret = m_firsteditable;
                        return;
                    }

                    oldx = newx;
                    newx += kerning / 2.0;
                }
            }
            m_caret = lineend;
            break;
        }
        lineend++;
        line++;
    }
    if ( m_caret < m_firsteditable )
        m_caret = m_firsteditable;
}

void a2dText::SetNextLineDirection( bool up )
{
    if ( up )
        m_textflags |= a2dCANOBJTEXT_NEXTLINEUP;
    else
        m_textflags &= ~a2dCANOBJTEXT_NEXTLINEUP;
    m_utbbox_changed = true;
    SetPending( true );
}

void a2dText::SetEngineeringText( bool engineering )
{
    if ( engineering )
        m_textflags |= a2dCANOBJTEXT_ENGINEERING;
    else
        m_textflags &= ~a2dCANOBJTEXT_ENGINEERING;
    m_utbbox_changed = true;
    SetPending( true );
}

void a2dText::SetCaretShow( bool visible )
{
    if ( visible )
        m_textflags |= a2dCANOBJTEXT_CARETVISIBLE;
    else
        m_textflags &= ~a2dCANOBJTEXT_CARETVISIBLE;
    m_utbbox_changed = true;
    SetPending( true );
}

void a2dText::SetFont(  const a2dFont& font, double lineSpaceFactor )
{
    m_font = font;
    m_linespace = font.GetSize() * lineSpaceFactor;
    m_utbbox_changed = true;
    SetPending( true );
}

#if wxART2D_USE_CVGIO
void a2dText::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    m_utbbox_changed = true;
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_linespace = parser.RequireAttributeValueDouble( wxT( "linespace" ) );
        m_text = parser.RequireAttributeValue( wxT( "text" ) );
        m_text.Replace( wxT( "\\n" ), wxT( "\n" ) );
        m_textflags = parser.GetAttributeValueInt( wxT( "textflags" ),
                      a2dCANOBJTEXT_MULTILINE
                      | a2dCANOBJTEXT_CARETVISIBLE
                      | a2dCANOBJTEXT_NEXTLINEUP );
        m_fontinfo.ParseString( parser.RequireAttributeValue( wxT( "font" ) ) );
        m_font = a2dFont::CreateFont( m_fontinfo );
        if ( !m_font.Ok() )
		{
            m_font = *a2dDEFAULT_CANVASFONT;
            m_font.SetSize( m_fontinfo.GetSize() );
			m_wrongLoad = true;
		}
		if ( parser.GetScale() != 1 )
			m_font.SetSize( m_font.GetSize() * parser.GetScale() );

		m_alignment = AlignmentString2TextAlignment( parser.GetAttributeValue( wxT( "alignment" ), wxT( "wxMINX|wxMINY" ) ) );
        m_firsteditable = parser.GetAttributeValueInt( wxT( "firsteditable" ), 0 );
    }
    else
    {
    }
}

void a2dText::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    //editing text makes original invisible, do this to set it visible temporarely, in order to save it visible
    if ( m_flags.m_editing )
        m_flags.m_visible = true;

    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "linespace" ), m_linespace );
        out.WriteNewLine();

        wxString str = m_text;
        str.Replace( wxT( "\n" ), wxT( "\\n" ) );
        out.WriteAttributeEnt( wxT( "text" ), str );
        out.WriteNewLine();
        out.WriteAttribute( wxT( "textflags" ), m_textflags );
		if ( m_wrongLoad )
			out.WriteAttribute( wxT( "font" ), m_fontinfo.CreateString() );        
		else
            out.WriteAttribute( wxT( "font" ), m_font.CreateString() );
        out.WriteNewLine();
        out.WriteAttribute( wxT( "firsteditable" ), m_firsteditable );

		wxString alignstring = TextAlignment2String( m_alignment );
        out.WriteAttribute( wxT( "alignment" ), alignstring );
    }
    else
    {
    }

    if ( m_flags.m_editing )
        m_flags.m_visible = true;
}
#endif //wxART2D_USE_CVGIO

bool a2dText::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    double xmin = m_untransbbox.GetMinX();
    double ymin = m_untransbbox.GetMinY();
    double xmax = m_untransbbox.GetMaxX();
    double ymax = m_untransbbox.GetMaxY();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, pw + margin );

    if ( hitEvent.m_how.IsHit() )
        return hitEvent.m_how.IsHit();
    else
        return hitEvent.m_how.IsHit();
}


#define SIN100 0.9848077530
#define COS100 -0.1736481776

a2dAffineMatrix a2dText::GetEngineeringTransform( a2dIterC& ic )
{
    a2dAffineMatrix mod_affine = ic.GetTransform();
    a2dAffineMatrix eng_affine;
    // If text is mirrored, flip it, to be readible
    if ( eng_affine.GetDeterminant() < 0.0 )
    {
        eng_affine.Mirror( true, false );
        eng_affine.Translate( 0.0, m_untransbbox.GetMinY() + m_untransbbox.GetMaxY() );
        mod_affine *= eng_affine;
    }
    // If text is not readible from the bottom right, rotate it 180 degrees.
//    double angle = wxDegToRad( 100.0 );
//    if ( ic.GetDrawer2D()->GetYaxis() && (eng_affine( 0, 0 ) * sin( angle )  < eng_affine( 0, 1 ) * cos( angle ) )
//            || !ic.GetDrawer2D()->GetYaxis() && (eng_affine( 0, 0 ) * sin( angle )  < -eng_affine( 0, 1 ) * cos( angle ) ) )
    if ( ic.GetDrawer2D()->GetYaxis() && ( mod_affine( 0, 0 ) * SIN100  < mod_affine( 0, 1 ) * COS100 )
            || !ic.GetDrawer2D()->GetYaxis() && ( mod_affine( 0, 0 ) * SIN100  < -mod_affine( 0, 1 ) * COS100 ) )
    {
        a2dAffineMatrix rot;
        rot.Mirror( true, true );
        rot.Translate( m_untransbbox.GetMinX() + m_untransbbox.GetMaxX(), m_untransbbox.GetMinY() + m_untransbbox.GetMaxY() );
        eng_affine *= rot;
    }
    return eng_affine;
}

#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

void a2dText::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    ic.GetDrawer2D()->SetFont( m_font );

//#ifndef CaretOnTheFLy
    if ( m_flags.m_editingCopy )
        ic.GetDrawer2D()->SetNormalizedFont( m_flags.m_editingCopy );//GetCaretShow() );
//#endif
    // For debugging
    //ic.GetDrawer2D()->SetDrawerFill(*a2dWHITE_FILL);
    //ic.GetDrawer2D()->DrawRoundedRectangle(BBOX2XYWH(m_untransbbox), 0);

    if( GetDrawFrame() )
    {
        if ( !GetBackGround() )
        {
            a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL);
            ic.GetDrawer2D()->DrawRoundedRectangle( BBOX2XYWH( m_untransbbox ), 0 );
            ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
        }
        else
            ic.GetDrawer2D()->DrawRoundedRectangle( BBOX2XYWH( m_untransbbox ), 0 );
    }

    if ( GetEngineeringText() )
        ic.GetDrawer2D()->PushTransform( GetEngineeringTransform( ic ) );

    //invert style is not so good so a rectangle in that case
    if ( ic.GetDrawer2D()->GetDrawStyle() != a2dWIREFRAME_INVERT )
    {
        if ( !GetDrawFrame() && m_text.Length() == 0 )
            ic.GetDrawer2D()->DrawRoundedRectangle( BBOX2XYWH( m_untransbbox ), 0 );

        int up = GetNextLineDirection() ? 1 : -1;
        size_t linestart;
        size_t lineend = ( size_t ) 0;
        size_t textlength = m_text.Length();
        int line = 0;
        wxString linetext;
        while ( lineend < textlength )
        {
            linestart = lineend;
            while ( lineend < textlength && m_text[lineend] != wxChar( '\n' ) )
                lineend++;
            linetext = m_text.Mid( linestart, lineend - linestart );

            if ( ( m_selection_start_pos == m_selection_end_pos )
                    || ( m_selection_start_pos > lineend )
                    || ( m_selection_end_pos < linestart ) )
            {
                // No selection on this line, so draw normal textline
                ic.GetDrawer2D()->DrawText( linetext, 0.0, line * up * ( GetLineHeight() + m_linespace ), m_alignment, !GetDrawFrame() && GetBackGround() );
            }
            else if ( m_selection_start_pos < linestart
                      && m_selection_end_pos > lineend )
            {
                // Whole line selected, so draw inverted textline
                ic.GetDrawer2D()->OverRuleFixedStyle();
                a2dStroke currentstroke = ic.GetDrawer2D()->GetDrawerStroke();
                ic.GetDrawer2D()->SetDrawerFill( a2dFill( currentstroke.GetColour() ) );
                wxColour col = currentstroke.GetColour();
                if ( col.Red() + col.Green() + col.Blue() > 3 * 192 )
                    ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
                else
                    ic.GetDrawer2D()->SetDrawerStroke( *a2dWHITE_STROKE );
                ic.GetDrawer2D()->DrawText( linetext, 0.0, line * up * ( GetLineHeight() + m_linespace ), m_alignment, true );
                ic.GetDrawer2D()->ReStoreFixedStyle();
            }
            else
            {
                ic.GetDrawer2D()->OverRuleFixedStyle();
                // Draw some parts normal and some parts inverted
                a2dBoundingBox bbox = m_font.GetTextExtent( linetext, m_alignment );
                double x = bbox.GetMinX();
                double y = bbox.GetMaxY();
                double w;
                //we use the boundingbox of the whole text to draw correctly part of un selcted and selected text lines.
                //Else center aligned text would become really hard to to draw in parts.
                //With the temporay alignent set like next, it becomes simpler.
                int alignselect = wxMINX | wxMAXY;

                wxString left, sel, right;
                if ( m_selection_start_pos > linestart )
                {
                    // Draw unselected part left of selection.
                    left = linetext.Left( m_selection_start_pos - linestart );
                    ic.GetDrawer2D()->DrawText( left, x, y + line * up * ( GetLineHeight() + m_linespace ), alignselect, !GetDrawFrame() && GetBackGround() );
                    m_font.GetTextExtent( left, wxMINX | wxMAXY, &w );
                    x += w;
                }
                // Draw selected part.
                if ( m_selection_start_pos >= linestart )
                    sel = linetext.Mid( m_selection_start_pos - linestart, m_selection_end_pos - m_selection_start_pos );
                else
                    sel = linetext.Mid( 0, m_selection_end_pos - linestart );
                ic.GetDrawer2D()->SetDrawStyle( a2dFILLED );
                a2dStroke currentstroke = ic.GetDrawer2D()->GetDrawerStroke();
                ic.GetDrawer2D()->SetDrawerFill( a2dFill( currentstroke.GetColour() ) );
                wxColour col = currentstroke.GetColour();
                if ( col.Red() + col.Green() + col.Blue() > 3 * 192 )
                    ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
                else
                    ic.GetDrawer2D()->SetDrawerStroke( *a2dWHITE_STROKE );
                if ( !left.IsEmpty() && ! sel.IsEmpty() )
                    x += m_font.GetKerning( left.Last(), sel[( size_t ) 0] );
                ic.GetDrawer2D()->DrawText( sel, x, y + line * up * ( GetLineHeight() + m_linespace ), alignselect, true );
                m_font.GetTextExtent( sel, wxMINX | wxMAXY, &w );
                x += w;
                ic.GetDrawer2D()->ReStoreFixedStyle();
                if ( m_selection_end_pos < lineend )
                {
                    // Draw unselected part right of selection.
                    right = linetext.Right( lineend - m_selection_end_pos );
                    if ( !sel.IsEmpty() && !right.IsEmpty() )
                        x += m_font.GetKerning( sel.Last(), right[( size_t ) 0] );
                    ic.GetDrawer2D()->DrawText( right, x, y + line * up * ( GetLineHeight() + m_linespace ), alignselect, !GetDrawFrame() && GetBackGround() );
                }
            }
            lineend++;
            line++;
        }
    }
    else
    {
        a2dSmrtPtr<a2dRect> tmp = new a2dRect( BBOX2XYWH( m_untransbbox ) );
        tmp->SetRoot( m_root, false );
        tmp->Update( updatemask_force );
        tmp->SetFill( *a2dTRANSPARENT_FILL );
        tmp->SetStroke( ic.GetDrawer2D()->GetDrawerStroke() );
        tmp->Render( ic, _ON );
    }

#ifdef CaretOnTheFLy
    // here font is correctly recalculated for current zoom, so after drawing it.
    // It can now be used to calculate caret in Un-normalized text size.
    if ( m_flags.m_editingCopy )
    {
        if( GetCaretShow() )
        {
            if ( m_text.Length() == 0 )
            {
                m_XcaretinLine = m_untransbbox.GetMinX();
                m_YcaretinLine = m_untransbbox.GetMinY();
            }
            else
            {
                double wcaret = 0.0;
                int up = GetNextLineDirection() ? 1 : -1;
                size_t linestart;
                size_t lineend = ( size_t ) 0;
                size_t textlength = m_text.Length();
                int line = 0;
                wxString text;
                while ( lineend <= textlength )
                {
                    linestart = lineend;
                    while ( lineend < textlength && m_text[lineend] != wxChar( '\n' ) )
                        lineend++;

                    if ( lineend >= m_caret && linestart <= m_caret )
                    {
                        a2dBoundingBox linebbox;
                        text = m_text.Mid( linestart, lineend - linestart );
                        linebbox = m_font.GetTextExtent( text, m_alignment );
                        linebbox.Translate( 0.0, line * up * ( GetLineHeight() + m_linespace ) );

                        // Get width of text left of cursor
                        m_font.GetTextExtent( text.Left( m_caret - linestart ), m_alignment, &wcaret );
                        // If previous character exists ...
                        if ( m_caret - linestart > 1 && ( size_t ) ( m_caret - linestart ) <= text.Length() )
                            // ... Put caret halfway the kerning value
                            wcaret += m_font.GetKerning( text[( size_t ) ( m_caret - linestart - 2 )],
                                                         text[( size_t ) ( m_caret - linestart - 1 )] ) / 2;
                        m_XcaretinLine = linebbox.GetMinX() + wcaret;
                        m_YcaretinLine = linebbox.GetMinY();

                        //only this line where the caret is
                        break;
                    }
                    lineend++;
                    line++;
                }
            }
        }
        else
        {
            if ( m_text.Length() == 0 )
            {
                m_XcaretinLine = m_bbox.GetMinX();
                m_YcaretinLine = m_bbox.GetMinY();
            }

        }
        if ( m_flags.m_editingCopy )
        {
            m_pixelExtend = DRAGRECTWIDTH / 2;
        }
    }
#endif //CaretOnTheFLy

    //now do the carret drawing  (this is faster than combining with the text drawing itself.)
    //i think because of the stroke change happening only once like this.
    if ( ic.GetDrawer2D()->GetDrawStyle() != a2dWIREFRAME_INVERT
            && GetCaretShow() && ( m_caret != -1 )
            && ( m_selection_start_pos == m_selection_end_pos ) )
    {
        ic.GetDrawer2D()->OverRuleFixedStyle();

        a2dStroke tmp = a2dStroke( wxColour( 0, 0, 0 ), 2 );
        ic.GetDrawer2D()->SetDrawerStroke( tmp );
        ic.GetDrawer2D()->DrawLine( m_XcaretinLine, m_YcaretinLine,
                                    m_XcaretinLine, m_YcaretinLine + GetLineHeight() );
        ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );

        //wxLogDebug( "pending %d caret %d (a2dText*)0x%p", m_flags.m_pending, m_caret, this );
        //wxLogDebug(wxT("xc=%12.6lf, yc=%12.6lf"),m_XcaretinLine,m_YcaretinLine);

        ic.GetDrawer2D()->ReStoreFixedStyle();
    }

    ic.GetDrawer2D()->SetFont( *a2dNullFONT );

    if ( m_flags.m_editingCopy )
    {
        ic.GetDrawer2D()->OverRuleFixedStyle();
        a2dSmrtPtr<a2dRect> around = new a2dRect( BBOX2XYWH( m_untransbbox ) );
        //a2dStroke strokearound = a2dStroke( GetHabitat()->GetArtProvider().GetImage( a2dBitmap_EditRectangleFill )
        //    , wxColour( 0, 0, 0 ), wxColour( 255, 0, 255 ), a2dSTROKE_STIPPLE );
        a2dStroke strokearound = a2dStroke(  wxColour( 192, 192, 192 ) , DRAGRECTWIDTH, a2dSTROKE_CROSSDIAG_HATCH  );
        strokearound.SetWidth( DRAGRECTWIDTH );
        strokearound.SetPixelStroke( true );
        around->SetStroke( strokearound );

        //a2dFill fillaround = a2dFill( GetHabitat()->GetArtProvider().GetImage( a2dBitmap_EditRectangleFill )
        //    , wxColour( 0, 0, 0 ), wxColour( 255, 0, 255 ), a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
        //around->SetFill( fillaround );
        around->SetFill( *a2dTRANSPARENT_FILL );
        around->SetRoot( m_root, false );
        around->Update( updatemask_force );
        around->Render( ic, _ON );
        ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
        ic.GetDrawer2D()->ReStoreFixedStyle();
    }

    // Restore transform;
    if ( GetEngineeringText() )
        ic.GetDrawer2D()->PopTransform();

//#ifndef CaretOnTheFLy
    if ( m_flags.m_editingCopy )
        ic.GetDrawer2D()->SetNormalizedFont( false );
//#endif
}

bool a2dText::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();

        if( GetCaretShow() )
        {
            if ( m_text.Length() == 0 )
            {
                m_XcaretinLine = m_untransbbox.GetMinX();
                m_YcaretinLine = m_untransbbox.GetMinY();
            }
            else
            {
                double wcaret = 0.0;
                int up = GetNextLineDirection() ? 1 : -1;
                size_t linestart;
                size_t lineend = ( size_t ) 0;
                size_t textlength = m_text.Length();
                int line = 0;
                wxString text;
                while ( lineend <= textlength )
                {
                    linestart = lineend;
                    while ( lineend < textlength && m_text[lineend] != wxChar( '\n' ) )
                        lineend++;

                    if ( lineend >= m_caret && linestart <= m_caret )
                    {
                        a2dBoundingBox linebbox;
                        text = m_text.Mid( linestart, lineend - linestart );
                        linebbox = m_font.GetTextExtent( text, m_alignment );
                        linebbox.Translate( 0.0, line * up * ( GetLineHeight() + m_linespace ) );

                        // Get width of text left of cursor
                        m_font.GetTextExtent( text.Left( m_caret - linestart ), m_alignment, &wcaret );
                        // If previous character exists ...
                        if ( m_caret - linestart > 1 && ( size_t ) ( m_caret - linestart ) <= text.Length() )
                            // ... Put caret halfway the kerning value
                            wcaret += m_font.GetKerning( text[( size_t ) ( m_caret - linestart - 2 )],
                                                         text[( size_t ) ( m_caret - linestart - 1 )] ) / 2;
                        m_XcaretinLine = linebbox.GetMinX() + wcaret;
                        m_YcaretinLine = linebbox.GetMinY();

                        //only this line where the caret is
                        break;
                    }
                    lineend++;
                    line++;
                }
            }
        }
        else
        {
            if ( m_text.Length() == 0 )
            {
                m_XcaretinLine = m_bbox.GetMinX();
                m_YcaretinLine = m_bbox.GetMinY();
            }

        }
        if ( m_flags.m_editingCopy )
        {
            m_pixelExtend = DRAGRECTWIDTH / 2;
        }
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

a2dBoundingBox a2dText::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    if( !m_utbbox_changed && GetCaretShow() )
    {
        return m_untransbbox;
    }

    a2dBoundingBox bbox;

    // When no text, use a bounding box with the size of an '|'
    if ( m_text.Length() == 0 )
    {
        bbox = m_font.GetTextExtent( wxT( "|" ), m_alignment );
    }
    else
    {
        double wcaret = 0.0;
        int up = GetNextLineDirection() ? 1 : -1;
        a2dBoundingBox linebbox;
        size_t linestart;
        size_t lineend = ( size_t ) 0;
        size_t textlength = m_text.Length();
        int line = 0;
        wxString text;
        while ( lineend <= textlength )
        {
            linestart = lineend;
            while ( lineend < textlength && m_text[lineend] != wxChar( '\n' ) )
                lineend++;
            text = m_text.Mid( linestart, lineend - linestart );

            linebbox = m_font.GetTextExtent( text, m_alignment );
            linebbox.Translate( 0.0, line * up * ( GetLineHeight() + m_linespace ) );
            bbox.Expand( linebbox );
            lineend++;
            line++;
        }
    }
    m_untransbbox = bbox;
    m_utbbox_changed = false;
    return bbox;
}

bool a2dText::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        SetCaretShow( true );
        m_caret = m_selection_start_pos = m_selection_end_pos = 0;
        a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );
        //save the text as is now, undo of the whole string is easy
        PROPID_PreEditText->SetPropertyToObject( this, m_text );
        if ( editstyle & wxEDITSTYLE_COPY )
            original->SetVisible( false );
        if ( editmode == 1 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //calculate bbox's else mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //stil set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            if ( m_font.GetType() != a2dFONT_WXDC )
            {
                // since this is not a dc-font, allow stretching etc.
                PROPID_IncludeChildren->SetPropertyToObject( this, false );
                PROPID_Allowrotation->SetPropertyToObject( this, true );
                PROPID_Allowskew->SetPropertyToObject( this, true );

                return a2dCanvasObject::DoStartEdit( editmode, editstyle );
            }
            else
            {
                m_flags.m_visiblechilds = true;
                m_flags.m_childrenOnSameLayer = true;

                //add properties and handles
                a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );
                if ( untrans.GetWidth() == 0 || untrans.GetWidth() == 0 )
                    untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN | a2dCANOBJ_BBOX_EDIT );

                double x, y, w, h;
                x = untrans.GetMinX();
                y = untrans.GetMinY();
                w = untrans.GetWidth();
                h = untrans.GetHeight();

                int up = GetNextLineDirection() ? 1 : -1;
                a2dHandle* handle = NULL;
                Append( handle = new a2dHandle( this, 0, up * GetTextHeight(), wxT( "__text_fontsize__" ) ) );
                handle->SetLayer( m_layer );

                // show rotation
                a2dSLine* rotline;
                if ( m_alignment & wxMAXX )
                    rotline = new a2dSLine( 0, 0, -w / 4, 0 );
                else
                    rotline = new a2dSLine( 0, 0, w / 4, 0 );
                rotline->SetStroke( *wxRED, 0 );
                rotline->SetPreRenderAsChild( false );
                rotline->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                rotline->SetHitFlags( a2dCANOBJ_EVENT_NON );
                rotline->SetLayer( m_layer );
                Append( rotline );

                if ( m_alignment & wxMAXX )
                    Append( handle = new a2dHandle( this, -w / 4, 0, wxT( "__text_rotate__" ) ) );
                else
                    Append( handle = new a2dHandle( this, w / 4, 0, wxT( "__text_rotate__" ) ) );
                handle->SetLayer( m_layer );

                m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
                m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );

                //calculate bbox's else mouse events may take place when first idle event is not yet
                //processed to do this calculation.
                Update( updatemask_force );
                //still set it pending to do the redraw ( in place )
                SetPending( true );
                return true;
            }
        }
    }
    return false;
}

void a2dText::DoEndEdit()
{
    m_caret = -1;
    m_selection_start_pos = m_selection_end_pos = 0;
    a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );

    original->m_caret = 0;
    original->SetCaretShow( false );
    original->SetVisible( true );
    //original->m_text = PROPID_PreEditText->GetPropertyValue( this );
    //m_root->GetCommandProcessor()->Submit( new a2dCommand_ChangeText( original,  m_text, m_caret, m_font, m_linespace, m_textflags ) );
}

void a2dText::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );

        bool extend_selection;
        bool extend_start;
        bool changed = true;
        size_t initial_pos = m_caret;

        extend_selection = event.ShiftDown();
        extend_start = FALSE;

        if ( extend_selection )
        {
            if ( m_selection_start_pos == m_selection_end_pos )
            {
                m_selection_start_pos = m_caret;
                m_selection_end_pos = m_caret;
            }
            extend_start = ( m_caret == m_selection_start_pos );
        }

        size_t textlength = m_text.Length();
        switch ( event.GetKeyCode() )
        {
            case WXK_HOME:
                if( event.ControlDown() )
                    // move to start of text
                    m_caret = m_firsteditable;
                else
                {
                    // move to start of line
                    while ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] != wxChar( '\n' ) )
                        m_caret--;
                }
                break;
            case WXK_END:
                if( event.ControlDown() )
                    // move to end of text
                    m_caret = textlength;
                else
                    // move to end of line
                    while ( m_caret < textlength && m_text[ m_caret] != wxChar( '\n' ) )
                        m_caret++;
                break;
                //        case GDK_KP_Page_Up:
                //  case GDK_Page_Up:   move_cursor_page_ver (text, -1); break;
                //        case GDK_KP_Page_Down:
                //  case GDK_Page_Down: move_cursor_page_ver (text, +1); break;
                /* CUA has Ctrl-Up/Ctrl-Down as paragraph up down */
            case WXK_UP:
                // move up one line
            {
                double up = GetNextLineDirection() ? 1.0 : -1.0;
                SetCaretUntransformed( m_XcaretinLine, m_YcaretinLine + GetLineHeight() / 2 - up * ( GetLineHeight() + m_linespace ) );
            }
            break;
            case WXK_DOWN:
                // move down one line
            {
                double up = GetNextLineDirection() ? 1.0 : -1.0;
                SetCaretUntransformed( m_XcaretinLine, m_YcaretinLine + GetLineHeight() / 2 + up * ( GetLineHeight() + m_linespace ) );
            }
            break;
            case WXK_LEFT:
                if( event.ControlDown() )
                {
                    // move cursor to start of (previous) word
                    while ( m_caret > m_firsteditable && ( m_text[ m_caret - 1] == wxChar( '\n' )
                                                           || m_text[ m_caret - 1] == wxChar( ' ' ) ) )
                        m_caret--;
                    while ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] != wxChar( '\n' )
                            && m_text[ m_caret - 1 ] != wxChar( ' ' ) )
                        m_caret--;
                }
                else
                {
                    // move cursor left
                    if ( m_caret > m_firsteditable )
                        m_caret--;
                }
                break;
            case WXK_RIGHT:
                if( event.ControlDown() )
                {
                    // move cursor to start of next word
                    while ( m_caret < textlength && m_text[m_caret] != wxChar( '\n' )
                            && m_text[m_caret] != wxChar( ' ' ) )
                        m_caret++;
                    while ( m_caret < textlength && ( m_text[m_caret] == wxChar( '\n' ) )
                            || ( m_text[m_caret] == wxChar( ' ' ) ) )
                        m_caret++;
                }
                else
                    // move cursor right
                    if ( m_caret < textlength )
                        m_caret++;
                break;
            case WXK_BACK:
                if ( m_selection_start_pos != m_selection_end_pos )
                {
                    // delete selection
                    extend_selection = FALSE;
                    m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                    m_utbbox_changed = true;
                    m_caret = m_selection_end_pos = m_selection_start_pos;
                }
                else if( event.ControlDown() )
                {
                    // delete to start of (previous) word
                    size_t curpos = m_caret;
                    while ( m_caret > m_firsteditable && ( m_text[m_caret - 1] == wxChar( '\n' )
                                                           || m_text[m_caret - 1] == wxChar( ' ' ) ) )
                        m_caret--;
                    while ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] != wxChar( '\n' )
                            && m_text[ m_caret - 1 ] != wxChar( ' ' ) )
                        m_caret--;
                    m_text = m_text.Left( m_caret ) + m_text.Right( textlength - curpos );
                    m_utbbox_changed = true;
                }
                else if ( m_caret > m_firsteditable )
                {
                    // delete previous character
                    m_text = m_text.Left( m_caret - 1 ) + m_text.Right( textlength - m_caret );
                    m_utbbox_changed = true;
                    m_caret--;
                }
                break;
            case WXK_CLEAR:
            {
                // delete line
                while ( m_caret < textlength && ( m_text[m_caret] != wxChar( '\n' ) ) )
                    m_caret++;
                size_t endline = m_caret;
                if ( m_caret < textlength && ( m_text[m_caret] == wxChar( '\n' ) ) )
                    endline++;
                while ( m_caret > 0 && m_text[m_caret - 1 ] != wxChar( '\n' ) )
                    m_caret--;
                m_text = m_text.Left( m_caret ) + m_text.Right( textlength - endline );
                m_utbbox_changed = true;
            }
            case WXK_INSERT:
                if( event.ShiftDown() )
                {
                    // insert text from clipboard
                    extend_selection = false;
                    changed = false;
                    if ( wxTheClipboard->Open() )
                    {
                        if ( wxTheClipboard->IsSupported( wxDF_TEXT ) )
                        {
                            wxTextDataObject data;
                            wxTheClipboard->GetData( data );
                            m_text = m_text.Left( m_caret ) + data.GetText() + m_text.Right( textlength - m_caret );
                            m_utbbox_changed = true;
                            m_caret += data.GetText().Length();
                            changed = true;
                        }
                        wxTheClipboard->Close();
                    }
                }
                else if( event.ControlDown() )
                {
                    // copy selection to clipboard
                    if ( m_selection_start_pos != m_selection_end_pos )
                    {
                        // Write some text to the clipboard
                        if ( wxTheClipboard->Open() )
                        {
                            wxTheClipboard->SetData( new wxTextDataObject( m_text.Mid( m_selection_start_pos, m_selection_end_pos - m_selection_start_pos ) ) );
                            wxTheClipboard->Close();
                        }
                    }
                    changed = false;
                }
                else
                {
                    changed = false;
                    /* gtk_toggle_insert(text) -- IMPLEMENT */
                }
                break;
            case WXK_DELETE:
                if ( m_selection_start_pos != m_selection_end_pos )
                {
                    extend_selection = FALSE;
                    if( event.ShiftDown() )
                    {
                        // copy selection to clipboard
                        if ( wxTheClipboard->Open() )
                        {
                            wxTheClipboard->SetData( new wxTextDataObject( m_text.Mid( m_selection_start_pos, m_selection_end_pos - m_selection_start_pos ) ) );
                            wxTheClipboard->Close();
                        }
                    }
                    // delete selection
                    m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                    m_utbbox_changed = true;
                    m_caret = m_selection_end_pos = m_selection_start_pos;
                }
                else if( event.ControlDown() )
                {
                    // delete to start of next word
                    size_t curpos = m_caret;
                    while ( m_caret < textlength && m_text[m_caret] != wxChar( '\n' )
                            && m_text[m_caret] != wxChar( ' ' ) )
                        m_caret++;
                    while ( m_caret < textlength && m_text[m_caret] != wxChar( '\n' )
                            && m_text[m_caret] == wxChar( ' ' ) )
                        m_caret++;
                    m_text = m_text.Left( curpos ) + m_text.Right( textlength - m_caret );
                    m_utbbox_changed = true;
                    m_caret = curpos;
                }
                else if ( m_caret < textlength )
                {
                    // delete next character
                    m_text = m_text.Left( m_caret ) + m_text.Right( textlength - ( m_caret + 1 ) );
                    m_utbbox_changed = true;
                }
                break;
            case WXK_TAB:
                //special meaning in wxArt2D: change edit mode
            {
                changed = false;
                wxUint16* editmode = PROPID_Editmode->GetPropertyValuePtr( this );
                if ( editmode )
                {
                    ( *editmode )++;
                    if ( ( *editmode ) > 1 ) ( *editmode ) = 0;
                    ReStartEdit( ( *editmode ) );
                }
            }
            break;
            case WXK_RETURN:
                // start a new line
                if ( GetMultiLine() )
                {
                    m_text = m_text.Left( m_caret ) + wxT( "\n" )  + m_text.Right( textlength - m_caret );
                    m_utbbox_changed = true;
                    m_caret++;
                }
                else
                {
                    changed = false;
                    wxBell();
                }
                break;
            case WXK_ESCAPE:
                changed = false;
                break;
            default:
                if( event.ControlDown() )
                {
                    switch( event.GetKeyCode() )
                    {
                        case 'A' - '@':
                            // move to start of line
                            while ( m_caret > m_firsteditable && m_text[m_caret - 1 ] != wxChar( '\n' ) )
                                m_caret--;
                            break;
                        case 'B' - '@':
                            // move cursor left
                            if ( m_caret > m_firsteditable )
                                m_caret--;
                            break;
                        case 'C' - '@':
                            // copy selection to clipboard
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                // Write some text to the clipboard
                                if ( wxTheClipboard->Open() )
                                {
                                    wxTheClipboard->SetData( new wxTextDataObject( m_text.Mid( m_selection_start_pos, m_selection_end_pos - m_selection_start_pos ) ) );
                                    wxTheClipboard->Close();
                                }
                            }
                            changed = false;
                            break;
                        case 'D' - '@':
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                // delete selection
                                extend_selection = FALSE;
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else if ( m_caret < textlength )
                            {
                                // delete next character
                                m_text = m_text.Left( m_caret ) + m_text.Right( textlength - ( m_caret + 1 ) );
                                m_utbbox_changed = true;
                            }
                            break;
                        case 'E' - '@':
                            // move to end of line
                            while ( m_caret < textlength && m_text[m_caret] != wxChar( '\n' ) )
                                m_caret++;
                            break;
                        case 'F' - '@':
                            // move cursor right
                            if ( m_caret < textlength )
                                m_caret++;
                            break;
                        case 'H' - '@':
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                // delete selection
                                extend_selection = FALSE;
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else if ( m_caret > m_firsteditable )
                            {
                                // delete previous character
                                m_text = m_text.Left( m_caret - 1 ) + m_text.Right( textlength - m_caret );
                                m_utbbox_changed = true;
                                m_caret--;
                            }
                            break;
                        case 'K' - '@':
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                // delete selection
                                extend_selection = FALSE;
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else
                            {
                                // delete to end of line
                                size_t curpos = m_caret;
                                while ( m_caret < textlength && ( m_text[m_caret] != wxChar( '\n' ) ) )
                                    m_caret++;
                                m_text = m_text.Left( curpos ) + m_text.Right( textlength - m_caret );
                                m_utbbox_changed = true;
                                m_caret = curpos;
                            }
                            break;
                        case 'N' - '@':
                            // move to start of next line
                            while ( m_caret < textlength && ( m_text[m_caret] != wxChar( '\n' ) ) )
                                m_caret++;
                            if ( m_caret < textlength && m_text[m_caret] == wxChar( '\n' ) )
                                m_caret++;
                            break;
                        case 'P' - '@':
                            // move to start of previous line
                            while ( m_caret > m_firsteditable && m_text[m_caret - 1 ] != wxChar( '\n' ) )
                                m_caret--;
                            if ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] == wxChar( '\n' ) )
                                m_caret--;
                            while ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] != wxChar( '\n' ) )
                                m_caret--;
                            break;
                        case 'U' - '@':
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                // delete selection
                                extend_selection = FALSE;
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else
                            {
                                // delete line
                                while ( m_caret < textlength && ( m_text[m_caret] != wxChar( '\n' ) ) )
                                    m_caret++;
                                size_t endline = m_caret;
                                if ( m_caret < textlength && ( m_text[m_caret] == wxChar( '\n' ) ) )
                                    endline++;
                                while ( m_caret > 0 && m_text[ m_caret - 1 ] != wxChar( '\n' ) )
                                    m_caret--;
                                m_text = m_text.Left( m_caret ) + m_text.Right( textlength - endline );
                                m_utbbox_changed = true;
                            }
                            break;
                        case 'V' - '@':
                            // insert text from clipboard
                            extend_selection = false;
                            changed = false;
                            if ( wxTheClipboard->Open() )
                            {
                                if ( wxTheClipboard->IsSupported( wxDF_TEXT ) )
                                {
                                    wxTextDataObject data;
                                    wxTheClipboard->GetData( data );
                                    m_text = m_text.Left( m_caret ) + data.GetText() + m_text.Right( textlength - m_caret );
                                    m_utbbox_changed = true;
                                    m_caret += data.GetText().Length();
                                    changed = true;
                                }
                                wxTheClipboard->Close();
                            }
                            break;
                        case 'W' - '@':
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                // delete selection
                                extend_selection = FALSE;
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else
                            {
                                // delete to start of (previous) word
                                size_t curpos = m_caret;
                                while ( m_caret > m_firsteditable && ( m_text[m_caret - 1] == wxChar( '\n' )
                                                                       || m_text[m_caret - 1] == wxChar( ' ' ) ) )
                                    m_caret--;
                                while ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] != wxChar( '\n' )
                                        && m_text[ m_caret - 1 ] != wxChar( ' ' ) )
                                    m_caret--;
                                m_text = m_text.Left( m_caret ) + m_text.Right( textlength - curpos );
                                m_utbbox_changed = true;
                            }
                            break;
                        case 'X' - '@':
                            // cut selection to clipboard
                            extend_selection = FALSE;
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                if ( wxTheClipboard->Open() )
                                {
                                    wxTheClipboard->SetData( new wxTextDataObject( m_text.Mid( m_selection_start_pos, m_selection_end_pos - m_selection_start_pos ) ) );
                                    wxTheClipboard->Close();
                                }
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else
                                changed = false;
                            break;
                        case WXK_NUMPAD1:
                            SetAlignment( wxMINX | wxMINY );
                            break;
                        case WXK_NUMPAD2:
                            SetAlignment( wxMINY );
                            break;
                        case WXK_NUMPAD3:
                            SetAlignment( wxMAXX | wxMINY );
                            break;
                        case WXK_NUMPAD4:
                            SetAlignment( wxMINX );
                            break;
                        case WXK_NUMPAD5:
                            SetAlignment( 0 );
                            break;
                        case WXK_NUMPAD6:
                            SetAlignment( wxMAXX );
                            break;
                        case WXK_NUMPAD7:
                            SetAlignment( wxMINX | wxMAXY );
                            break;
                        case WXK_NUMPAD8:
                            SetAlignment( wxMAXY );
                            break;
                        case WXK_NUMPAD9:
                            SetAlignment( wxMAXX | wxMAXY );
                            break;
                        case WXK_NUMPAD0:
                            SetAlignment( wxBASELINE );
                            break;
                        default:
                            changed = false;
                    }
                    break;
                }
                if( event.AltDown() )
                {
                    switch( event.GetKeyCode() )
                    {
                        case 'b': case 'B':
                            // move cursor to start of (previous) word
                            while ( m_caret > m_firsteditable && ( m_text[m_caret - 1] == wxChar( '\n' )
                                                                   || m_text[m_caret - 1] == wxChar( ' ' ) ) )
                                m_caret--;
                            while ( m_caret > m_firsteditable && m_text[ m_caret - 1 ] != wxChar( '\n' )
                                    && m_text[ m_caret - 1 ] != wxChar( ' ' ) )
                                m_caret--;
                            break;
                        case 'd': case 'D':
                            if ( m_selection_start_pos != m_selection_end_pos )
                            {
                                extend_selection = FALSE;
                                if( event.ShiftDown() )
                                {
                                    // copy selection to clipboard
                                    if ( wxTheClipboard->Open() )
                                    {
                                        wxTheClipboard->SetData( new wxTextDataObject( m_text.Mid( m_selection_start_pos, m_selection_end_pos - m_selection_start_pos ) ) );
                                        wxTheClipboard->Close();
                                    }
                                }
                                // delete selection
                                m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                                m_utbbox_changed = true;
                                m_caret = m_selection_end_pos = m_selection_start_pos;
                            }
                            else
                            {
                                // delete to start of next word
                                size_t curpos = m_caret;
                                while ( m_caret < textlength && m_text[m_caret] != wxChar( '\n' )
                                        && m_text[m_caret] != wxChar( ' ' ) )
                                    m_caret++;
                                while ( m_caret < textlength && ( m_text[m_caret] == wxChar( '\n' ) )
                                        || ( m_text[m_caret] == wxChar( ' ' ) ) )
                                    m_caret++;
                                m_text = m_text.Left( curpos ) + m_text.Right( textlength - m_caret );
                                m_utbbox_changed = true;
                                m_caret = curpos;
                            }
                            break;
                        case 'f': case 'F':
                            // move cursor to start of next word
                            while ( m_caret < textlength && m_text[m_caret] != wxChar( '\n' )
                                    && m_text[m_caret] != wxChar( ' ' ) )
                                m_caret++;
                            while ( m_caret < textlength && ( m_text[m_caret] == wxChar( '\n' ) )
                                    || ( m_text[m_caret] == wxChar( ' ' ) ) )
                                m_caret++;
                            break;
                        default:
                            changed = false;
                    }
                    break;
                }
                else
                {
                    extend_selection = false;
                    if ( m_selection_start_pos != m_selection_end_pos )
                    {
                        m_text = m_text.Left( m_selection_start_pos ) + m_text.Right( textlength - m_selection_end_pos );
                        m_utbbox_changed = true;
                        m_caret = m_selection_end_pos = m_selection_start_pos;
			            textlength = m_text.Length();
                    }
                    wxChar h = event.GetUnicodeKey();
                    m_text = m_text.Left( m_caret ) + h  + m_text.Right( textlength - m_caret );
                    m_utbbox_changed = true;
                    m_caret++;
                }
        } // switch ( event.GetKeyCode() );

        if ( changed && ( m_caret != initial_pos ) )
        {
            if ( extend_selection )
            {
                if ( m_selection_start_pos == initial_pos )
                {
                    m_selection_start_pos = m_selection_end_pos;
                    m_selection_end_pos = initial_pos;
                }
                if ( m_selection_start_pos > m_caret )
                {
                    m_selection_end_pos = m_selection_start_pos;
                    m_selection_start_pos = m_caret;
                }
                else
                    m_selection_end_pos = m_caret;
            }
            else
                m_selection_start_pos = m_selection_end_pos = m_caret;
        }
        if ( changed )
        {
            SetPending( true );
		    m_root->GetCommandProcessor()->Submit( new a2dCommand_ChangeText( original,  m_text, m_caret, m_font, m_linespace, m_textflags ) );
            //original->SetText( m_text );
            //original->SetAlignment( m_alignment );
        }
    }
//    else
//        event.Skip();
}

void a2dText::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    static bool busydrag = false;

    wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );
    PROPID_SkipBase->SetPropertyToObject( this, false );
    if ( editmode == 1 )
    {
        if ( m_flags.m_editingCopy )
        {
            a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );
            wxMouseEvent mouseevent = event.GetMouseEvent();

            //to world group coordinates to do hit test in world group coordinates
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();

            if ( mouseevent.LeftDown() )
            {
                a2dAffineMatrix matrix = ic->GetUserToDeviceTransform();
                matrix.Invert();
                double dragwidthworld = matrix.TransformDistance( DRAGRECTWIDTH );
                if ( GetBbox().PointOnBox(  xw, yw, dragwidthworld ) )
                {
                    busydrag = true;
                    event.Skip();
                }
                else if ( !mouseevent.ControlDown() )
                {
                    if ( GetBbox().PointInBox(  xw, yw, dragwidthworld ) )
                    {
                        SetCaret( *ic, xw, yw );
                        m_selection_start_pos = m_selection_end_pos = m_caret;
					    //just to remember caret change, also at start
				        m_root->GetCommandProcessor()->Submit( 
						    new a2dCommand_ChangeText( original,  m_text, m_caret, m_font, m_linespace, m_textflags ) );

                        SetPending( true );
                    }
                    else
                        EndEdit();

                    return;
                }
            }
            else if ( event.GetMouseEvent().Moving() && !busydrag )
            {
                a2dAffineMatrix matrix = ic->GetUserToDeviceTransform();
                matrix.Invert();
                double dragwidthworld = matrix.TransformDistance( DRAGRECTWIDTH );
                if ( GetBbox().PointOnBox(  xw, yw, dragwidthworld ) )
                {
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Move ) );
                }
                else if ( GetBbox().PointInBox(  xw, yw, dragwidthworld ) )
                {
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
                }
                else
                {
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Edit ) );
                }
                PROPID_SkipBase->SetPropertyToObject( this, true );
                event.Skip();
            }
            else if ( mouseevent.Dragging() )
            {
                if ( busydrag )
                    event.Skip();
                else if ( mouseevent.LeftIsDown() && !mouseevent.ControlDown() )
                {
                    a2dCanvasObject* hitobject;
                    a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
                    hitobject = IsHitWorld( *ic, hitevent );
                    if ( hitobject )
                    {
                        size_t initial_pos = m_caret;
                        SetCaret( *ic, xw, yw );
                        if ( m_selection_start_pos == initial_pos )
                        {
                            m_selection_start_pos = m_selection_end_pos;
                            m_selection_end_pos = initial_pos;
                        }
                        if ( m_selection_start_pos > m_caret )
                        {
                            m_selection_end_pos = m_selection_start_pos;
                            m_selection_start_pos = m_caret;
                        }
                        else
                            m_selection_end_pos = m_caret;
                        SetPending( true );
                    }
                    return;
                }
            }
            else if ( mouseevent.LeftUp() && busydrag )
            {
                busydrag = false;
                event.Skip();
            }
            else
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dText::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrict = GetHabitat()->GetRestrictionEngine();
        a2dHandle* draghandle = event.GetCanvasHandle();

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );
        a2dAffineMatrix origworld = m_lworld;

        if ( editmode == 0 && m_font.GetType() == a2dFONT_WXDC )
        {
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();

            a2dAffineMatrix atWorld = ic->GetTransform();
            a2dAffineMatrix inverse = ic->GetInverseTransform();

            double xwi;
            double ywi;
            inverse.TransformPoint( xw, yw, xwi, ywi );

            a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );

            a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );

            double xmin, ymin, xmax, ymax, w, h;
            xmin = untrans.GetMinX();
            ymin = untrans.GetMinY();
            xmax = untrans.GetMaxX();
            ymax = untrans.GetMaxY();
            w = untrans.GetWidth();
            h = untrans.GetHeight();

            if ( event.GetMouseEvent().LeftDown() )
            {
            }
            else if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__text_fontsize__" ) )
                {
                    original->SetTextHeight( GetTextHeight() );
                    original->SetNextLineDirection( GetNextLineDirection() );
                }
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                if ( draghandle->GetName() == wxT( "__text_fontsize__" ) )
                {
                    SetTextHeight( fabs( ywi ) );
                    SetNextLineDirection( ywi < 0 );
                    draghandle->SetPosXY( 0, ywi );
                }
                else if ( draghandle->GetName() == wxT( "__text_rotate__" ) )
                {
                    if( restrict )
                        restrict->RestrictPoint( xw, yw, a2dRestrictionEngine::snapToPointAngle );

                    double xr, yr;
                    m_lworld.TransformPoint( 0, 0, xr, yr );

                    //modify object
                    double dx, dy;

                    dx = xw - xr;
                    dy = yw - yr;
                    double angn;
                    if ( !dx && !dy )
                        angn = 0;
                    else
                    {
                        if ( m_alignment & wxMAXX )
                            angn = wxRadToDeg( atan2( -dy, -dx ) );
                        else
                            angn = wxRadToDeg( atan2( dy, dx ) );
                    }
                    m_lworld = m_lworld.Rotate( angn - m_lworld.GetRotation(), xr, yr );
                }
                else
                    event.Skip();
                SetPending( true );
            }
        }
        else
        {
            event.Skip();
        }
    }
    else
    {
        event.Skip();
    }
}

bool a2dText::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED(margin) )
{
    a2dPinClass* toCreate;
    if ( GetGeneratePins() && ( toCreate = toConnectTo->GetPinClassForTask( task, this ) ) )
    {
        double xmin = m_untransbbox.GetMinX();
        double ymin = m_untransbbox.GetMinY();
        double xmax = m_untransbbox.GetMaxX();
        double ymax = m_untransbbox.GetMaxY();

        double midx = ( m_untransbbox.GetMaxX() + m_untransbbox.GetMinX() ) / 2;
        double midy = ( m_untransbbox.GetMaxY() + m_untransbbox.GetMinY() ) / 2;

        m_flags.m_visiblechilds = true;
        a2dPin* newPin = NULL;
        if ( !HasPinNamed( wxT( "pinc*" ), true ) )
        {
            newPin = AddPin( wxT( "pinc" ), midx, midy, a2dPin::temporaryObjectPin, toCreate );
            newPin->SetInternal( true );
        }
        if ( !HasPinNamed( wxT( "pin1" ), true ) )
            AddPin( wxT( "pin1" ), xmin, ymin, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin2" ), true ) )
            AddPin( wxT( "pin2" ), midx, ymin, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin3" ), true ) )
            AddPin( wxT( "pin3" ), xmax, ymin, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin4" ), true ) )
            AddPin( wxT( "pin4" ), xmax, midy, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin5" ), true ) )
            AddPin( wxT( "pin5" ), xmax, ymax, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin6" ), true ) )
            AddPin( wxT( "pin6" ), midx, ymax, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin7" ), true ) )
            AddPin( wxT( "pin7" ), xmin, ymax, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin8" ), true ) )
            AddPin( wxT( "pin8" ), xmin, midy, a2dPin::temporaryObjectPin, toCreate );

        wxASSERT( HasPins( true ) );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );

        return true;
    }
    return false;
}

bool a2dText::AdjustPinLocation()
{
    if ( !HasPins() )
        return false;

    DoGetUnTransformedBbox();

    const double xmin = m_untransbbox.GetMinX();
    const double ymin = m_untransbbox.GetMinY();
    const double xmax = m_untransbbox.GetMaxX();
    const double ymax = m_untransbbox.GetMaxY();
    const double midx = ( m_untransbbox.GetMaxX() + m_untransbbox.GetMinX() ) / 2;
    const double midy = ( m_untransbbox.GetMaxY() + m_untransbbox.GetMinY() ) / 2;

    a2dPin* pin = NULL;
    if ( ( pin = HasPinNamed( wxT( "pinc*" ), false ) ) != NULL )
        pin->SetPosXY( midx, midy );
    if ( ( pin = HasPinNamed( wxT( "pin1" ),  false ) ) != NULL )
        pin->SetPosXY( xmin, ymin );
    if ( ( pin = HasPinNamed( wxT( "pin2" ),  false ) ) != NULL )
        pin->SetPosXY( midx, ymin );
    if ( ( pin = HasPinNamed( wxT( "pin3" ),  false ) ) != NULL )
        pin->SetPosXY( xmax, ymin );
    if ( ( pin = HasPinNamed( wxT( "pin4" ),  false ) ) != NULL )
        pin->SetPosXY( xmax, midy );
    if ( ( pin = HasPinNamed( wxT( "pin5" ),  false ) ) != NULL )
        pin->SetPosXY( xmax, ymax );
    if ( ( pin = HasPinNamed( wxT( "pin6" ),  false ) ) != NULL )
        pin->SetPosXY( midx, ymax );
    if ( ( pin = HasPinNamed( wxT( "pin7" ),  false ) ) != NULL )
        pin->SetPosXY( xmin, ymax );
    if ( ( pin = HasPinNamed( wxT( "pin8" ),  false ) ) != NULL )
        pin->SetPosXY( xmin, midy );

    return true;
}

//----------------------------------------------------------------------------
// a2dTextGDS
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dTextGDS, a2dText )

double a2dTextGDS::m_fontScaleFactor = 1.0;

a2dTextGDS::a2dTextGDS( a2dFontType fontType, const wxString& text, double height, short int presentation )
    : a2dText( text, 0.0, 0.0, *a2dDEFAULT_CANVASFONT )
{
    m_alignment = 0;
    m_presentationflags.total = presentation;
    SetHorizontal( m_presentationflags.bits.m_horizontal );
    SetVertical( m_presentationflags.bits.m_vertical );

    m_fontType = fontType;
    if ( m_fontType == a2dFONT_STROKED )
    {
        //we choose a precentage instead of another font.
        int weight = 2;
        switch( m_presentationflags.bits.m_font )
        {
            default:
            case 0 : weight = 2;
                break;
            case 1 : weight = 4;
                break;
            case 2 : weight = 8;
                break;
            case 3 : weight = 12;
                break;
        }
        a2dText::SetFont( a2dFont( wxT( "nen.chr" ), height, m_monoSpaced, weight ) );
    }
    if ( m_fontType == a2dFONT_WXDC )
    {
        wxFontFamily FontFamily = wxFONTFAMILY_MODERN; //default does not rotate if to small
        switch( m_presentationflags.bits.m_font )
        {
            case 0 : FontFamily = wxFONTFAMILY_DEFAULT; break;
            case 1 : FontFamily = wxFONTFAMILY_ROMAN; break;
            case 2 : FontFamily = wxFONTFAMILY_SWISS; break;
            case 3 : FontFamily = wxFONTFAMILY_DECORATIVE; break;
            default: FontFamily = wxFONTFAMILY_MODERN; break;
        }
        a2dText::SetFont( a2dFont( height, FontFamily, wxFONTSTYLE_NORMAL ) );
    }

    // a2dFont( wxT("nen.chr"), 0, true );
    //: a2dText( text, 0.0,0.0, a2dFont( 1.0, wxFONTFAMILY_SWISS ) )
    //: a2dText( text, 0.0,0.0, a2dFont( 1.0, wxFONTFAMILY_MODERN ) )
    //: a2dText( text, 0.0,0.0, a2dFont( wxT("LiberationSans-Regular.ttf" ) ) )

    m_texttype = 0;
    m_pathtype =    TEXT_PATH_END_SQAURE;
    m_width = 0;
    m_monoSpaced = true;
    SetBackGround( false );
}

a2dTextGDS::a2dTextGDS( const a2dTextGDS& other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs )
{
    m_presentationflags.total = other.m_presentationflags.total;
    m_texttype = other.m_texttype;
    m_pathtype = other.m_pathtype;
    m_monoSpaced = other.m_monoSpaced;
    m_width = other.m_width;
}

a2dObject* a2dTextGDS::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dTextGDS( *this, options, refs );
};

void a2dTextGDS::SetFontGDS( char nw_font )
{
    double h = GetTextHeight();
    if ( m_fontType == a2dFONT_STROKED )
    {
        m_font = a2dFont( wxT( "nen.chr" ), 0, m_monoSpaced );
        m_presentationflags.bits.m_font = nw_font;
        //we choose a precentage instead of another font.
        switch( nw_font )
        {
            default:
            case 0 : m_font.SetWeight( 2 );
                break;
            case 1 : m_font.SetWeight( 4 );
                break;
            case 2 : m_font.SetWeight( 8 );
                break;
            case 3 : m_font.SetWeight( 12 );
                break;
        }
        SetTextHeight( h );
    }
    if ( m_fontType == a2dFONT_WXDC )
    {
        wxFontFamily FontFamily = wxFONTFAMILY_MODERN; //default does not rotate if to small
        switch( nw_font )
        {
            case 0 : FontFamily = wxFONTFAMILY_DEFAULT; break;
            case 1 : FontFamily = wxFONTFAMILY_ROMAN; break;
            case 2 : FontFamily = wxFONTFAMILY_SWISS; break;
            case 3 : FontFamily = wxFONTFAMILY_DECORATIVE; break;
            default: FontFamily = wxFONTFAMILY_MODERN; break;
        }
        a2dText::SetFont( a2dFont( 1, FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ) );
        SetTextHeight( h );
    }
}

void a2dTextGDS::SetVertical( char nw_vertical )
{
    m_presentationflags.bits.m_vertical = nw_vertical;
    switch ( m_presentationflags.bits.m_horizontal )
    {
        case 0 :
        default:
            m_alignment = wxMINY | m_alignment;
            break;
        case 1 :
            m_alignment = wxMIDY | m_alignment;
            break;
        case 2 :
            m_alignment = wxMAXY | m_alignment;
            break;
    }
}


void a2dTextGDS::SetHorizontal( char nw_horizontal )
{
    m_presentationflags.bits.m_horizontal = nw_horizontal;
    switch ( m_presentationflags.bits.m_horizontal )
    {
        case 0 :
        default:
            m_alignment = wxMINX | m_alignment;
            break;
        case 1 :
            m_alignment = wxMIDX | m_alignment;
            break;
        case 2 :
            m_alignment = wxMAXX | m_alignment;
            break;
    }
}


short int a2dTextGDS::GetFontGDS()
{
    return ( short int ) m_presentationflags.bits.m_font;
}


short int a2dTextGDS::GetVertical()
{
    return ( short int ) m_presentationflags.bits.m_vertical;
}


short int a2dTextGDS::GetHorizontal()
{
    return ( short int ) m_presentationflags.bits.m_horizontal;
}

void a2dTextGDS::SetPresentationFlags(  short int the_total )
{
    m_presentationflags.total = the_total;
    SetHorizontal( m_presentationflags.bits.m_horizontal );
    SetVertical( m_presentationflags.bits.m_vertical );
    SetFontGDS( m_presentationflags.bits.m_font );
}

short int a2dTextGDS::GetPresentationFlags( )
{
    return m_presentationflags.total;
}

#if wxART2D_USE_CVGIO
void a2dTextGDS::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dText::DoLoad( parent, parser, xmlparts );
    m_utbbox_changed = true;
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_texttype = parser.GetAttributeValueInt( wxT( "texttype" ), 0 );
        m_pathtype = ( TEXT_PATHTYPE ) parser.GetAttributeValueInt( wxT( "pathtype" ), 0 );
        m_presentationflags.total = 0;
        SetVertical( parser.GetAttributeValueInt( wxT( "vertical" ), 0 ) );
        SetHorizontal( parser.GetAttributeValueInt( wxT( "horizontal" ), 0 ) );
        SetFontGDS( parser.GetAttributeValueInt( wxT( "fontgds" ), 0 ) );
    }
    else
    {
    }
}

void a2dTextGDS::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dText::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "texttype" ), m_texttype );
        out.WriteAttribute( wxT( "pathtype" ), m_pathtype );
        if ( GetPresentationFlags() )
        {
            // write only when a_presentation isn't default
            if ( GetHorizontal() != DEFAULT_PRESENTATION_HORIZONTAL )
            {
                out.WriteAttribute( wxT( "horizontal" ), GetHorizontal() );
            }
            // write only when a_presentation isn't default
            if ( GetVertical() != DEFAULT_PRESENTATION_VERTICAL )
            {
                out.WriteAttribute( wxT( "vertical" ), GetVertical() );
            }
            // write only when a_presentation isn't default
            if ( GetFontGDS() != DEFAULT_PRESENTATION_FONT )
            {
                out.WriteAttribute( wxT( "fontgds" ), GetFontGDS() );
            }
        }
        out.WriteNewLine();
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO



//===========================================================================
// a2dTextChanges
//===========================================================================

a2dFontInfoList a2dTextChanges::m_fontlist;
a2dFontInfoList a2dTextChanges::m_alreadyusedfonts;

a2dTextChanges::a2dTextChanges()
	: 
    m_Font(), 
    m_bType( false ),
    m_bFont( false ),
    m_bStyle( false ),
    m_bSize( false ),
    m_textflags( a2dText::a2dCANVASTEXT_DEFAULTFLAGS ),
    m_btextflags( false ),
    m_alignment( a2dDEFAULT_ALIGNMENT ),
    m_balignment( false )
{
	m_alreadyusedfonts.Clear();
}

a2dTextChanges::a2dTextChanges(const a2dFont& font, unsigned int textflags, int alignment )
{
	Set( font, textflags, alignment );
}

// Creates the list of available fonts once
// Optional: filter fonts, if a filled filterlist and filtertype is passed
void a2dTextChanges::InitialiseFontList(const wxArrayString &filterlist, const wxString filtertype)
{
	if(m_fontlist.GetCount()==0)
	{
		m_fontlist.DeleteContents( true );
		a2dFont::GetInfoList( m_fontlist );
		
		// optional filtering
		if( filterlist.GetCount() > 0 && filtertype != wxT( "" ) )
		{
			a2dFontInfoList reduced_list;
			for ( size_t i = 0; i < m_fontlist.GetCount(); i++ )
			{
				wxString fontname = m_fontlist.Item( i )->GetData()->GetName();
				wxString fonttype = m_fontlist.Item( i )->GetData()->GetType();

				for(int u = 0; u < filterlist.GetCount(); u++)
				{
					if ( ( fonttype == filtertype ) && fontname == filterlist[u] )
					{
						reduced_list.Append( new a2dFontInfo( *m_fontlist.Item( i )->GetData() ) );
					}
				}
			}
			m_fontlist.Clear();
			m_fontlist.insert( m_fontlist.end(), reduced_list.begin(), reduced_list.end() );	//copy reduced (filtered) list on member fontlist
		}
	}
}

// Initialises with <font>.
void a2dTextChanges::Set(const a2dFont& font, unsigned int textflags, int alignment, bool wrongloadflag )
{
	m_Font = font;
	m_bType = true;
	m_bFont = true;
	m_bName = true;
	m_bStyle = true;
	m_bSize = true;
    m_textflags = textflags;
    m_btextflags = true;
    m_alignment = alignment;
    m_balignment = true;

	if( wrongloadflag == false )
	{
		m_commonstyle.Clear();

		//start the list of common styles
		for ( size_t i = 0; i < m_fontlist.GetCount(); i++ )
		{
			wxString fontname = m_fontlist.Item( i )->GetData()->GetName();
			wxString fonttype = m_fontlist.Item( i )->GetData()->GetType();
			if ( ( fonttype == m_Font.GetFontInfo().GetType() || m_Font.GetFontInfo().GetType() == _( "* (All types)" ) )
						&& fontname == m_Font.GetFontInfo().GetName() )
			{
				wxString fontstyle = m_fontlist.Item( i )->GetData()->GetStyle();
				size_t j;
				for ( j = 0; j < m_commonstyle.GetCount(); j++ )
				{
					if ( m_commonstyle.Item( j ) == fontstyle )
						break;
				}
				if ( j == m_commonstyle.GetCount() )
					m_commonstyle.Add( fontstyle );
			}
		}
	}
}

// Compares with <font> and sets to invalid if unequal.
void a2dTextChanges::Mix(const a2dFont& font, unsigned int textflags, int alignment )
{
	if( m_bType && (font.GetFontInfo().GetType() != m_Font.GetFontInfo().GetType()) ) m_bType = false;
	if( m_bFont && (font.GetFontInfo().GetName() != m_Font.GetFontInfo().GetName()) ) 
	{
		m_bName = false;

		// -----------generate a list of common styles, when font is not the same-------------
		wxArrayString fonts1;

		//for speed optimization, search in list of already used fonts first
		for ( size_t i = 0; i < m_alreadyusedfonts.GetCount(); i++ )
		{
			wxString fontname = m_alreadyusedfonts.Item( i )->GetData()->GetName();
			wxString fonttype = m_alreadyusedfonts.Item( i )->GetData()->GetType();
			if ( ( fonttype == font.GetFontInfo().GetType() || font.GetFontInfo().GetType() == _( "* (All types)" ) )
					&& fontname == font.GetFontInfo().GetName() )
			{
				wxString fontstyle = m_alreadyusedfonts.Item( i )->GetData()->GetStyle();
				size_t j;
				for ( j = 0; j < fonts1.GetCount(); j++ )
				{
					if ( fonts1.Item( j ) == fontstyle )
						break;
				}
				if ( j == fonts1.GetCount() )
					fonts1.Add( fontstyle );
			}
		}

		if( fonts1.GetCount() == 0 )
		{
			for ( size_t i = 0; i < m_fontlist.GetCount(); i++ )
			{
				wxString fontname = m_fontlist.Item( i )->GetData()->GetName();
				wxString fonttype = m_fontlist.Item( i )->GetData()->GetType();
				if ( ( fonttype == font.GetFontInfo().GetType() || font.GetFontInfo().GetType() == _( "* (All types)" ) )
						&& fontname == font.GetFontInfo().GetName() )
				{
					wxString fontstyle = m_fontlist.Item( i )->GetData()->GetStyle();
					size_t j;
					for ( j = 0; j < fonts1.GetCount(); j++ )
					{
						if ( fonts1.Item( j ) == fontstyle )
							break;
					}
					if ( j == fonts1.GetCount() )
					{
						fonts1.Add( fontstyle );
						m_alreadyusedfonts.push_back( m_fontlist.Item( i )->GetData() );
					}
				}
			}
		}
		wxArrayString commonstyle_temp;
		for( int i = 0; i < fonts1.GetCount(); i++ )
		{
			for( int u = 0; u < m_commonstyle.GetCount(); u++ )
			{
				if( fonts1[i] == m_commonstyle[u])
				{
					commonstyle_temp.Add(fonts1[i]);
					break;
				}
			}
		}
		m_commonstyle=commonstyle_temp;
	}
	// -----------------------------------------------------------------------------------

	if( m_bStyle && (font.GetFontInfo().GetStyle() != m_Font.GetFontInfo().GetStyle()) ) m_bStyle = false;
	if( m_bSize && (font.GetFontInfo().GetSize() != m_Font.GetFontInfo().GetSize()) ) m_bSize = false;

	if( m_balignment && (m_alignment != alignment) ) m_balignment = false;
	if( m_btextflags && (m_textflags != textflags) ) m_btextflags = false;
}

a2dFont a2dTextChanges::GetFont(const a2dFont& font, wxArrayString valid_styles, wxArrayString valid_extras ) const
{
	a2dFont mixed=font;
	a2dFontInfo fontinfo=mixed.GetFontInfo();
	
	wxString  valid_style="", valid_extra;
	if( m_bType && (mixed.GetFontInfo().GetType() != m_Font.GetFontInfo().GetType()) )
	{
		fontinfo.SetType( m_Font.GetFontInfo().GetType() );
	}
	if( m_bName && (mixed.GetFontInfo().GetName() != m_Font.GetFontInfo().GetName()) )
	{
		fontinfo.SetName( m_Font.GetFontInfo().GetName() );

		// When style is invalid (e.g. the new font doesn't support the given style), choose "Regular" or the first one in the list
		if( !m_bStyle )	
		{
			for( int i = 0; i < valid_styles.GetCount() ; i++ )
			{
				if( ( valid_style == "" && valid_styles[i] == wxT( "Regular" ) ) || fontinfo.GetStyle() == valid_styles[i] )
				{
					valid_style = valid_styles[i];
					valid_extra = valid_extras[i];
				}
			}
			fontinfo.SetStyle( valid_style );
			fontinfo.SetExtra( valid_extra );
		}
	}
	if( m_bStyle && ( (mixed.GetFontInfo().GetStyle() != m_Font.GetFontInfo().GetStyle()) || (mixed.GetFontInfo().GetExtra() != m_Font.GetFontInfo().GetExtra()) ) )
	{
		fontinfo.SetStyle( m_Font.GetFontInfo().GetStyle() );
		if( m_bName )
			fontinfo.SetExtra( m_Font.GetFontInfo().GetExtra() );
		else
		{
			fontinfo.SetExtra( "" );

			//for speed optimization, search in list of already used fonts first
			for ( size_t i = 0; i < m_alreadyusedfonts.GetCount(); i++ )
			{
				wxString fonttype = m_alreadyusedfonts.Item( i )->GetData()->GetType();
				wxString fontname = m_alreadyusedfonts.Item( i )->GetData()->GetName();
				wxString fontstyle = m_alreadyusedfonts.Item( i )->GetData()->GetStyle();

				if ( ( fonttype == fontinfo.GetType() || fontinfo.GetType() == _( "* (All types)" ) )
					&& fontname == fontinfo.GetName() && fontstyle == m_Font.GetFontInfo().GetStyle() )
				{
					fontinfo.SetExtra( m_alreadyusedfonts.Item( i )->GetData()->GetExtra() );		//used when font-name was not changed -> only style was mixed
					break;
				}
			}

			if( fontinfo.GetExtra() == "" )
			{
				for ( size_t i = 0; i < m_fontlist.GetCount(); i++ )
				{
					wxString fonttype = m_fontlist.Item( i )->GetData()->GetType();
					wxString fontname = m_fontlist.Item( i )->GetData()->GetName();
					wxString fontstyle = m_fontlist.Item( i )->GetData()->GetStyle();

					if ( ( fonttype == fontinfo.GetType() || fontinfo.GetType() == _( "* (All types)" ) )
						&& fontname == fontinfo.GetName() && fontstyle == m_Font.GetFontInfo().GetStyle() )
					{
						fontinfo.SetExtra( m_fontlist.Item( i )->GetData()->GetExtra() );		//used when font-name was not changed -> only style was mixed
						m_alreadyusedfonts.push_back( m_fontlist.Item( i )->GetData() );
						break;
					}
				}
			}
		}
	}
	if( m_bSize && (mixed.GetFontInfo().GetSize() != m_Font.GetFontInfo().GetSize()) )
	{
		fontinfo.SetSize( m_Font.GetFontInfo().GetSize() );
	}

	mixed=a2dFont::CreateFont( fontinfo );
	return mixed;
}

int a2dTextChanges::GetAlignment( int Alignment ) const
{
	if(m_balignment) return m_alignment;
	return Alignment;
}

unsigned int a2dTextChanges::GetTextFlags( unsigned int textflags ) const
{
	if(m_btextflags) return m_textflags;
	return textflags;
}


wxUint32 SetTextChanges( a2dTextChanges& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask, a2dLayers* layersetup )
{
	int nr = 0;
    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;
    	a2dText* text = wxDynamicCast( obj, a2dText );

        if ( obj->GetFixedStyle() ||  obj->GetRelease() || !obj->CheckMask( mask ) || !text )
            continue;

        obj->SetBin2( true );

        a2dFont font = text->GetFont();

        if ( !nr )
        {
            returned.Set( font, text->GetTextFlags(), text->GetAlignment() );
        }
        else
            returned.Mix( font, text->GetTextFlags(), text->GetAlignment() );
        nr++;
    }
    return nr;
}
