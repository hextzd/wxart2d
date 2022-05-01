PROJECT( myOwnApp )

#modify this!!
SET( WXART2D_DIR ${wxart2d_BINARY_DIR} )

# additional are modified Find routines
SET ( CMAKE_MODULE_PATH "${wxart2d_BINARY_DIR}/share/wxart2d" )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

OPTION( myOwnApp_USE_DEBUG "Compiled for debug or not" ON )

# set/modify @wxart2d_BINARY_DIR@ to your debug and release build directories
# if you want to use a specific BUILD/INSTALL!!
IF ( myOwnApp_USE_DEBUG )
    SET( WXART2D_DIR "${wxart2d_BINARY_DIR}" )
ELSE( myOwnApp_USE_DEBUG )
    SET( WXART2D_DIR "${wxart2d_BINARY_DIR}" )
ENDIF( myOwnApp_USE_DEBUG )

# will be found, but is not yet compiled at this stage. QUIET prevents ERRORS for not found libs.
FIND_PACKAGE( wxArt2D QUIET )
IF( WXART2D_FOUND )
    INCLUDE(${WXART2D_USE_FILE})
ELSE( WXART2D_FOUND )
    MESSAGE( "WXART2D library was not found" )
ENDIF( WXART2D_FOUND )

INCLUDE_DIRECTORIES( ${myOwnApp_SOURCE_DIR}/include )

COMPILE_RC( "${WXWINDOWS_RC}" wx.o )

ADD_EXECUTABLE( myOwnAppname  ${WIN32GUI}
    src/myfile1.cpp include/myfile1.h
    ${WXWINDOWS_RC}
)

SET(myOwnApp_FILES  
    mondrian.xpm
    smile.xpm 
)
    
COPYFILES( ${myOwnApp_SOURCE_DIR}/src ${myOwnApp_BINARY_DIR}/src ${myOwnApp_FILES} )


IF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )

# when using this application outside wxart2d tree, this can be handy
#include_external_msproject( editor ${WXART2D_DIR}/editor/src/editor.vcproj )
#include_external_msproject( general ${wxDocview_PREFIX}/general/src/general.vcproj )
#include_external_msproject( xmlparse ${wxDocview_PREFIX}/xmlparse/src/xmlparse.vcproj )
#include_external_msproject( docview ${wxDocview_PREFIX}/docview/src/docview.vcproj )
#include_external_msproject( canvas ${WXART2D_DIR}/canvas/src/canvas.vcproj )
#include_external_msproject( gdsio ${WXART2D_DIR}/gdsio/src/gdsio.vcproj )
#include_external_msproject( keyio ${WXART2D_DIR}/keyio/src/keyio.vcproj )
#include_external_msproject( artbase ${WXART2D_DIR}/artbase/src/artbase.vcproj )
#include_external_msproject( aggdrawer ${WXART2D_DIR}/aggdrawer/src/aggdrawer.vcproj )
#include_external_msproject( gdiplusdrawer ${WXART2D_DIR}/gdiplusdrawer/src/gdiplusdrawer.vcproj )
#include_external_msproject( luawraps ${WXART2D_DIR}/luawraps/src/luawraps.vcproj )
#include_external_msproject( canextobj ${WXART2D_DIR}/canextobj/src/canextobj.vcproj )
#include_external_msproject( kbool ${WXART2D_DIR}/../../thirdparty/kbool/src/kbool.vcproj )
#include_external_msproject( svgio ${WXART2D_DIR}/svgio/src/svgio.vcproj )
#include_external_msproject( curves ${WXART2D_DIR}/curves/src/curves.vcproj )

    IF ( wxWidgets_USE_DEBUG )

        ADD_DEPENDENCIES( myOwnAppname
            xmlparse
            docview
            general
        )

    ENDIF ( wxWidgets_USE_DEBUG )

ADD_DEPENDENCIES( myOwnAppname
    canvas  
    curves 
    editor 
    canextobj  
    aggdrawer 
    artbase 
    gdsio 
    svgio  
)

ENDIF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" ) 
 

