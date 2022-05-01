# \file myproject.cmake
# \author Klaas Holwerda
#
#     Copyright: 2008 (c) Klaas Holwerda
#

IF("${wxart2d_SOURCE_DIR}" MATCHES "^${wxart2d_BINARY_DIR}$" )
    message( STATUS "in tree build" )
ELSE("${wxart2d_SOURCE_DIR}" MATCHES "^${wxart2d_BINARY_DIR}$" )
    message( STATUS "myapp CMake file generation" )
    CONFIGURE_FILE(${wxart2d_SOURCE_DIR}/myapp/CMakeLists.txt.in
                   ${wxart2d_BINARY_DIR}/myapp/CMakeLists.txt
                   @ONLY )
                                  
    CONFIGURE_FILE(${wxart2d_SOURCE_DIR}/myapp/src/myfile1.cpp
               ${wxart2d_BINARY_DIR}/myapp/src/myfile1.cpp
               COPYONLY )

    CONFIGURE_FILE(${wxart2d_SOURCE_DIR}/myapp/include/myfile1.h
               ${wxart2d_BINARY_DIR}/myapp/include/myfile1.h
               COPYONLY )

    CONFIGURE_FILE(${wxart2d_SOURCE_DIR}/myapp/src/smile.xpm
               ${wxart2d_BINARY_DIR}/myapp/src/smile.xpm
               COPYONLY )

    CONFIGURE_FILE(${wxart2d_SOURCE_DIR}/share/wxart2d/FindwxArt2D.cmake
                   ${wxart2d_BINARY_DIR}/myapp/share/wxart2d/FindwxArt2D.cmake
                   @ONLY )

    CONFIGURE_FILE(${wxart2d_SOURCE_DIR}/myapp/src/mondrian.xpm
               ${wxart2d_BINARY_DIR}/myapp/src/mondrian.xpm
               COPYONLY )
                   
ENDIF("${wxart2d_SOURCE_DIR}" MATCHES "^${wxart2d_BINARY_DIR}$" )

