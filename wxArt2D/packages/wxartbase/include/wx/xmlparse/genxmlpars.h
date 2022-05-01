/*! \file wx/xmlparse/genxmlpars.h
    \brief - XML I/O classes which is Pull parser based for reading XML files.

    The serialization of documents is XML based, and streamimg in or out into files is
    in several format cases also in XML.
    The classes here integrate the a2dIOHandlerStrIn and a2dIOHandlerStrOut with
    the XML parsing and XML writing classes.
    IO in and output for XML based formats do derive from the classes here.

    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: genxmlpars.h,v 1.34 2009/07/15 21:23:43 titato Exp $
*/
#ifndef _WX_DOCVIEWXMLIO_H_
#define _WX_DOCVIEWXMLIO_H_

#include "wxartbaseprivate.h"

#if wxART2D_USE_XMLPARSE

#include <wx/defs.h>
#include <wx/string.h>

#include <expat.h>

#include "wx/general/genmod.h"

//! For exceptions thrown while parsing XML files
/*!
    \ingroup fileio
*/
class a2dIOHandlerXMLException
{

public:
    //! Constructor.
    a2dIOHandlerXMLException( const wxString& exMessage ): m_message( exMessage ) {}

    //! Constructor with row and line params
    a2dIOHandlerXMLException( const wxString& exMessage, int exRow, int exColumn )
        : m_message( exMessage ), m_row( exRow ), m_column( exColumn ) {}

    //! Copy constructor.
    a2dIOHandlerXMLException( const a2dIOHandlerXMLException& other )
    {  m_message       = other.m_message; }

    a2dIOHandlerXMLException& operator=( const a2dIOHandlerXMLException& other )
    {
        if ( &other != this )
        {
            m_message = other.m_message;
        }

        return *this;
    }

    virtual ~a2dIOHandlerXMLException() {}

    int getLineNumber() const { return m_row; }
    int getColumnNumber() const { return m_column; }

    wxString getMessage() const  { return m_message; }
    void setMessage( const wxString& exMessage )  { m_message = exMessage; }

protected:

    wxString m_message;
    int m_row;
    int m_column;
};

//! Types of XML events when reading files.
/*!
    \ingroup fileio
*/
enum XMLeventType
{
    NO_TAG, /*!< not set yet */
    START_DOCUMENT, /*!< Start of document */
    END_DOCUMENT,   /*!< End of document */
    START_TAG,      /*!< Start tag */
    END_TAG,        /*!< End tag */
    CONTENT         /*!< Content between start and end tag */
};

//! translate XMLeventType to a string
wxString GetTypeString( const XMLeventType& type );

#include <wx/variant.h>


//! wxString keys to hold attributes
/*!
*/
class A2DGENERALDLLEXP a2dAttributeHash;
class A2DGENERALDLLEXP a2dAttributeHash_wxImplementation_HashTable;
class A2DGENERALDLLEXP a2dAttributeHash_wxImplementation_KeyEx;
WX_DECLARE_STRING_HASH_MAP_WITH_DECL( wxString, a2dAttributeHash, class A2DGENERALDLLEXP );


//! used in XML parsing to hold one tag and its atributes
/*!
    The a2dIOHandlerXMLPull and derived classes use this object
    to parse XML documents.

    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dXMLTag : public a2dObject
{
public:
    //! Constructor.
    /*!
        \see XMLeventType

        \param type    the event type
        \param tagname the tag name
    */
    a2dXMLTag( XMLeventType type = NO_TAG, wxString tagname = wxT( "" ) );

    //! Copy constructor.
    /*!
        \param tag the original tag
    */
    a2dXMLTag( const a2dXMLTag& tag );

    //! Destructor.
    ~a2dXMLTag();

    //! create an exact copy of this property
    virtual a2dObject* Clone( CloneOptions options ) const;

    wxString GetName() const { return m_tag; }

    bool SetAttribute( const wxString& attributeName, const wxString& Value );

    wxString* GetAttribute( const wxString& attributeName );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //! tag name
    wxString m_tag;

    //! Content of the tag
    wxString m_text;

    //! event type
    XMLeventType m_type;

    int m_line;

    int m_column;

    //! flag to trigger if content was written
    bool m_hasContent;

    //! does the current XMLTag have attributes?
    bool m_hasattributes;

    a2dAttributeHash m_attributes;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dXMLTag>;
template class A2DGENERALDLLEXP std::allocator<class a2dSmrtPtr<class a2dXMLTag> >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dXMLTag>, std::allocator<class a2dSmrtPtr<class a2dXMLTag> > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dXMLTag>, std::allocator<class a2dSmrtPtr<class a2dXMLTag> > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list<class a2dSmrtPtr<class a2dXMLTag> >;
template class A2DGENERALDLLEXP a2dlist<class a2dSmrtPtr<class a2dXMLTag> >;
template class A2DGENERALDLLEXP a2dSmrtPtrList< a2dXMLTag >;
#endif

const size_t XMLBUFSIZE = 1024;

class A2DGENERALDLLEXP a2dIOHandlerXMLPull;


//! Input and output handler for XML alike files
/*!
    The file is parsed by pulling information from it.
    One chunk of information is a XML begin_tag + attributes or an end_tag or content between
    the tags.
    The caller decides what should be pulled next after a reading chunk.
    This makes it very easy to read known fixes file formats, since one can
    directly store the information read into an object of choice.
    This is ideal for mapping a certain XML input format to the programs
    internal format.

    See also http://www.xmlpull.org/ for further details about XML pull parsing.

    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dIOHandlerXMLPull : public a2dIOHandlerStrIn
{
public:
    //! Constructor.
    /*!
        XML Pull parser
    */
    a2dIOHandlerXMLPull();

    //! Destructor.
    ~a2dIOHandlerXMLPull();

    //! return the tag where the stream is situated
    a2dXMLTag* GetCurrentTag() { return m_current; }

    //! Inits the IO handler for reading
    /*!
        \see a2dIOHandlerXML::InitializeLoad for description and remarks
    */
    virtual void InitializeLoad( );

    //! Reset the object after loading
    virtual void ResetLoad();

    double ParseDouble( const wxString& buffer, unsigned int& position ) ;
    void SkipCommaSpaces( const wxString& buffer, unsigned int& position );
    void SkipSpaces( const wxString& buffer, unsigned int& position );

    //! Forces a special tag
    /*!
        If the current XMLeventType and the tag name is not the required type and
        name, an a2dIOHandlerXMLException will occur.

        \see a2dIOHandlerXMLException

        \param type the required tag type
        \param name the required tag name
    */
    void Require( const XMLeventType& type, wxString name );

    //! Returns name of the current XML tag
    /*!
        This method returns the name of the current XML tag

        \return tag name
    */
    wxString GetTagName();

    //! Returns the current content
    /*!
        Returns the content between the opening XML tag and the ending tag

        \return content between tag
    */
    virtual wxString GetContent() = 0;

    //! Returns the type of current event
    /*!
        The event type may be one of the following:
        - START_DOCUMENT
        - END_DOCUMENT
        - START_TAG
        - END_TAG
        - CONTENT

        \see XMLeventType

        \return the event type
    */
    XMLeventType GetEventType();

    //! Returns depth.
    int GetDepth() { return m_depth; }

    //! Walks to next tag
    /*!
        Does internally a call to Next() and checks if the XMLEventType is an
        <code>START_TAG</code> or <code>END_TAG</code>, otherwise an
        a2dIOHandlerXMLException is thrown.

        \see a2dIOHandlerXMLException
        \see Next

        \return the event type
    */
    XMLeventType NextTag();

    //! Walks to next element and returns event type.
    /*!
        Goes to the next element and returns the event type of the
        element.

        \see NextTag

        \return the event type
    */
    XMLeventType Next();


    //! Returns the value of an attribute
    /*!
        Returns the value of the attribute "attrib"

        \param  attrib   the attribute name
        \param  defaultv the default value (empty string by default)

        \return the value of the attribute if found or the default value
    */
    wxString GetAttributeValue( const wxString& attrib, const wxString& defaultv = wxT( "" ) );

    //! Forces an attribute and returns its string value.
    /*!
        Walks through the attributes of the current start tag and
        returns the value of the requested attribute.
        If the attribute is not found, an a2dIOHandlerXMLException will be
        thrown.

        \see a2dIOHandlerXMLException

        \param  attrib the attribute name

        \return the attribute value
    */
    wxString RequireAttributeValue( const wxString& attrib );

    //! Returns the double value of an attribute
    /*!
        \see GetAttributeValue

        \param  attrib   the attribute name
        \param  defaultv the default value (0 by default)

        \return the value of the attribute if found or the default value
    */
    double GetAttributeValueDouble( const wxString& attrib, double defaultv = 0 );

    //! Forces an attribute and returns its double value.
    /*!
        \see RequireAttributeValue

        \param  attrib the attribute name

        \return the attribute value
    */
    double RequireAttributeValueDouble( const wxString& attrib );

    wxLongLong GetAttributeValueLongLong( const wxString& attrib, wxLongLong defaultv = 0 );

    wxLongLong RequireAttributeValueLongLong( const wxString& attrib );

    //! Returns the integer value of an attribute
    /*!
        \see GetAttributeValue

        \param  attrib   the attribute name
        \param  defaultv the default value (0 by default)

        \return the value of the attribute if found or the default value
    */
    int GetAttributeValueInt( const wxString& attrib, int defaultv = 0 );

    //! cast to wxUint16 of  GetAttributeValueInt()
    inline wxUint16 GetAttributeValueUint16( const wxString& attrib, wxUint16 defaultv = 0 )
    { return ( wxUint16 ) GetAttributeValueInt( attrib, ( int ) defaultv ); }

    //! cast to wxUint32 of  GetAttributeValueInt()
    inline wxUint32 GetAttributeValueUint32( const wxString& attrib, wxUint32 defaultv = 0 )
    { return ( wxUint32 ) GetAttributeValueLong( attrib, ( long ) defaultv ); }

    //! Forces an attribute and returns its integer value.
    /*!
        \see RequireAttributeValue

        \param  attrib the attribute name

        \return the attribute value
    */
    int RequireAttributeValueInt( const wxString& attrib );

    //! Returns the long value of an attribute
    /*!
        \see GetAttributeValue

        \param  attrib   the attribute name
        \param  defaultv the default value (0 by default)

        \return the value of the attribute if found or the default value
    */
    long GetAttributeValueLong( const wxString& attrib, long defaultv = 0 );

    //! Forces an attribute and returns its long integer value.
    /*!
        \see RequireAttributeValue

        \param  attrib the attribute name

        \return the attribute value
    */
    long RequireAttributeValueLong( const wxString& attrib );


    //! Returns the boolean value of an attribute
    /*!
        Attribute values like "true", "true", "True"...
        will be returned as boolean (case-insensitive).

        \remark
        An attribute value of "1" or "0" are not valid boolean values.

        \see GetAttributeValue

        \param  attrib   the attribute name
        \param  defaultv the default value

        \return the value of the attribute if found or the default value
    */
    bool GetAttributeValueBool( const wxString& attrib, bool defaultv = false );

    //! Forces an attribute and returns its boolean value.
    /*!
        \see RequireAttributeValue

        \param  attrib the attribute name

        \return the attribute value
    */
    bool RequireAttributeValueBool( const wxString& attrib );

    //! Does the current tag have this attribute?
    bool HasAttribute( const wxString& attrib );

    //! Skips all child elements / tags of current element / tag.
    void SkipSubTree();

    //! where in the input was column the current tag
    virtual int GetCurrentColumnNumber();

    //! where in the input was line the current tag
    virtual int GetCurrentLineNumber();

    //! Returns last error as string
    virtual wxString GetErrorString();

    //! Scale factor to scale input numbers (int double etc. ) with this.
    //! This can be used by a derived IO handler to scale input numbers.
    void SetScale( double scale ) { m_scale = scale; }

    //! See GetScale()
    double GetScale() { return m_scale; }

protected:

    static wxString m_encoding;

    static wxString m_version;

    //! Reads next piece of document into buffer.
    virtual void FillQueue() = 0;

    //! queue of begin and end elements tags not yet processed
    a2dSmrtPtrList<a2dXMLTag> m_elements;

    int m_depth;

    double m_scale;

    a2dXMLTag* m_startdoc;
    a2dXMLTag* m_enddoc;

    //! current XML tag
    a2dXMLTag* m_current;
};

//! Input and output handler for XML alike files
/*!
    The file is parsed by pulling information from it.
    One chunk of information is a XML begin_tag + attributes or an end_tag or content between
    the tags.
    The caller decides what should be pulled next after a reading chunk.
    This makes it very easy to read known fixes file formats, since one can
    directly store the information read into an object of choice.
    This is ideal for mapping a certain XML input format to the programs
    internal format.

    See also http://www.xmlpull.org/ for further details about XML pull parsing.

    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dIOHandlerXMLPullExpat : public a2dIOHandlerXMLPull
{
public:
    //! Constructor.
    /*!
        XML Pull parser
    */
    a2dIOHandlerXMLPullExpat();

    //! Destructor.
    ~a2dIOHandlerXMLPullExpat();

    //! return the tag where the stream is situated
    a2dXMLTag* GetCurrentTag() { return m_current; }

    //! Inits the IO handler for reading
    /*!
        \see a2dIOHandlerXML::InitializeLoad for description and remarks
    */
    virtual void InitializeLoad( );

    //! Reset the object after loading
    virtual void ResetLoad();


    //! Returns the current content
    /*!
        Returns the content between the opening XML tag and the ending tag

        \return content between tag
    */
    wxString GetContent();

    //! where in the input was column the current tag
    int GetExpatCurrentColumnNumber();

    //! where in the input was line the current tag
    int GetExpatCurrentLineNumber();

    wxString GetExpatErrorString();

protected:

    char* GetBuffer() { return (char*) XML_GetBuffer( m_parser, XMLBUFSIZE ); }

    bool ParseBuffer( size_t& bytes_read, bool& done ) { return XML_ParseBuffer( m_parser, bytes_read, done ) == XML_STATUS_OK; }

    //! Reads next piece of document into buffer.
    void FillQueue();

    static void BaseStartElementHnd( void* userData, const char* name, const char** atts );
    static void BaseEndElementHnd( void* userData, const char* name );
    static void BaseCharacterDataHnd( void* userData, const char* s, int len );
    static void BaseCommentHnd( void* userData, const char* data );
    static void BaseDefaultHnd( void* userData, const char* s, int len );

    //! expat parser
    XML_Parser m_parser;

    //!buffer used for passing part of the input stream
    char* m_buffer;

    //! true if reached end of document
    bool m_done;

};

#if wxDOCVIEW_USE_IRRXML

#include <irrXML.h>
using namespace irr;
using namespace io;

//! Input and output handler for XML alike files
/*!
    The file is parsed by pulling information from it.
    One chunk of information is a XML begin_tag + attributes or an end_tag or content between
    the tags.
    The caller decides what should be pulled next after a reading chunk.
    This makes it very easy to read known fixes file formats, since one can
    directly store the information read into an object of choice.
    This is ideal for mapping a certain XML input format to the programs
    internal format.

    See also http://www.xmlpull.org/ for further details about XML pull parsing.

    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dIOHandlerXMLPull2 : public a2dIOHandlerXMLPull
{
public:
    //! Constructor.
    /*!
        \see a2dIOHandlerXML::a2dIOHandlerXML for description and remarks
    */
    a2dIOHandlerXMLPull2();

    //! Destructor.
    ~a2dIOHandlerXMLPull2();

    //! Inits the IO handler for reading
    /*!
        \see a2dIOHandlerXML::InitializeLoad for description and remarks
    */
    virtual void InitializeLoad( );

    //! Reset the object after loading
    virtual void ResetLoad();

    wxString GetContent();

protected:

    //! Reads next piece of document into buffer.
    void FillQueue();

    //! expat parser
    IrrXMLReader* m_parser;
};

#endif //wxDOCVIEW_USE_IRRXML

//! for writing XML files.
/*!
    basic function for writing tags and attributes to a stream.
*/
class A2DGENERALDLLEXP a2dIOHandlerXMLWrite : public a2dIOHandlerStrOut
{
public:
    //! Constructor.
    /*!
    */
    a2dIOHandlerXMLWrite();

    //! Destructor.
    ~a2dIOHandlerXMLWrite();

    //! Inits the IO handler for writing
    /*!
        \see a2dIOHandlerXML::InitializeSave for description and remarks
    */
    virtual void InitializeSave( );

    //! Reset the object after saving
    virtual void ResetSave();

    //output routines

    //! Returns depth.
    int GetDepth() { return m_depth; }

    //! Writes a userdefined string into document.
    /*!
        The string might be a comment or a DTD declaration.

        \remark
        There's no check if it's a valid XML-syntax conform string.
        Use this method with care.

        \param string   the string to write

        newLine is a line break _before_ string
    */
    void EndlWriteString( const wxString& string );

    //! Writes the XML header declaration.
    /*!
        Each XML document should begin with a XML header / prolog, which
        informes about the XML version and the document encoding.

        \remark
        It's highly recommended, that you write this header, otherwise you
        might run into troubles.

        \param version    the XML version (currently only "1.0" is valid)
        \param encoding   the encoding of the document
        \param standalone <code>true</code> if it's a standalone xml doc, else <code>false</code>
                          to indicate that it has an external DTD

    */
    void WriteStartDocument( const wxString& version,  const wxString& encoding, bool standalone );

    //! Checks if all open tags are closed
    /*!
        This method does not write anything, but checks if there're open
        tags with no close tag and gives an ASSERT msg.

        This method is very useful for debuging proposes and should be called.

    */
    void WriteEndDocument();

    //! Writes start tag which has no attributes.
    /*!
        \remark
        If the start tag should have attributes, you've to call
        WriteStartElementAttributes.

        \see WriteStartElementAttributes

        \param name    the tag name
        \param newLine <code>true</code> if there should be a
                       line break _before_ the tag, else <code>false</code>
    */
    void WriteStartElement( const wxString& name, bool newLine = true  );

    //! Writes start tag which has attributes
    /*!
        \remark
        Don't forget to call WriteEndAttributes when you've written all
        attributes.

        \see WriteEndAttributes

        \param name    the tag name
        \param newLine <code>true</code> if there should be a
                       line break _before_ the tag, else <code>false</code>
    */
    void WriteStartElementAttributes( const wxString& name, bool newLine = true  );

    //! Writes start tag.
    void WriteStartElement( const a2dXMLTag& tag, bool newLine = true  );

    //! Writes start and end tag.
    /*!
        This method writes a complete set of tags

        \param name    the tag name
        \param content the content between the start an end tag
        \param newLine <code>true</code> if there should be a
                       line break _before_ the tag, else <code>false</code>
    */
    void WriteElement( const wxString& name, const wxString& content = wxT( "" ), bool newLine = true );

    // ! We have to hack here because size_t is defined as 'unsigned long'
    // and wxUint64 is defined as 'unsigned long long'.
    template<typename w_TypeRef>
    void WriteAttribute( const wxString& name, const w_TypeRef& value )
    {
        wxString error = wxT( "Wrong start tag" ) + this->m_current->m_tag;
        wxASSERT_MSG(  this->m_current->m_hasattributes , error );
        *this << wxT( " " ) << name << wxT( "=\"" ) << value << wxT( "\"" );
    }

    void WriteAttribute( const wxString& name, const size_t& value )
    {
        wxString error = wxT( "Wrong start tag" ) + this->m_current->m_tag;
        wxASSERT_MSG(  this->m_current->m_hasattributes , error );
        *this << wxT( " " ) << name << wxT( "=\"" ) << ( wxUint32 ) value << wxT( "\"" );
    }

    void WriteAttributeEnt( const wxString& name, const wxString& value );

    //! Writes an attribute with boolean value.
    /*!
        The boolean value will be written lower-case ("true" / "false")

        \see WriteStartElementAttributes

        \param name  the attribute name
        \param value the attribute value
        \param onlyIfTrue if true the attribute is only written when value is true.
    */
    void WriteAttribute( const wxString& name, bool value, bool onlyIfTrue = false );

    //! Writes an a2dXMLAttribute.
    void WriteAttribute( const a2dNamedProperty& attribute );

    //! "Closes" the start tag after writing all attributes (writes the ">" or "/>" bracket).
    /*!
        \param close If set to <code>true</code> a "/>" (short tag) will be written, else ">"
        \see WriteStartElementAttributes
    */
    void WriteEndAttributes( bool close = false );

    //! Writes correspondending end tag for the current start tag.
    /*!
        \param newLine <code>true</code> if there should be a
                       line break _before_ the tag, else <code>false</code>
    */
    void WriteEndElement( bool newLine = true );

    //! Writes content between start and end tag.
    /*!
        \param content the content of the current tag
    */
    void WriteContent( const wxString& content );

    //! Writes a new line and takes care of indentation.
    void WriteNewLine();

    //! number of spaces in one indentation
    void SetIndentSize( int size ) { m_indentSize = size; }

    virtual int GetCurrentColumnNumber();

    virtual int GetCurrentLineNumber();

    void XmlEncodeStringToStream( const wxChar* string, size_t len = wxString::npos );

    //! write a double number.
    virtual void WriteDouble( double d )
    {
        wxString str = wxString::FromCDouble( d );
        WriteString( str );
    }

    void WriteStringEnt( const wxString& string );

    a2dIOHandlerXMLWrite& operator<<( const wxInt16& c );
    a2dIOHandlerXMLWrite& operator<<( const wxInt32& c );
    a2dIOHandlerXMLWrite& operator<<( const wxInt64& c );
    a2dIOHandlerXMLWrite& operator<<( const wxLongLong& c );
    a2dIOHandlerXMLWrite& operator<<( const wxULongLong& c );
    a2dIOHandlerXMLWrite& operator<<( const wxUint16& c );
    a2dIOHandlerXMLWrite& operator<<( const wxUint32& c );
    a2dIOHandlerXMLWrite& operator<<( const wxUint64& c );
    a2dIOHandlerXMLWrite& operator<<( const double& f );
    a2dIOHandlerXMLWrite& operator<<( const float& f );

    a2dIOHandlerXMLWrite& operator<<( const wxChar* string );
    a2dIOHandlerXMLWrite& operator<<( const wxString& string );
    a2dIOHandlerXMLWrite& operator<<( char c );
#if wxUSE_UNICODE && wxWCHAR_T_IS_REAL_TYPE
    a2dIOHandlerXMLWrite& operator<<( wchar_t wc );
#endif // wxUSE_UNICODE

    //! Scale factor to scale output numbers (int double etc. ) with this.
    //! This can be used by a derived IO handler to scale output numbers.
    void SetScale( double scale ) { m_scale = scale; }

    //! See GetScale()
    double GetScale() { return m_scale; }

    void SetIndent( int indent ) { m_indent = indent; }

    int GetIndent() { return m_indent; }

protected:

    void WriteIndent();

    int m_indent;

    int m_indentSize;

    int m_depth;

    double m_scale;

    //! queue of begin and end elements tags not yet processed
    a2dSmrtPtrList<a2dXMLTag> m_elements;

    a2dXMLTag* m_startdoc;
    a2dXMLTag* m_enddoc;

    //! current XML tag
    a2dXMLTag* m_current;
};


#if wxART2D_USE_CVGIO
//! Input and output handler for the XmlSer format.
/*!
    \ingroup fileio
*/
#if wxDOCVIEW_USE_IRRXML
    class A2DGENERALDLLEXP a2dIOHandlerXmlSerIn : public a2dIOHandlerXMLPull2
#else
    class A2DGENERALDLLEXP a2dIOHandlerXmlSerIn : public a2dIOHandlerXMLPullExpat
#endif //wxDOCVIEW_USE_IRRXML
{
    DECLARE_DYNAMIC_CLASS( a2dIOHandlerXmlSerIn )
public:
    //! Constructor.
    a2dIOHandlerXmlSerIn();

    //! Destructor.
    ~a2dIOHandlerXmlSerIn();

    //! test header of the file to see if its CVG format
    bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo = NULL );

    //! load a complete document
    /*!
        A pointer of this class is passed to the a2dCanvasDocument,
        which then loads itself using this parser.
        The attribute classname in the CVG format is used to create the right
        object while prasing the CVG and call the Load routine for that object.
    */
    virtual bool Load( a2dDocumentInputStream& stream , wxObject* doc );

    //! load one object from a CVG file.
    a2dObject* LoadOneObject( wxObject* parent );

    //! set version of library or document being parsed for
    void SetFormatVersion( wxString formatVersion ) { m_formatVersion = formatVersion; }

    //! get version of library or document
    wxString GetFormatVersion() { return m_formatVersion; }

protected:

    wxString m_formatVersion;

    //! the document to store/load the data found into
    wxObject* m_doc;

};

//! Input and output handler for the XmlSer format.
/*!
    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dIOHandlerXmlSerOut : public a2dIOHandlerXMLWrite
{
    DECLARE_DYNAMIC_CLASS( a2dIOHandlerXmlSerOut )
public:
    //! Constructor.
    a2dIOHandlerXmlSerOut();

    //! Destructor.
    ~a2dIOHandlerXmlSerOut();


    //! this handler can also save as CVG
    bool CanSave( const wxObject* obj = NULL ) { return true; }

    //! save a2dCanvasDocument as CVG
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc );

protected:

    //! the document to store/load the data found into
    wxObject* m_doc;
};
#endif //wxART2D_USE_CVGIO

//! Input and output handler for the XmlSer format.
/*!
    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dXmlString : public a2dIOHandlerXMLWrite
{
public:
    //! Constructor.
    a2dXmlString( const wxString& str = wxT( "" ) );

    //! Destructor.
    ~a2dXmlString();

    virtual void Reset();

    //! write a command string
    /*!
        \param name value for name attribute of the command
        \param close if true close the command tag, else more attributes can be added
    */
    void WriteCommand( const wxString& name, bool close = true );

#if wxUSE_STD_IOSTREAM
    const wxString& GetString() const
    {
        static wxString aap = wxString( m_strstream.str().c_str(), wxConvUTF8 );
        return aap;
    }
#else
    const wxString GetString() const { return m_strstream.GetString(); }
#endif

    //! this handler can also save as CVG
    bool CanSave( const wxObject* obj = NULL ) { return true; }

    //! save a2dCanvasDocument as CVG
    virtual bool Save( a2dDocumentOutputStream& WXUNUSED( stream ), const wxObject* WXUNUSED( doc ) ) { return true; }

protected:

    a2dDocumentStringOutputStream m_strstream;
};


#endif // wxART2D_USE_XMLPARSE

#endif /* _WX_DOCVIEWXMLIO_H_ */

