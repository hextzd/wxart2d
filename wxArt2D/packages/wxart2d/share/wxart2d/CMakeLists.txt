# \file wxart2share project
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.19 2009/06/23 18:56:07 titato Exp $

PROJECT( wxart2share )

FOREACH( flag ${WXART2D_FLAGS} )
    SET( wxart2d_FLAGS "${wxart2d_FLAGS} ${flag}" )
ENDFOREACH( flag )
FOREACH( includedir ${WXART2D_INCLUDE_DIRS} )
    SET( wxart2d_INCLUDE_DIRS "${wxart2d_INCLUDE_DIRS} -I${includedir}" )
ENDFOREACH( includedir )
FOREACH( linkdir ${WXART2D_LIBRARY_DIRS} )
    SET( wxart2d_LIBRARY_DIRS "${wxart2d_LIBRARY_DIRS} -L${linkdir}" )
ENDFOREACH( linkdir )

# first add libraries named in wxWidgets fashion, tuned to the specific wxWidgets used to compile
FOREACH( lib ${WXART2D_LIBRARIES_WXLIKE} )     
    # be aware order is changed here ( e.g. genart as last to link )
    SET( wxart2d_LIBRARIES "-l${lib} ${wxart2d_LIBRARIES}" )
ENDFOREACH( lib ) 

# now add full named
FOREACH( lib ${WXART2D_LIBRARIES} )
    SET( wxart2d_LIBRARIES "${wxart2d_LIBRARIES} -l${lib}" )
ENDFOREACH( lib )

# we like to use macros inside the user application too,
# this variable MACROS will be used in wxart2d-config.cmake.in
# FILE( READ ${wxart2share_SOURCE_DIR}/Macros.cmake MACROS )

CONFIGURE_FILE(${wxart2share_SOURCE_DIR}/wxart2d-config.c.in
               ${wxart2share_BINARY_DIR}/wxart2d-config.c
               ESCAPE_QUOTES @ONLY IMMEDIATE )              
               
CONFIGURE_FILE( ${wxart2share_SOURCE_DIR}/wxArt2DConfig.cmake.in
                ${wxart2share_BINARY_DIR}/wxArt2DConfig.cmake
                @ONLY IMMEDIATE )
               
CONFIGURE_FILE( ${wxart2share_SOURCE_DIR}/UsewxArt2D.cmake.in
                ${wxart2share_BINARY_DIR}/UsewxArt2D.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${wxart2share_SOURCE_DIR}/FindwxArt2D.cmake
                ${wxart2share_BINARY_DIR}/FindwxArt2D.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${wxart2share_SOURCE_DIR}/wxart2dMacros.cmake
                ${wxart2share_BINARY_DIR}/wxart2dMacros.cmake
                COPYONLY @ONLY IMMEDIATE )
              
INSTALL( FILES ${wxart2share_BINARY_DIR}/wxArt2DConfig.cmake DESTINATION "share/wxart2d"  )
INSTALL( FILES ${wxart2share_BINARY_DIR}/UsewxArt2D.cmake DESTINATION "share/wxart2d" )              
INSTALL( FILES ${wxart2share_BINARY_DIR}/FindwxArt2D.cmake DESTINATION "share/wxart2d" )              
INSTALL( FILES ${wxart2share_BINARY_DIR}/wxart2dMacros.cmake DESTINATION "share/wxart2d" )              

# config file when not yet installed
ADD_EXECUTABLE( notwxart2d-config
    ${wxart2share_BINARY_DIR}/wxart2d-config.c
    notinstalled.c
)

IF( UNIX )
    # config file when installed
    ADD_EXECUTABLE( wxart2d-config
        ${wxart2share_BINARY_DIR}/wxart2d-config.c
        installed.c
    )
    IF( MINGW OR CYGWIN )
        SET ( CMAKE_EXECUTABLE_SUFFIX ".exe" )
    ENDIF( MINGW OR CYGWIN )
ELSE( UNIX )
    SET ( CMAKE_EXECUTABLE_SUFFIX ".exe" )
ENDIF( UNIX )

ADD_CUSTOM_COMMAND(
    TARGET notwxart2d-config
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy
        "${CMAKE_CFG_INTDIR}/notwxart2d-config${CMAKE_EXECUTABLE_SUFFIX}"
        "${wxart2d_BINARY_DIR}/wxart2d-config${CMAKE_EXECUTABLE_SUFFIX}"
)    

INSTALL( FILES "${wxart2d_BINARY_DIR}/wxart2d-config${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION "bin" )


