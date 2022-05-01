// ---------------------------------------------------------------------------
// a2dError
// ---------------------------------------------------------------------------

typedef unsigned int a2dErrorWarningCode
typedef int a2dSignal
    
#include "wx/general/gen.h"
class a2dError
{
    a2dError( const wxString& errorIdName = wxEmptyString, const wxString &error = wxEmptyString, bool warning = false )
    wxString& GetErrorMessage()
    a2dErrorWarningCode GetErrorCode()
    void SetErrorMessage( const wxString& error )
    void SetErrorCode( a2dErrorWarningCode code )
    
    #define_object a2dError_NoError
    #define_object a2dError_CommandError
    #define_object a2dError_NotSpecified
    #define_object a2dError_CanceledFileDialog
    #define_object a2dError_FileCouldNotOpen
    #define_object a2dError_CouldNotCreateDocument
    #define_object a2dError_NoDocTemplateRef
    #define_object a2dError_DocumentsOpened
    #define_object a2dError_SetEnv
    #define_object a2dError_GetEnv
    #define_object a2dError_GetVar
    #define_object a2dError_SaveFile
    #define_object a2dError_LoadFile
    #define_object a2dError_ExportFile
    #define_object a2dError_ImportFile
    #define_object a2dError_IOHandler
    #define_object a2dError_SaveObject
    #define_object a2dError_LoadObject
    #define_object a2dError_FileHistory
    #define_object a2dError_ImportObject
    #define_object a2dError_XMLparse
    #define_object a2dError_LoadLayers
    #define_object a2dError_property
    #define_object a2dError_NoView
    #define_object a2dError_NoDocument
    #define_object a2dError_NoController
    #define_object a2dError_NoTool
    #define_object a2dError_LinkRef
    #define_object a2dError_NoWrapper
    #define_object a2dError_LinkPin
    #define_object a2dError_NoPinClass
    #define_object a2dError_CouldNotEvaluatePath
    #define_object a2dError_Canceled
    #define_object a2dError_CouldNotLoad
    #define_object a2dError_NoSelection
    #define_object a2dError_ToManyOpen
    
};

class a2dRefMap : public a2dObject
{

public:

    a2dRefMap();

    virtual void Initialize();

    // a2dHashMapIntToObject& GetObjectHashMap() { return m_objecthashmap; }

    // a2dHashMapCreateObject& GetObjectCreateHashMap() { return m_objectCreate; }

    // a2dResolveIdList& GetResolveIdList() { return m_toResolve; }

    // bool ResolveOrAdd( a2dSmrtPtr<a2dObject>* storedHere, const wxString& id );

    // bool ResolveOrAdd( a2dAutoZeroPtr<a2dObject>* storedHere, const wxString& id );

    bool ResolveOrAddLink( a2dObject* obj, const wxString& id );

    void ResolveInform( a2dObject* obj );

    void SetLastObjectLoadedId( const wxString& lastid );

    virtual bool LinkReferences( bool ignoreNonResolved = false );

    bool RemoveReference( const wxString& idToRemove );

    a2dError GetLastError() const;

    void SetLastError( a2dError error );
};
    
// ---------------------------------------------------------------------------
// a2dPropertyEditEvent
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dPropertyEditEvent : public wxEvent
{
    a2dPropertyEditEvent( a2dObject* object, a2dNamedPropertyList* properties = NULL )
    a2dPropertyEditEvent( const a2dPropertyEditEvent& other )
    a2dNamedPropertyList* GetPropertyList()
    bool GetEdited()
    void SetEdited( bool edited )
};

// ---------------------------------------------------------------------------
// a2dCommandProcessorEvent
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dCommandProcessorEvent : public wxEvent
{
    a2dCommandProcessorEvent( wxEventType type, a2dCommand* cmd )
    a2dCommandProcessorEvent( a2dCommand* cmd,const wxString& undoLabel, bool canUndo,const wxString& redoLabel, bool canRedo )
    a2dCommandProcessorEvent(const a2dCommandProcessorEvent & event)
    virtual wxEvent *Clone() const
    a2dCommand* GetCommand()
    wxString& GetUndoMenuLabel()
    wxString& GetRedoMenuLabel()
    bool CanUndo()
    bool CanRedo()
};


// ---------------------------------------------------------------------------
// a2dPoint2D
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"

#include "wx/geometry.h"

class %delete a2dPoint2D
{
    //a2dPoint2D()
    a2dPoint2D( wxDouble x=0, wxDouble y=0 )
    a2dPoint2D( const a2dPoint2D &pt )
    a2dPoint2D( const wxPoint &pt )

    //void GetFloor( wxInt32 *x , wxInt32 *y ) const
    //void GetRounded( wxInt32 *x , wxInt32 *y ) const
    wxDouble GetVectorLength() const
    wxDouble GetVectorAngle() const
    void SetVectorLength( wxDouble length )
    void SetVectorAngle( wxDouble degrees )
    //void SetPolarCoordinates( wxDouble angle, wxDouble length ) - no function body in wxWidgets
    //void Normalize() - no function body in wxWidgets
    wxDouble GetDistance( const a2dPoint2D &pt ) const
    wxDouble GetDistanceSquare( const a2dPoint2D &pt ) const
    wxDouble GetDotProduct( const a2dPoint2D &vec ) const
    wxDouble GetCrossProduct( const a2dPoint2D &vec ) const

    %rename X %member_func wxDouble m_x
    %rename Y %member_func wxDouble m_y

    %operator a2dPoint2D operator-()
    %operator a2dPoint2D& operator=(const a2dPoint2D& pt)
    %operator a2dPoint2D& operator+=(const a2dPoint2D& pt)
    %operator a2dPoint2D& operator-=(const a2dPoint2D& pt)
    %operator a2dPoint2D& operator*=(const a2dPoint2D& pt)
    //a2dPoint2D& operator*=(wxDouble n)
    //a2dPoint2D& operator*=(wxInt32 n)
    %operator a2dPoint2D& operator/=(const a2dPoint2D& pt)
    //a2dPoint2D& operator/=(wxDouble n)
    //a2dPoint2D& operator/=(wxInt32 n)
    %operator bool operator==(const a2dPoint2D& pt) const
    //bool operator!=(const a2dPoint2D& pt) const
};

enum a2dXmlSer_flag
{
    a2dXmlSer_attrib,
    a2dXmlSer_Content
};

enum a2dObject::CloneOptions
{
    clone_members,
    clone_properties,
    clone_childs,
    clone_setoriginal,
    clone_seteditcopy,
    clone_flat,
    clone_deep
};

class a2dDocumentStringOutputStream
{
};
class a2dDocumentInputStream
{
};
class a2dDocumentOutputStream
{
};
//%class a2dDocumentFileInputStream
//};
//%class a2dDocumentFileOutputStream
//};

// ---------------------------------------------------------------------------
// a2dPathList
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dPathList : public wxPathList
{
    a2dPathList()
    //~a2dPathList()
    wxString FindValidPath(const wxString& filename)
    wxString FindAbsoluteValidPath(const wxString& filename)
    wxString GetAsString()
    bool ExpandPath( wxString& pathToExpand )
};

// ---------------------------------------------------------------------------
// a2dPropertyId
// ---------------------------------------------------------------------------
#include "wx/general/id.h"

enum a2dPropertyId::SetFlags
{
    set_none,
    set_onlymember,
    set_remove_non_member,
    set_remove_property
};

enum a2dPropertyId::Flags
{
    flag_none,
    flag_temporary,
    flag_transfer,
    flag_onlymember,
    flag_onlymemberhint,
    flag_listfirst,
    flag_multiple,
    flag_clonedeep,
    flag_listonly,
    flag_norender,
    flag_userDefined
};

// ---------------------------------------------------------------------------
// a2dPropertyId
// ---------------------------------------------------------------------------

#include "wx/general/id.h"
class a2dPropertyId
{
    a2dPropertyId( const wxString& name, a2dPropertyId::Flags flags )
    virtual wxString GetName() const
    virtual const a2dPropertyId *GetMainID() const
    virtual const a2dPropertyId *GetSubID() const
    //virtual bool operator == ( const a2dPropertyId &other ) const
    virtual a2dNamedProperty *CreatePropertyFromString( const wxString &value ) const
    bool IsCloneDeep() const
    bool IsTemporary() const
    bool IsTransfer() const
    bool IsOnlyMember() const
    bool IsOnlyMemberHint() const
    bool IsListFirst() const
    bool IsListOnly() const
    bool CheckFlags( a2dPropertyId::Flags flags ) const
    bool CheckCollect( const a2dPropertyId *id, a2dPropertyId::Flags flags ) const
    void RemovePropertyFromObject( a2dObject *obj ) const
};

// ---------------------------------------------------------------------------
// a2dObject
// ---------------------------------------------------------------------------
#include "wx/general/gen.h"
#include "wx/uri.h"

wxLongLong GetUniqueSerializationId() const

class a2dObject : public wxObject
{
    // abstract a2dObject()
    // abstract a2dObject( const a2dObject &other )
    //virtual ~a2dObject()
    %operator void operator = ( const a2dObject &other )
    //virtual void Dump(wxSTD ostream& str)
    int GetOwnedBy()
    virtual a2dObject* SetNonDynamic()
    void CheckReferences()
    virtual bool IsTemporary_DontSave() const
    //virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dObjectList* towrite )
    //virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
    virtual bool AlwaysWriteSerializationId() const
    virtual bool LinkReference( a2dObject *other )
    virtual void ResolveInform(  )
    //void Walker( wxObject* parent, a2dWalkerIOHandler& handler )
    void SetCheck(bool check)
    bool GetCheck() const
    void SetRelease( bool value )
    bool GetRelease( ) const
    a2dNamedProperty* GetProperty( const a2dPropertyId* propertyId, a2dPropertyId::Flags flags = a2dPropertyId::flag_none ) const
    void SetProperty( a2dNamedProperty *propertyHolder, a2dPropertyId::SetFlags flags = a2dPropertyId::set_none )
    void SetProperty( const a2dNamedProperty& propertyHolder, a2dPropertyId::SetFlags flags = a2dPropertyId::set_none )
    void CollectProperties2( a2dNamedPropertyList *total, const a2dPropertyId *id, a2dPropertyId::Flags flags = a2dPropertyId::flag_none ) const
    virtual bool HasProperty( const a2dPropertyId* id, const wxString &stringvalue )
    virtual a2dObject* Clone( a2dObject::CloneOptions options ) const

    void TakeOverProperties( a2dObject* from, a2dPropertyIdList* listOfIds )
    //wxURI GetURI() const
    //void SetURI( const wxURI& uri )
    const a2dNamedPropertyList& GetPropertyList() const
    bool HasDynamicProperties() const
    virtual bool EditProperties( const a2dPropertyId *id, bool withUndo = true )
    bool RemoveProperty(const a2dPropertyId* id, bool all = true )
    void RemoveTemporaryProperties( )
    virtual a2dNamedProperty *FindProperty( const a2dPropertyId* id )
    virtual const a2dNamedProperty *FindProperty( const a2dPropertyId* id ) const
    
    virtual wxString GetName() const
    virtual void SetName(const wxString& name)
    
    // %member static const a2dPropertyIdString PROPID_Name
    // %member static const a2dPropertyIdVoidPtr PROPID_autozeroptrlist
    // %member a2dSignal sm_Edit_properties
};

// ---------------------------------------------------------------------------
// a2dNamedProperty
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dNamedProperty : public wxObject
{
    // abstract a2dNamedProperty()
    // abstract a2dNamedProperty( const a2dPropertyId* id )
    // abstract a2dNamedProperty( const a2dNamedProperty &other )
    void SetToObject ( a2dObject *obj, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none )
    void SetToObjectClone( a2dObject *obj, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    const a2dPropertyId* GetId() const
    wxString GetName() const
    virtual wxString GetString() const
    virtual double GetDouble() const
    virtual bool GetBool() const
    virtual wxInt16 GetInt16() const
    virtual wxUint16 GetUint16() const
    virtual wxInt32 GetInt32() const
    virtual wxUint32 GetUint32() const
    virtual void* GetVoidPtr() const
    virtual a2dObject* GetRefObject() const
    virtual a2dObject* GetRefObjectNA() const
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation()  const
    virtual bool GetVisible() const
    virtual bool GetCanRender() const
    double ParseDouble( unsigned int& position )
    void SkipCommaSpaces( unsigned int& position )
    void SkipSpaces( unsigned int& position )
    //void Walker( wxObject* parent, a2dWalkerIOHandler& handler )
    //virtual void DoWalker( wxObject *parent, a2dWalkerIOHandler& handler )
};


// ---------------------------------------------------------------------------
// a2dBoolProperty
// ---------------------------------------------------------------------------
class a2dPropertyIdBool : public a2dPropertyId
{
    a2dPropertyIdBool( const wxString& name, bool defaultvalue );
    void SetPropertyToObject( a2dObject *obj, bool value,  a2dPropertyId::SetFlags flags = a2dPropertyId::set_none ) const
    static a2dPropertyIdBool* GetDummy()    
};

#include "wx/general/gen.h"
class a2dBoolProperty : public a2dNamedProperty
{
    a2dBoolProperty()
    a2dBoolProperty( const a2dPropertyIdBool* id, bool deep = true )
    a2dBoolProperty(const a2dBoolProperty &other, a2dObject::CloneOptions options )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dBoolProperty *CreatePropertyFromString( const a2dPropertyIdBool* id, const wxString &value )
    void SetValue( bool value )
    bool GetValue() const
    virtual bool GetBool() const
    //bool* GetValuePtr()
    //bool& GetValueRef()
    //const bool* GetValuePtr() const
    //const bool& GetValueRef() const
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dColourProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdColour : public a2dPropertyId
{
    a2dPropertyIdColour( const wxString& name, wxColour defaultvalue );
    void SetPropertyToObject( a2dObject *obj, wxColour value,  a2dPropertyId::SetFlags flags = a2dPropertyId::set_none ) const
    static a2dPropertyIdColour* GetDummy()    
};

#include "wx/general/gen.h"
class a2dColourProperty : public a2dNamedProperty
{
    a2dColourProperty()
    a2dColourProperty( const a2dPropertyIdColour* id, const wxColour& colour )
    a2dColourProperty(const a2dColourProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dColourProperty *CreatePropertyFromString( const a2dPropertyIdColour* id, const wxString &value )
    void SetValue( const wxColour& colour )
    wxColour GetValue() const
    wxColour *GetValuePtr()
    wxColour &GetValueRef()
    void SetColour( const wxColour& colour )
    wxColour GetColour()
    wxColour& GetColourRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dMenuProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdMenu : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dMenuProperty : public a2dNamedProperty
{
    a2dMenuProperty()
    a2dMenuProperty( const a2dPropertyIdMenu* id, wxMenu* menu )
    a2dMenuProperty(const a2dMenuProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dMenuProperty *CreatePropertyFromString( const a2dPropertyIdMenu* id, const wxString &value )
    void SetValue( wxMenu* menu )
    wxMenu* GetValue() const
    wxMenu** GetValuePtr()
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dDateTimeProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdDateTime : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dDateTimeProperty : public a2dNamedProperty
{
    a2dDateTimeProperty()
    a2dDateTimeProperty( const a2dPropertyIdDateTime* id, const wxDateTime& datetime )
    a2dDateTimeProperty(const a2dDateTimeProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dDateTimeProperty *CreatePropertyFromString( const a2dPropertyIdDateTime* id, const wxString &value )
    void SetValue( const wxDateTime& datetimeObject )
    wxDateTime GetValue() const
    wxDateTime *GetValuePtr()
    wxDateTime &GetValueRef()
    void SetDateTime( const wxDateTime& datetimeObject )
    wxDateTime GetDateTime()
    wxDateTime& GetDateTimeRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dDoubleProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdDouble : public a2dPropertyId
{
    a2dPropertyIdDouble( const wxString& name, double defaultvalue );
    void SetPropertyToObject( a2dObject *obj, double value, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    static a2dPropertyIdDouble* GetDummy()    
};

#include "wx/general/gen.h"
class a2dDoubleProperty : public a2dNamedProperty
{
    a2dDoubleProperty()
    a2dDoubleProperty( const a2dPropertyIdDouble* id, double value )
    a2dDoubleProperty(const a2dDoubleProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dDoubleProperty *CreatePropertyFromString( const a2dPropertyIdDouble* id, const wxString &value )
    void SetValue( double value )
    double GetValue() const
    virtual double GetDouble() const
    double* GetValuePtr()
    //double& GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dFloatProperty
// ---------------------------------------------------------------------------

class  a2dPropertyIdFloat : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dFloatProperty : public a2dNamedProperty
{
    a2dFloatProperty()
    a2dFloatProperty( const a2dPropertyIdFloat* id, float value )
    a2dFloatProperty(const a2dFloatProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dFloatProperty *CreatePropertyFromString( const a2dPropertyIdFloat* id, const wxString &value )
    void SetValue( float value )
    float GetValue() const
    virtual float GetFloat() const
    float* GetValuePtr()
    //float& GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};


// ---------------------------------------------------------------------------
// a2dWindowProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdWindow : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dWindowProperty : public a2dNamedProperty
{
    a2dWindowProperty()
    a2dWindowProperty( const a2dPropertyIdWindow* id, wxWindow* window )
    a2dWindowProperty(const a2dWindowProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dWindowProperty *CreatePropertyFromString( const a2dPropertyIdWindow* id, const wxString &value )
    void SetValue( wxWindow* window )
    wxWindow* GetValue() const
    wxWindow** GetValuePtr()
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dUint32Property
// ---------------------------------------------------------------------------

class a2dPropertyIdUint32 : public a2dPropertyId
{
    a2dPropertyIdUint32( const wxString& name, wxUint32 defaultvalue );
    void SetPropertyToObject( a2dObject *obj, wxUint32 value, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    static a2dPropertyIdUint32* GetDummy()    
};

#include "wx/general/gen.h"
class a2dUint32Property : public a2dNamedProperty
{
    a2dUint32Property()
    a2dUint32Property( const a2dPropertyIdUint32* id, wxUint32 value )
    a2dUint32Property(const a2dUint32Property &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dUint32Property *CreatePropertyFromString( const a2dPropertyIdUint32* id, const wxString &value )
    void SetValue( wxUint32 value )
    wxUint32 GetValue() const
    virtual wxUint32 GetUint32() const
    wxUint32* GetValuePtr()
    //wxUint32& GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dPathListProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdPathList : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dPathListProperty : public a2dNamedProperty
{
    a2dPathListProperty()
    a2dPathListProperty( const a2dPropertyIdPathList* id, const a2dPathList& pathlist )
    a2dPathListProperty(const a2dPathListProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dPathListProperty *CreatePropertyFromString( const a2dPropertyIdPathList* id, const wxString &value )
    void SetValue( const a2dPathList& pathlistObject )
    //a2dPathList GetValue() const
    a2dPathList *GetValuePtr()
    a2dPathList &GetValueRef()
    void SetFileName( const a2dPathList& pathlistObject )
    //a2dPathList GetFileName()
    a2dPathList& GetFileNameRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dStringProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdString : public a2dPropertyId
{
    a2dPropertyIdString( const wxString& name, wxString defaultvalue );
    void SetPropertyToObject( a2dObject *obj, const wxString& value, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    static a2dPropertyIdString* GetDummy()    
};

#include "wx/general/gen.h"
class a2dStringProperty : public a2dNamedProperty
{
    a2dStringProperty()
    a2dStringProperty( const a2dPropertyIdString* id, const wxString& value )
    a2dStringProperty(const a2dStringProperty &other, a2dObject::CloneOptions options )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dStringProperty *CreatePropertyFromString( const a2dPropertyIdString* id, const wxString &value )
    void SetValue( const wxString& value )
    wxString& GetValue()
    const wxString& GetValue() const
    virtual wxString GetString() const
    wxString* GetValuePtr()
    virtual wxString StringRepresentation()  const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};


// ---------------------------------------------------------------------------
// a2dProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdRefObject : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dProperty : public a2dNamedProperty
{
    a2dProperty()
    a2dProperty( const a2dPropertyIdRefObject* id, a2dObject *value )
    a2dProperty(const a2dProperty &other, a2dObject::CloneOptions options )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dProperty *CreatePropertyFromString( const a2dPropertyIdRefObject* id, const wxString &value )
    void SetValue( a2dObject *value )
    virtual a2dObject* GetRefObject() const
    virtual a2dObject* GetRefObjectNA() const
    a2dObject *GetValue() const
    // a2dObjectPtr *GetValuePtr()
    // a2dObjectPtr &GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dInt16Property
// ---------------------------------------------------------------------------

class a2dPropertyIdInt16 : public a2dPropertyId
{
    a2dPropertyIdInt16( const wxString& name, wxInt16 defaultvalue );
    void SetPropertyToObject( a2dObject *obj, wxInt16 value, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    static a2dPropertyIdInt16* GetDummy()    
};

#include "wx/general/gen.h"
class a2dInt16Property : public a2dNamedProperty
{
    a2dInt16Property()
    a2dInt16Property( const a2dPropertyIdInt16* id, wxInt16 value )
    a2dInt16Property(const a2dInt16Property &other)
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dInt16Property *CreatePropertyFromString( const a2dPropertyIdInt16* id, const wxString &value )
    void SetValue( wxInt16 value )
    wxInt16 GetValue() const
    virtual wxInt16 GetInt16() const
    wxInt16* GetValuePtr()
    //wxInt16& GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dUint16Property
// ---------------------------------------------------------------------------

class a2dPropertyIdUint16 : public a2dPropertyId
{
    a2dPropertyIdUint16( const wxString& name, wxUint16 defaultvalue );
    void SetPropertyToObject( a2dObject *obj, wxUint16 value, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    static a2dPropertyIdUint16* GetDummy()    
};

#include "wx/general/gen.h"
class a2dUint16Property : public a2dNamedProperty
{
    a2dUint16Property()
    a2dUint16Property( const a2dPropertyIdUint16* id, wxUint16 value )
    a2dUint16Property(const a2dUint16Property &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dUint16Property *CreatePropertyFromString( const a2dPropertyIdUint16* id, const wxString &value )
    void SetValue( wxUint16 value )
    wxUint16 GetValue() const
    virtual wxUint16 GetUint16() const
    wxUint16* GetValuePtr()
    //wxUint16& GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dTipWindowProperty
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dTipWindowProperty : public a2dWindowProperty
{
    a2dTipWindowProperty()
    // a2dTipWindowProperty( const a2dPropertyIdWindow* id, wxTipWindow* window, const wxString& tipstring, bool useObjRect = true )
    // a2dTipWindowProperty( const a2dPropertyIdWindow* id, wxTipWindow* window, const wxString& tipstring, const wxRect& rect )
    a2dTipWindowProperty(const a2dTipWindowProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dTipWindowProperty *CreatePropertyFromString( const a2dPropertyIdWindow* id, const wxString &value )
    void SetString( const wxString& tipstring )
    const wxString& GetString()
    const wxRect& GetRect() const
    bool GetUseObjRect() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dInt32Property
// ---------------------------------------------------------------------------

class a2dPropertyIdInt32 : public a2dPropertyId
{
    a2dPropertyIdInt32( const wxString& name, wxInt32 defaultvalue );
    void SetPropertyToObject( a2dObject *obj, wxInt32 value, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const
    static a2dPropertyIdInt32* GetDummy()    
};

#include "wx/general/gen.h"
class a2dInt32Property : public a2dNamedProperty
{
    a2dInt32Property()
    a2dInt32Property( const a2dPropertyIdInt32* id, wxInt32 value )
    a2dInt32Property(const a2dInt32Property &other)
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dInt32Property *CreatePropertyFromString( const a2dPropertyIdInt32* id, const wxString &value )
    void SetValue( wxInt32 value )
    wxInt32 GetValue() const
    virtual wxInt32 GetInt32() const
    wxInt32* GetValuePtr()
    //wxInt32& GetValueRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dVoidPtrProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdVoidPtr : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dVoidPtrProperty : public a2dNamedProperty
{
    a2dVoidPtrProperty()
    a2dVoidPtrProperty( const a2dPropertyIdVoidPtr* id, void* value )
    a2dVoidPtrProperty(const a2dVoidPtrProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dVoidPtrProperty *CreatePropertyFromString( const a2dPropertyIdVoidPtr* id, const wxString &value )
    void SetValue( void* value )
    void* GetValue() const
    virtual void* GetVoidPtr() const
    void** GetValuePtr()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dFileNameProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdFileName : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dFileNameProperty : public a2dNamedProperty
{
    a2dFileNameProperty()
    a2dFileNameProperty( const a2dPropertyIdFileName* id, const wxFileName& filename )
    a2dFileNameProperty(const a2dFileNameProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dFileNameProperty *CreatePropertyFromString( const a2dPropertyIdFileName* id, const wxString &value )
    void SetValue( const wxFileName& filenameObject )
    wxFileName GetValue() const
    wxFileName *GetValuePtr()
    wxFileName &GetValueRef()
    void SetFileName( const wxFileName& filenameObject )
    wxFileName GetFileName()
    wxFileName& GetFileNameRef()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dObjectProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdObject : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dObjectProperty : public a2dNamedProperty
{
    a2dObjectProperty()
    a2dObjectProperty( const a2dPropertyIdObject* id, const wxObject& value )
    a2dObjectProperty( const a2dObjectProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    // static a2dObjectProperty *CreatePropertyFromString( const a2dPropertyIdObject* id, const wxString &value )
    void SetValue( const wxObject& value )
    const wxObject& GetValue() const
};

// ---------------------------------------------------------------------------
// a2dArrayStringProperty
// ---------------------------------------------------------------------------

class a2dPropertyIdArrayString : public a2dPropertyId
{
};

#include "wx/general/gen.h"
class a2dArrayStringProperty : public a2dNamedProperty
{
    a2dArrayStringProperty()
    a2dArrayStringProperty( const a2dPropertyIdArrayString* id, const wxArrayString& value )
    a2dArrayStringProperty( const a2dPropertyIdArrayString* id, const wxString& value )
    a2dArrayStringProperty( const a2dArrayStringProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dArrayStringProperty *CreatePropertyFromString( const a2dPropertyIdArrayString* id, const wxString &value )
    void Add( const wxString& value )
    void SetValue( const wxArrayString& value )
    wxArrayString& GetValue()
    const wxArrayString& GetValue() const
    wxArrayString* GetValuePtr()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};



// ---------------------------------------------------------------------------
// a2dObjectList
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dObjectList
{
};

// ---------------------------------------------------------------------------
// a2dNamedPropertyList
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dNamedPropertyList
{
    a2dNamedPropertyList()
    a2dNamedPropertyList( const a2dNamedPropertyList &other )
    bool Remove( const a2dPropertyId* id, bool all = true )
    void RemoveTemporary()
    void Walker( wxObject* parent, a2dWalkerIOHandler& handler )
};

// ---------------------------------------------------------------------------
// a2dIOHandler
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dIOHandler : public a2dObject
{
    // a2dIOHandler()
    // ~a2dIOHandler()
    virtual void Initialize()
    wxObject* CreateObject( const wxString& symbolicName )
    //a2dHashMapIntToObject& GetObjectHashMap()
    ///a2dHashMapCreateObject& GetObjectCreateHashMap()
    //a2dResolveIdList& GetResolveIdList()
    //bool ResolveOrAdd( a2dSmrtPtr<a2dObject>* storedHere, const wxString &id = "" )
    //bool ResolveOrAdd( a2dAutoZeroPtr<a2dObject>* storedHere, const wxString &id = "" )
    //bool ResolveOrAddLink( a2dObject *obj, const wxString &id = "" )
    //void ResolveInform( a2dObject *obj )
    //void SetLastObjectLoadedId( const wxString& lastid )
    virtual bool LinkReferences()
    a2dError GetLastError() const
    void SetLastError( a2dError error )
};

// ---------------------------------------------------------------------------
// a2dIOHandlerStrIn
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dIOHandlerStrIn : public a2dIOHandler
{
    // a2dIOHandlerStrIn( wxMBConv& conv = wxConvUTF8 )
    virtual void InitializeLoad()
    virtual void ResetLoad()
    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL ) = 0
    virtual bool Load( a2dDocumentInputStream& stream , wxObject* obj ) = 0
};


// ---------------------------------------------------------------------------
// a2dIOHandlerStrOut
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dIOHandlerStrOut : public a2dIOHandler
{
    // a2dIOHandlerStrOut( wxMBConv& conv = wxConvUTF8 )
    virtual void InitializeSave()
    virtual void ResetSave()
    virtual bool CanSave( const wxObject* obj = NULL ) = 0
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* obj ) = 0
	void SetPrecision( int precision = -1 );
};
 
// ---------------------------------------------------------------------------
// a2dWalkerIOHandler
// ---------------------------------------------------------------------------

#include "wx/general/gen.h"
class a2dWalkerIOHandler : public a2dIOHandler
{
    // a2dWalkerIOHandler()
    // ~a2dWalkerIOHandler()
    virtual void Initialize()
    wxString GetErrorString()
    void SetStopWalking( bool stopWalking )
    bool GetStopWalking()
    void IncCurrentDepth()
    void DecCurrentDepth()
    int GetCurrentDepth()
    void SetDepthFirst( bool depthFirst )
    bool GetDepthFirst()
    void SetUseCheck( bool useCheck )
    bool GetUseCheck()
    //void Register( const wxString& classname, a2dObjectIOFn* IOfunc )
    //void Unregister( const wxString& classname )
    virtual bool WalkTask( wxObject* parent, wxObject* object, long event )    
    wxObject* GetParent()
};

// ---------------------------------------------------------------------------
// a2dCommandId
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dCommandId
{
    a2dCommandId( const wxString& commandName )
    virtual wxString GetIdClassName()
    wxString GetName() const
    static const a2dCommandId& GetCommandByName(const wxString& commandName)
    //static a2dHashMapCommandIds &GetHashMap()
};

// ---------------------------------------------------------------------------
// a2dCommand
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dCommand : public a2dObject
{
    %member static const a2dCommandId sm_noCommandId
    %member static const a2dCommandId sm_noCommandTypeId
    %member static const a2dCommandId sm_groupCommandId
    a2dCommand( bool canUndo  = false,const a2dCommandId& commandId = sm_noCommandId,const a2dCommandId& commandTypeId = sm_noCommandTypeId,const wxString& menuString = wxEmptyString
    virtual a2dObject* Clone( a2dObject::CloneOptions options = a2dObject::clone_deep ) const
    a2dCommand* TClone( a2dObject::CloneOptions options = a2dObject::clone_deep  )
    virtual wxString GetName() const
    const a2dCommandId* GetCommandId()
    const a2dCommandId* GetCommandTypeId()
    virtual a2dCommand *CloneAndBind( a2dObject *object )
    virtual bool Do() = 0
    virtual bool Undo() = 0
    virtual bool CanUndo() const
    virtual bool Redo()
    void SetCommandProcessor( a2dCommandProcessor* cmp )
    a2dCommandProcessor* GetCommandProcessor()
    virtual bool ClearAfterCurrentCommand( a2dCommand *current )
    virtual a2dCommandGroup *IsCommandGroup()
    virtual a2dCommand *FindPrevious( a2dCommand *current ) const
    virtual a2dCommand *FindNext( a2dCommand *current ) const
    virtual bool Remove( a2dCommand *command )
    virtual bool ContainsCommand( a2dCommand *command )
    virtual void DistributeEvent( wxEventType eventType )
};


// ---------------------------------------------------------------------------
// a2dComEvent
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dComEvent : public wxEvent
{
    %member const a2dSignal sm_changedProperty
    %member const a2dSignal sm_changedRefObject
    %member const a2dSignal sm_non
    a2dComEvent( wxObject* sender, a2dSignal id )
    a2dComEvent( a2dObject* sender, const a2dPropertyId* propId, a2dSignal id )
    a2dComEvent( wxObject* sender, a2dNamedProperty* property, a2dSignal id, bool ownProp = false )
    a2dComEvent( wxObject* sender, const a2dNamedProperty& property, a2dSignal id )
    a2dComEvent( wxObject* sender, a2dObject* refObject, a2dSignal id )
    // a2dComEvent( wxObject* sender, const a2dPropertyId* propId, a2dSignal id  )
    a2dComEvent( wxObject* sender, const a2dPropertyIdRefObject* propId, a2dObject* refObject, a2dSignal id  )
    a2dComEvent( wxObject* sender, const a2dPropertyIdBool* propId, bool propertyValue, a2dSignal id )
    a2dComEvent( wxObject* sender, const a2dPropertyIdUint16* propId, wxUint16 propertyValue, a2dSignal id  )
    a2dComEvent( wxObject* sender, const a2dPropertyIdInt32* propId, wxInt32 propertyValue, a2dSignal id  )
    a2dComEvent( wxObject* sender, const a2dPropertyIdDouble* propId, double propertyValue, a2dSignal id  )
    a2dComEvent( wxObject* sender, const a2dPropertyIdString* propId, const wxString&  propertyValue, a2dSignal id  )
    a2dComEvent()
    a2dComEvent( const a2dComEvent& other )
    wxEvent* Clone() const
    a2dNamedProperty* GetProperty()
    const wxEventType* GetEventComIdReturn()
};



// ---------------------------------------------------------------------------
// a2dCommandGroup
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dCommandGroup : public a2dCommand
{
    a2dCommandGroup( const wxString &name, a2dCommandGroup* parent = NULL )
    virtual a2dObject* Clone( a2dObject::CloneOptions options = a2dObject::clone_deep ) const
    a2dCommand* TClone( a2dObject::CloneOptions options = a2dObject::clone_deep  )
    virtual wxString GetGroupName() const
    bool ClearCommandsById( const a2dCommandId& commandId, a2dCommand *fromcommand = NULL )
    virtual bool ClearAfterCurrentCommand( a2dCommand *current )
    virtual a2dCommandGroup *IsCommandGroup()
    virtual a2dCommand *FindPrevious( a2dCommand *current ) const
    virtual a2dCommand *FindNext( a2dCommand *current ) const
    virtual bool Remove( a2dCommand *command )
    void Add( a2dCommand *command )
    virtual bool ContainsCommand( a2dCommand *command )
    a2dCommandGroup* GetParentGroup()
    void SetParentGroup( a2dCommandGroup* parent )
    virtual bool Do()
    virtual bool Undo()
};

// ---------------------------------------------------------------------------
// a2dCommandProcessor
// ---------------------------------------------------------------------------

#include "wx/general/comevt.h"
class a2dCommandProcessor : public a2dObject
{
    a2dCommandProcessor(int maxCommands = -1)
    virtual bool Submit(a2dCommand *command, bool storeIt = true)
    virtual void Store(a2dCommand *command)
    virtual bool Undo()
    virtual bool Redo()
    virtual bool CanUndo() const
    virtual bool CanRedo() const
    virtual void Initialize()
    virtual void SetMenuStrings()
    wxString GetUndoMenuLabel() const
    wxString GetRedoMenuLabel() const
    a2dCommand *GetCurrentCommand() const
    int GetMaxCommands() const
    a2dCommandGroup* GetRootGroup() const
    a2dCommandGroup* GetCurrentGroup() const
    virtual void ClearCommands()
    const wxString& GetUndoAccelerator() const
    const wxString& GetRedoAccelerator() const
    void SetUndoAccelerator(const wxString& accel)
    void SetRedoAccelerator(const wxString& accel)
    virtual bool SetOrAddPropertyToObject( a2dObject* propRefObject, const wxString& name, const wxString& value = "", bool withUndo = true )
    virtual bool SetOrAddPropertyToObject( a2dObject* propRefObject, a2dNamedProperty *property, bool withUndo = true )
    a2dCommandGroup *CommandGroupBegin( const wxString &name )
    void CommandGroupEnd( a2dCommandGroup *group )
};
