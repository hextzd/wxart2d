/*! \file apps/mars/object.h
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: object.h,v 1.7 2006/12/13 21:43:21 titato Exp $
*/

#ifndef __OBJECTH__
#define __OBJECTH__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"

//! property to hold a wxString type variable to be associated with a msObject
class msProperty : public a2dStringProperty
{
public:

    msProperty();

    msProperty( const a2dPropertyIdString* id, const wxString& value );

    msProperty( const msProperty* ori );

    msProperty( const msProperty& other, a2dObject::CloneOptions options );

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( msProperty )
};

//! Page class for a storage component
class msObject : public a2dCanvasObject
{
public:
    //! Constructor.
    msObject();

    //! Copy constructor
    msObject( const msObject& other, CloneOptions options, a2dRefMap* refs );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    msProperty* SetStringProperty( const wxString& name, const wxString& value );
    msProperty* GetStringProperty( const wxString& name );
    wxString GetStringPropertyValue( const wxString& name );
    void MergeProperties( msObject* from );

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( msObject )
};


/*
//Very minimal wrapper for a hash map.
#if 0
    #ifdef _MSC_VER
        #include <hash_map>
        // In case of compiler problems, please add more checks.
        #if _MSC_VER > 1300
            #define myhashmap stdext::hash_map
        #else
            #define myhashmap std::hash_map
        #endif
    #endif // _MSC_VER
    #ifdef __GNUG__
        #include <ext/hash_map>
        #define myhashmap __gnu_cxx::hash_map
    #endif // __GNUG__
#else
    template <class Key, class Type> class myhashmap
    {
    public:
        class KeyHash
        {
        public:
            unsigned long operator()( const Key& k ) const { return (unsigned long) k; }
            KeyHash& operator=(const KeyHash&) { return *this; }
        };
        class KeyEqual
        {
        public:
            bool operator()( const Key& a, const Key& b ) const { return (a == b); }
            KeyEqual& operator=(const KeyEqual&) { return *this; }
        };
        WX_DECLARE_HASH_MAP( Key, Type, KeyHash, KeyEqual, wxhash );
    public:
        Type& operator[](const Key& key) { return m_hash[ key ]; }
        void erase( Key& key ) { m_hash.erase( key ); }
        void clear() { m_hash.clear(); }
    protected:
        wxhash m_hash;
    };
#endif
*/

#endif // __OBJECTH__
