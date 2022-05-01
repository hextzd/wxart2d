
// *****************************************************************
// wx/xmlparse\genprop.h
// *****************************************************************

// *****************************************************************
// wx/xmlparse\genxmlpars.h
// *****************************************************************

enum XMLeventType
{
    START_DOCUMENT,
    END_DOCUMENT,
    START_TAG,
    END_TAG,
    CONTENT
};

// ---------------------------------------------------------------------------
// a2dIOHandlerXmlSerIn
// ---------------------------------------------------------------------------

#include "wx/xmlparse/genxmlpars.h"
class a2dIOHandlerXmlSerIn : public a2dIOHandlerXMLPull
{
    a2dIOHandlerXmlSerIn()
    bool CanLoad(a2dDocumentInputStream& stream, const wxObject* obj )
    virtual bool Load( a2dDocumentInputStream& stream , wxObject* doc )
    a2dObject* LoadOneObject( wxObject* parent )    
};

// ---------------------------------------------------------------------------
// a2dIOHandlerXMLWrite
// ---------------------------------------------------------------------------

#include "wx/xmlparse/genxmlpars.h"
class a2dIOHandlerXMLWrite : public a2dIOHandlerStrOut
{
    // %abstract a2dIOHandlerXMLWrite()
    virtual void InitializeSave( )
    virtual void ResetSave()
    int GetDepth()
    void EndlWriteString( const wxString& string )
    void WriteStartDocument( const wxString& version,  const wxString& encoding, bool standalone )
    void WriteEndDocument()
    void WriteStartElement( const wxString& name, bool newLine = true  )
    void WriteStartElementAttributes( const wxString& name, bool newLine = true  )
    void WriteStartElement( const a2dXMLTag& tag, bool newLine = true  )
    void WriteElement( const wxString& name, const wxString& content = "", bool newLine = true )
    void WriteAttribute(const wxString& name, const size_t & value)
    void WriteAttribute( const wxString& name, bool value, bool onlyIfTrue = false )
    void WriteAttribute( const a2dNamedProperty& attribute )
    void WriteEndAttributes(bool close = false)
    void WriteEndElement( bool newLine = true )
    void WriteContent( const wxString& content )
    void WriteNewLine()
    void SetIndentSize( int size )
    virtual int GetCurrentColumnNumber()
    virtual int GetCurrentLineNumber()
    //void XmlEncodeStringToStream( const wxChar *string, size_t len = wxString::npos )
    virtual void WriteDouble(double d)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxInt16 & c)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxInt32 & c)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxInt64 & c)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxUint16 & c)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxUint32 & c)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxUint64 & c)
    %operator a2dIOHandlerXMLWrite& operator<<(const double & f)
    %operator a2dIOHandlerXMLWrite& operator<<(const float & f)
    //%operator a2dIOHandlerXMLWrite& operator<<(const wxChar *string)
    %operator a2dIOHandlerXMLWrite& operator<<(const wxString& string)
    %operator a2dIOHandlerXMLWrite& operator<<(char c)
    %operator a2dIOHandlerXMLWrite& operator<<(wchar_t wc)
};

// ---------------------------------------------------------------------------
// a2dIOHandlerXmlSerOut
// ---------------------------------------------------------------------------

#include "wx/xmlparse/genxmlpars.h"
class a2dIOHandlerXmlSerOut : public a2dIOHandlerXMLWrite
{
    a2dIOHandlerXmlSerOut()
    bool CanSave( const wxObject* obj = NULL )
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc )
};

// ---------------------------------------------------------------------------
// a2dXMLTag
// ---------------------------------------------------------------------------

#include "wx/xmlparse/genxmlpars.h"
class a2dXMLTag : public a2dObject
{
    a2dXMLTag( XMLeventType type, wxString tagname = "" )
    a2dXMLTag(const a2dXMLTag& tag)
    virtual a2dObject* Clone( a2dObject::CloneOptions options ) const
    wxString GetName() const
    bool SetAttribute( const wxString& attributeName, const wxString& Value )
    wxString* GetAttribute( const wxString& attributeName )
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
    %member wxString m_tag
    %member wxString m_text
    %member XMLeventType m_type
    %member int m_line
    %member int m_column
    %member bool m_hasContent
    %member bool m_hasattributes
};

// ---------------------------------------------------------------------------
// a2dIOHandlerXMLPull
// ---------------------------------------------------------------------------

#include "wx/xmlparse/genxmlpars.h"
class a2dIOHandlerXMLPull : public a2dIOHandlerStrIn
{
    // %abstract a2dIOHandlerXMLPull()
    a2dXMLTag* GetCurrentTag()
    virtual void InitializeLoad( )
    virtual void ResetLoad()
    void Require( const XMLeventType& type, wxString name)
    wxString GetTagName()
    wxString GetContent()
    XMLeventType GetEventType()
    int GetDepth()
    XMLeventType NextTag()
    XMLeventType Next()
    wxString GetAttributeValue( const wxString& attrib, const wxString& defaultv = "" )
    wxString RequireAttributeValue( const wxString& attrib )
    double GetAttributeValueDouble( const wxString& attrib, double defaultv = 0 )
    double RequireAttributeValueDouble( const wxString& attrib )
    int GetAttributeValueInt( const wxString& attrib, int defaultv = 0 )
    wxUint16 GetAttributeValueUint16( const wxString& attrib, wxUint16 defaultv = 0 )
    wxUint32 GetAttributeValueUint32( const wxString& attrib, wxUint32 defaultv = 0 )
    int RequireAttributeValueInt( const wxString& attrib )
    long GetAttributeValueLong( const wxString& attrib, long defaultv = 0 )
    long RequireAttributeValueLong( const wxString& attrib )
    bool GetAttributeValueBool( const wxString& attrib, bool defaultv = false )
    bool RequireAttributeValueBool( const wxString& attrib )
    bool HasAttribute( const wxString& attrib )
    void SkipSubTree()
    virtual int GetCurrentColumnNumber()
    virtual int GetCurrentLineNumber()
    virtual wxString GetErrorString()
       
    double ParseDouble( const wxString& buffer, unsigned int& position )
    void SkipCommaSpaces( const wxString& buffer, unsigned int& position )
    void SkipSpaces( const wxString& buffer, unsigned int& position )
    
};

// ---------------------------------------------------------------------------
// a2dXmlString
// ---------------------------------------------------------------------------

#include "wx/xmlparse/genxmlpars.h"
class a2dXmlString : public a2dIOHandlerXMLWrite
{
    a2dXmlString( const wxString& str = "" )
    virtual void Reset()
    void WriteCommand( const wxString& name, bool close = true )
    const wxString& GetString() const
    const wxString GetString() const
    bool CanSave( const wxObject* obj = NULL )
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc )
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc )
};

// *****************************************************************
// wx/xmlparse\xmlencode.h
// *****************************************************************

%function wxString XmlDecodeStringToString(    const char* s, size_t len = wxString::npos );
