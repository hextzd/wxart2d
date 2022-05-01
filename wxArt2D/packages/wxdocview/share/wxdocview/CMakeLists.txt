# \file wxdocviewshare project
# \author Erik van der Wal
#
#     Copyright: 2004 (c) Erik van der Wal
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.19 2009/06/23 18:56:07 titato Exp $

PROJECT( wxdocviewshare )

FOREACH( flag ${wxDocview_FLAGS} )
    SET( wxdocview_FLAGS "${wxDocview_FLAGS} ${flag}" )
ENDFOREACH( flag )
FOREACH( includedir ${wxDocview_INCLUDE_DIRS} )
    SET( wxdocview_INCLUDE_DIRS "${wxDocview_INCLUDE_DIRS} -I${includedir}-${wxDocview_MAJOR_VERSION}.${wxDocview_MINOR_VERSION}" )
ENDFOREACH( includedir )
FOREACH( linkdir ${wxdocview_LIBRARY_DIRS} )
    SET( wxdocview_LIBRARY_DIRS "${wxDocview_LIBRARY_DIRS} -L${linkdir}" )
ENDFOREACH( linkdir )

# first add libraries named in wxWidgets fashion, tuned to the specific wxWidgets used to compile
FOREACH( lib ${wxDocview_LIBRARIES_WXLIKE} )    
    SET( wxdocview_LIBRARIES "${wxdocview_LIBRARIES} -l${lib}" )
ENDFOREACH( lib )  

# now add full named
FOREACH( lib ${wxDocview_LIBRARIES} )
    SET( wxdocview_LIBRARIES "${wxdocview_LIBRARIES} -l${lib}" )
ENDFOREACH( lib )

# we like to use macros inside the user application too,
# this variable MACROS will be used in wxdocview-config.cmake.in
# FILE( READ ${wxdocview_SOURCE_DIR}/share/wxdocview/wxdocviewMacros.cmake MACROS )

CONFIGURE_FILE(${wxdocviewshare_SOURCE_DIR}/wxdocview-config.c.in
               ${wxdocviewshare_BINARY_DIR}/wxdocview-config.c
               ESCAPE_QUOTES @ONLY IMMEDIATE )              
               
CONFIGURE_FILE( ${wxdocviewshare_SOURCE_DIR}/wxDocviewConfig.cmake.in
                ${wxdocviewshare_BINARY_DIR}/wxDocviewConfig.cmake
                @ONLY IMMEDIATE )
                
CONFIGURE_FILE( ${wxdocviewshare_SOURCE_DIR}/UsewxDocview.cmake.in
                ${wxdocviewshare_BINARY_DIR}/UsewxDocview.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${wxdocviewshare_SOURCE_DIR}/FindwxDocview.cmake
                ${wxdocviewshare_BINARY_DIR}/FindwxDocview.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${wxdocviewshare_SOURCE_DIR}/wxdocviewMacros.cmake
                ${wxdocviewshare_BINARY_DIR}/wxdocviewMacros.cmake
                COPYONLY @ONLY IMMEDIATE )
                
# notice the rename here                
INSTALL( FILES ${wxdocviewshare_BINARY_DIR}/wxDocviewConfig.cmake DESTINATION "share/wxdocview" )
INSTALL( FILES ${wxdocviewshare_BINARY_DIR}/UsewxDocview.cmake DESTINATION "share/wxdocview" )              
INSTALL( FILES ${wxdocviewshare_BINARY_DIR}/FindwxDocview.cmake DESTINATION "share/wxdocview" )              
INSTALL( FILES ${wxdocviewshare_BINARY_DIR}/wxdocviewMacros.cmake DESTINATION "share/wxdocview" )              

# config file when not yet installed
ADD_EXECUTABLE( notwxdocview-config
    ${wxdocviewshare_BINARY_DIR}/wxdocview-config.c
    notinstalled.c
)

IF( UNIX )
    # config file when installed
    ADD_EXECUTABLE( wxdocview-config
        ${wxdocviewshare_BINARY_DIR}/wxdocview-config.c
        installed.c
    )
    IF( MINGW OR CYGWIN )
        SET ( CMAKE_EXECUTABLE_SUFFIX ".exe" )
    ENDIF( MINGW OR CYGWIN )
ELSE( UNIX )
    SET ( CMAKE_EXECUTABLE_SUFFIX ".exe" )
ENDIF( UNIX )

ADD_CUSTOM_COMMAND(
    TARGET notwxdocview-config
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy
        "${CMAKE_CFG_INTDIR}/notwxdocview-config${CMAKE_EXECUTABLE_SUFFIX}"
        "${wxdocview_BINARY_DIR}/wxdocview-config${CMAKE_EXECUTABLE_SUFFIX}"
)    

INSTALL( FILES "${wxdocview_BINARY_DIR}/wxdocview-config${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION "bin" )


