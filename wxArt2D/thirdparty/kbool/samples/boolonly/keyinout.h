/*! \file kbool/samples/boolonly/keyinout.h
    \author Klaas Holwerda
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: keyinout.h,v 1.5 2009/02/06 21:33:03 titato Exp $
*/

#ifndef KEYINOUT_H
#define KEYINOUT_H

class kbBool_Engine;
#include "boolonly.h"

extern FILE* GetPolygonsFromBoolEngKEYStart( string filename );
extern void GetPolygonsFromBoolEngKEY( kbBool_Engine* booleng, FILE * file );
extern void GetPolygonsFromBoolEngKEYStop( FILE * file );

#include <list>
#include <fstream>

typedef std::istream a2dDocumentInputStream;
typedef std::ifstream a2dDocumentFileInputStream;

class a2dIOHandlerKeyIn
{

public:

    a2dIOHandlerKeyIn( kbBool_Engine* booleng );
    ~a2dIOHandlerKeyIn();

    char PeekNextC();

    void IncC();

    virtual bool Load( a2dDocumentInputStream& stream );

    virtual void InitializeLoad(  );

    string m_wantedStructure;

    std::list< string > m_structures;

protected:
    a2dDocumentInputStream* m_streami;

    bool GetCommand();

    bool NormalWord();
    bool QuotedString();
    bool BracedString();
    bool Comment();

    bool Blanks();

    bool Word();

    bool ReadItem( const string& type );

    bool SkipXYData();

    double ReadDouble();

    void ReadKey();
    bool ReadBgnlib();
    bool ReadBoundary();

    bool ReadElflags();
    bool ReadGdsHeader();
    bool ReadLayer();
    bool ReadLibrarySettings();
    bool Read( kbPoint& _Point );
    bool ReadStrans();
    bool ReadStructure();
    bool ReadElement();

    bool ReadPolygon();

    void SetFlags();

    //!last read record is stored
    bool m_back;

    //!keyword for record in KEY data
    string m_keyword;
    //!value for record in KEY data
    string m_value;

    //!type of the record in KEY data
    int m_recordtype;

    //!last read line in DATA
    long m_linenumber;

    char a;
    string m_error_mes;
    string m_b;

    //!size of the last read record
    unsigned int m_recordsize;


    double m_scale_in;

    double m_userunits_out;

    int m_objectFlags;

    int m_refMaxx; 
    int m_dx, m_dy;

    kbBool_Engine* m_booleng;

};

#endif //KEYINOUT_H
