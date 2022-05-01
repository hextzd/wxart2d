/*! \file samples/boolonly/boolonly.cpp
    \brief boolonly demonstrates the use of the boolean algorithm 
    \author Klaas Holwerda
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: boolonly.cpp,v 1.8 2009/09/26 19:01:19 titato Exp $
*/


#include "boolonly.h"
#include "keyinout.h"

#include <list>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <sstream>


// Constructors
kbPoint::kbPoint()
{
    m_x = 0.0;
    m_y = 0.0;
}

kbPoint::kbPoint( double const X, double const Y )
{
    m_x = X;
    m_y = Y;
}

BOOL_OP FromString( string operation )
{
    if ( operation == "OR" )
        return BOOL_OR;
    if ( operation == "AND" )
        return BOOL_AND;
    if ( operation == "EXOR" )
        return BOOL_EXOR;
    if ( operation == "A_SUB_B" )
        return BOOL_A_SUB_B;
    if ( operation == "B_SUB_A" )
        return BOOL_B_SUB_A;
    return BOOL_NON; 
}

void ArmBoolEng( kbBool_Engine* booleng )
{
    // set some global vals to arm the boolean engine
    double DGRID = 1000;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.001;   // snap with in this range points to lines in the intersection routines
    // should always be > DGRID  a  MARGE >= 10*1/DGRID is oke
    // this is also used to remove small segments and to decide when
    // two segments are in line.
    double CORRECTIONFACTOR = 500.0;  // correct the polygons by this number
    double CORRECTIONABER   = 1.0;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 1.5;    // when will we round the correction shape to a circle
    double SMOOTHABER       = 10.0;   // accuracy when smoothing a polygon
    double MAXLINEMERGE     = 1000.0; // leave as is, segments of this length in smoothen


    // DGRID is only meant to make fractional parts of input data which
    // are doubles, part of the integers used in vertexes within the boolean algorithm.
    // Within the algorithm all input data is multiplied with DGRID

    // space for extra intersection inside the boolean algorithms
    // only change this if there are problems
    int GRID = 10000;

    booleng->SetMarge( MARGE );
    booleng->SetGrid( GRID );
    booleng->SetDGrid( DGRID );
    booleng->SetCorrectionFactor( CORRECTIONFACTOR );
    booleng->SetCorrectionAber( CORRECTIONABER );
    booleng->SetSmoothAber( SMOOTHABER );
    booleng->SetMaxlinemerge( MAXLINEMERGE );
    booleng->SetRoundfactor( ROUNDFACTOR );

}

void AddPolygonsToBoolEng2( kbBool_Engine* booleng )
{
    int x1 = 100;
    int x2 = 200;
    int y1 = 100;
    int y2 = 200;
    int pitch1 = 200;
    int numRowsAndCols = 120;
    int i, j;

    for ( i = 0; i < numRowsAndCols; i++ )
    {
        for ( j = 0; j < numRowsAndCols; j++ )
        {
            // foreach point in a polygon ...
            if ( booleng->StartPolygonAdd( GROUP_A ) )
            {
                // Counter-Clockwise
                booleng->AddPoint( x1, y1 );
                booleng->AddPoint( x2, y1 );
                booleng->AddPoint( x2, y2 );
                booleng->AddPoint( x1, y2 );

            }
            booleng->EndPolygonAdd();
            x1 += pitch1;
            x2 += pitch1;
        }
        x1 = 100;
        x2 = 200;

        y1 += pitch1;
        y2 += pitch1;

    }

    x1 = 150;
    x2 = 250;
    y1 = 150;
    y2 = 250;

    for ( i = 0; i < numRowsAndCols; i++ )
    {
        for ( int j = 0; j < numRowsAndCols; j++ )
        {
            // foreach point in a polygon ...
            if ( booleng->StartPolygonAdd( GROUP_B ) )
            {
                // Counter Clockwise
                booleng->AddPoint( x1, y1 );
                booleng->AddPoint( x2, y1 );
                booleng->AddPoint( x2, y2 );
                booleng->AddPoint( x1, y2 );

            }
            booleng->EndPolygonAdd();
            x1 += pitch1;
            x2 += pitch1;
        }
        x1 = 150;
        x2 = 250;

        y1 += pitch1;
        y2 += pitch1;

    }

}

void AddPolygonsToBoolEng( kbBool_Engine* booleng )
{
    // foreach point in a polygon ...
    if ( booleng->StartPolygonAdd( GROUP_A ) )
    {
        booleng->AddPoint( 28237.480000, 396.364000 );
        booleng->AddPoint( 28237.980000, 394.121000 );
        booleng->AddPoint( 28242.000000, 395.699000 );
        booleng->AddPoint( 28240.830000, 397.679000 );
    }
    booleng->EndPolygonAdd();

    // foreach point in a polygon ...
    if ( booleng->StartPolygonAdd( GROUP_B ) )
    {
        booleng->AddPoint( 28242.100000, 398.491000 );
        booleng->AddPoint( 28240.580000, 397.485000 );
        booleng->AddPoint( 28237.910000, 394.381000 );
    }
    booleng->EndPolygonAdd();

    if ( booleng->StartPolygonAdd( GROUP_B ) )
    {
        booleng->AddPoint( 28243.440000, 399.709000 );
        booleng->AddPoint( 28237.910000, 394.381000 );
        booleng->AddPoint( 28239.290000, 394.763000 );
    }
    booleng->EndPolygonAdd();
}

void AddPolygonsToBoolEng3( kbBool_Engine* booleng )
{
    // foreach point in a polygon ...
    if ( booleng->StartPolygonAdd( GROUP_A ) )
    {
        booleng->AddPoint( 100, 100 );
        booleng->AddPoint( -100, 100 );
        booleng->AddPoint( -100, -100 );
        booleng->AddPoint( 100, -100 );
    }
    booleng->EndPolygonAdd();

    // foreach point in a polygon ...
    if ( booleng->StartPolygonAdd( GROUP_B ) )
    {
        booleng->AddPoint( 50, 50 );
        booleng->AddPoint( -50, 50 );
        booleng->AddPoint( -50, -50 );
        booleng->AddPoint( 50, -50 );
        booleng->EndPolygonAdd();
    }
    booleng->EndPolygonAdd();
}

void AddPolygonsToBoolEng4( kbBool_Engine* booleng )
{
    // foreach point in a polygon ...
    if ( booleng->StartPolygonAdd( GROUP_A ) )
    {
        booleng->AddPoint( 0, 0 );
        booleng->AddPoint( 0, 1000 );
        booleng->AddPoint( 1000, 1000 );
        booleng->AddPoint( 1000, 0 );
    }
    booleng->EndPolygonAdd();
}

void AddPolygonsToBoolEngFromKey( kbBool_Engine* booleng, string filename, string structure )
{
    a2dDocumentFileInputStream store( filename.c_str(), ios_base::in | ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        return;
    }

    a2dIOHandlerKeyIn keyhin( booleng );
    keyhin.m_wantedStructure = structure;
    keyhin.Load( store ); 
}

void GetPolygonsFromBoolEng( kbBool_Engine* booleng )
{
    // foreach resultant polygon in the booleng ...
    while ( booleng->StartPolygonGet() )
    {
        // foreach point in the polygon
        while ( booleng->PolygonHasMorePoints() )
        {
            fprintf( stderr, "x = %f\t", booleng->GetPolygonXPoint() );
            fprintf( stderr, "y = %f\n", booleng->GetPolygonYPoint() );
        }
        booleng->EndPolygonGet();
    }
}

int TestStructure( kbBool_Engine* booleng, BOOL_OP operation, string filename, string structure, FILE* fileout )
{
    AddPolygonsToBoolEngFromKey( booleng, filename, structure );
    booleng->Do_Operation( operation );
    printf("\nresulting polygons %d\n", operation );

    if ( fileout )
        GetPolygonsFromBoolEngKEY( booleng, fileout );
	else
    	GetPolygonsFromBoolEng( booleng );
    return 0;
}

int TestStructures( kbBool_Engine* booleng, string filename, FILE* fileout )
{
    a2dDocumentFileInputStream store( filename.c_str(), ios_base::in | ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        return 0;
    }

    a2dIOHandlerKeyIn keyhin( booleng );
    keyhin.Load( store ); 

    std::list< string >::iterator iter;
    for ( iter = keyhin.m_structures.begin(); iter != keyhin.m_structures.end(); iter++ )
    {
        string structurein = *iter;
        printf( "\n********* %s **************************\n", structurein.c_str() );
        booleng->Write_Log( "structure:", structurein );

        TestStructure( booleng, BOOL_OR, filename, structurein, fileout );
        TestStructure( booleng, BOOL_AND, filename, structurein, fileout );
        TestStructure( booleng, BOOL_EXOR, filename, structurein, fileout );
        TestStructure( booleng, BOOL_A_SUB_B, filename, structurein, fileout );
        TestStructure( booleng, BOOL_B_SUB_A, filename, structurein, fileout );
    }
    return 0;
}

int automatedTest()
{
    kbBool_Engine* booleng = new kbBool_Engine();
    ArmBoolEng( booleng );
    booleng->SetLog( true );
    booleng->SetDebug( true );

    string filename = "C:/data/soft/kboolpack/kbooln/testfiles/bigtest2l.key";
    //string filename = "C:/data/soft/kboolpack/kbooln/testfiles/bigthree.key";
    string structure = "S";
    for ( int i=1 ; i < 11; i++ )
    {
        char structurein[200];
        strcpy( structurein, structure.c_str() );
        sprintf( structurein, "%s%d", structure.c_str(), i );

        printf( "\n********* %s **************************\n", structurein );
        booleng->Write_Log( "structure:", structurein );

        TestStructure( booleng, BOOL_OR, filename, structurein, NULL );
        TestStructure( booleng, BOOL_AND, filename, structurein, NULL );
        TestStructure( booleng, BOOL_EXOR, filename, structurein, NULL );
        TestStructure( booleng, BOOL_A_SUB_B, filename, structurein, NULL );
        TestStructure( booleng, BOOL_B_SUB_A, filename, structurein, NULL );
    }

    booleng->SetLog( false );
    delete booleng;

    return 0;
}


int interactiveTest()
    {
    kbBool_Engine* booleng = new kbBool_Engine();
        ArmBoolEng( booleng );
        booleng->SetLog( true );
        booleng->SetDebug( true );

    float correction;
    char b, a = '1';
    do
    {
        //ANY of the following lines
        //AddPolygonsToBoolEngFromKey( booleng, "C:/data/soft/kboolpack/kbooln/testfiles/bigtest.key", "S10" );
        //AddPolygonsToBoolEng2( booleng );
        AddPolygonsToBoolEng3( booleng );

        printf( "\n***********************************\n" );
        printf( "*** version: % s \n", booleng->GetVersion().c_str() );
        printf( "***********************************\n" );
        printf( "1: OR operation\n" );
        printf( "2: AND operation\n" );
        printf( "3: EXOR operation\n" );
        printf( "4: A subtract B\n" );
        printf( "5: B subtract A\n" );
        printf( "6: Correct each polygon with a factor\n" );
        printf( "7: Smoothen each polygon\n" );
        printf( "8: Make a ring around each polygon\n" );
        printf( "9: No operation\n" );
        printf( "0: Quit\n" );
        printf( "***********************************\n" );

        printf( "type a number and <return>" );
		scanf("%c", &a, 1); 

        if ( a != 10 )
            scanf("%c", &b, 1); 

        switch (a)
        {
            case ('0'):
            {
                GetPolygonsFromBoolEng( booleng );
                break;
            }
            case ('1'):
                            booleng->Do_Operation(BOOL_OR);
                break;
            case ('2'):
                            booleng->Do_Operation(BOOL_AND);
                break;
            case ('3'):
                            booleng->Do_Operation(BOOL_EXOR);
                break;
            case ('4'):
                            booleng->Do_Operation(BOOL_A_SUB_B);
                break;
            case ('5'):
                            booleng->Do_Operation(BOOL_B_SUB_A);
                break;
            case ('6'):
                            printf( "give correction factor ( eg. 100.0 or - 100.0 )<return>: ");
                scanf(" % f", &correction ); // correct the polygons by this number
                booleng->SetCorrectionFactor( correction );
                booleng->Do_Operation(BOOL_CORRECTION);
                break;
            case ('7'):
                            booleng->Do_Operation(BOOL_SMOOTHEN);
                break;
            case ('8'):
                            printf("give width of ring <return>: ");
                scanf(" % f", &correction );
                // create a ring of this size
                booleng->SetCorrectionFactor( fabs( correction / 2.0) );
                booleng->Do_Operation(BOOL_MAKERING);
                break;
            case ('9'):
                            break;
            default:
                break;

        }

        if (a != '0')
		{
            printf("\nresulting polygons\n" );

            GetPolygonsFromBoolEng( booleng );

            //OR USE THIS
            //GetPolygonsFromBoolEngKEY( booleng );
        }
    }
    while (a != '0');

    booleng->SetLog( false );
        delete booleng;

    return 0;
    }


int main (int argc, char *argv[])
{
    int count;
    if (argc < 5 && argc > 1)
    {
        for (count = 1; count < argc; count++)
	        printf("argv[%d] = %s\n", count, argv[count]);

        string filename = argv[1];
        string structurein;
        if ( argc == 3 || argc == 4 )
            structurein = argv[2];

        kbBool_Engine* booleng = new kbBool_Engine();
        ArmBoolEng( booleng );
        //booleng->SetLog( true );
        //booleng->SetDebug( true );

        printf( "\n********* %s **************************\n", structurein.c_str() );
        booleng->Write_Log( "structure:", structurein );

        std::ostringstream stringStream;
        stringStream << filename << "OutOld"<< structurein;
        string outfile = stringStream.str();
        FILE* fileout = GetPolygonsFromBoolEngKEYStart( outfile );

        if ( argc == 4 )
        {
            BOOL_OP operation = FromString( argv[3] );
            TestStructure( booleng, operation, filename, structurein, fileout );
        }
        else if ( argc == 3 )
        {
            TestStructure( booleng, BOOL_OR, filename, structurein, fileout );
            TestStructure( booleng, BOOL_AND, filename, structurein, fileout );
            TestStructure( booleng, BOOL_EXOR, filename, structurein, fileout );
            TestStructure( booleng, BOOL_A_SUB_B, filename, structurein, fileout );
            TestStructure( booleng, BOOL_B_SUB_A, filename, structurein, fileout );
        }
        else
            TestStructures( booleng, filename, fileout );

        GetPolygonsFromBoolEngKEYStop( fileout );

        //booleng->SetLog( false );
        delete booleng;
        return 0;
    }
    else if (argc > 1)
    {
        printf("Usage: filename [structure] [operation] \n");
        printf("filename = Is KEY formatted file  \n");
        printf("structure = Specific structure in KEY formatted file, optional parameter, else all structures in file \n");
        printf("operation = one of this OR AND EXOR A_SUB_B B_SUB_A if not specified all opration are done\n");
        printf("No arguments is interactive mode\n");
        return 0;
    }

    interactiveTest();
}


