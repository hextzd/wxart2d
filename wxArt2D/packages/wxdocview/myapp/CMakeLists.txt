PROJECT( myOwnDocviewApp )

#modify this!!
SET( wxDocview_DIR ${wxdocview_BINARY_DIR} )

# additional are modified Find routines
SET ( CMAKE_MODULE_PATH "${wxdocview_BINARY_DIR}/share/wxdocview" )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

OPTION( myOwnDocviewApp_USE_DEBUG "Compiled for debug or not" ON )

# set/modify @wxdocview_BINARY_DIR@ to your debug and release build directories
# if you want to use a specific BUILD/INSTALL!!
IF ( myOwnDocviewApp_USE_DEBUG )
    SET( wxDocview_DIR "${wxdocview_BINARY_DIR}" )
ELSE( myOwnDocviewApp_USE_DEBUG )
    SET( wxDocview_DIR "${wxdocview_BINARY_DIR}" )
ENDIF( myOwnDocviewApp_USE_DEBUG )

# will be found, but is not yet compiled at this stage. QUIET prevents ERRORS for not found libs.
FIND_PACKAGE( wxDocview QUIET )
IF( WXDOCVIEW_FOUND )
    INCLUDE(${wxDocview_USE_FILE})         
    message( STATUS "wxWidgets_LIBRARIES for myapp: ${wxWidgets_LIBRARIES}" )
    message( STATUS "wxDocview_LIBRARIES for myapp: ${wxDocview_LIBRARIES}" )
ELSE( WXDOCVIEW_FOUND )
    MESSAGE( "wxDocview library was not found for myapp" )
ENDIF( WXDOCVIEW_FOUND )

INCLUDE_DIRECTORIES( ${myOwnDocviewApp_SOURCE_DIR}/include )

COMPILE_RC( "${WXWINDOWS_RC}" wx.o )

ADD_EXECUTABLE( myOwnDocviewAppname  ${WIN32GUI}
    include/view.h include/doc.h include/docdialog.h
    src/view.cpp src/doc.cpp src/docdialog.cpp
    src/myfile1.cpp include/myfile1.h

    ${WXWINDOWS_RC}
)

TARGET_LINK_LIBRARIES( myOwnDocviewAppname ${wxDocview_LIBRARY_WXLIKE} ${wxWidgets_LIBRARIES} )

SET(myOwnDocviewApp_FILES  
    mondrian.xpm
    smile.xpm 
)
    
COPYFILES( ${myOwnDocviewApp_SOURCE_DIR}/src ${myOwnDocviewApp_BINARY_DIR}/src ${myOwnDocviewApp_FILES} )


IF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )

# when using this application outside wxDocview tree, this can be handy
#include_external_msproject( general ${WXDOCVIEW_DIR}/general/src/general.vcproj )
#include_external_msproject( xmlparse ${WXDOCVIEW_DIR}/xmlparse/src/xmlparse.vcproj )
#include_external_msproject( docview ${WXDOCVIEW_DIR}/docview/src/docview.vcproj )

ADD_DEPENDENCIES( myOwnDocviewAppname
    xmlparse 
    docview  
    generalt 
)

ENDIF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" ) 
 

