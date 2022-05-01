# \file myproject.cmake
# \author Klaas Holwerda
#
#     Copyright: 2008 (c) Klaas Holwerda
#

IF("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )
    message( STATUS "in tree build" )
ELSE("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )
    message( STATUS "myapp CMake file generation" )
    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/CMakeLists.txt.in
                   ${wxdocview_BINARY_DIR}/myapp/CMakeLists.txt
                   @ONLY )
                                  
    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/src/myfile1.cpp
               ${wxdocview_BINARY_DIR}/myapp/src/myfile1.cpp
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/include/myfile1.h
               ${wxdocview_BINARY_DIR}/myapp/include/myfile1.h
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/src/doc.cpp
               ${wxdocview_BINARY_DIR}/myapp/src/doc.cpp
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/include/doc.h
               ${wxdocview_BINARY_DIR}/myapp/include/doc.h
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/src/view.cpp
               ${wxdocview_BINARY_DIR}/myapp/src/view.cpp
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/include/view.h
               ${wxdocview_BINARY_DIR}/myapp/include/view.h
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/src/docdialog.cpp
               ${wxdocview_BINARY_DIR}/myapp/src/docdialog.cpp
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/include/docdialog.h
               ${wxdocview_BINARY_DIR}/myapp/include/docdialog.h
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/src/smile.xpm
               ${wxdocview_BINARY_DIR}/myapp/src/smile.xpm
               COPYONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/share/wxdocview/FindwxDocview.cmake
                   ${wxdocview_BINARY_DIR}/myapp/share/wxdocview/FindwxDocview.cmake
                   @ONLY )

    CONFIGURE_FILE(${wxdocview_SOURCE_DIR}/myapp/src/mondrian.xpm
               ${wxdocview_BINARY_DIR}/myapp/src/mondrian.xpm
               COPYONLY )
                   
ENDIF("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )

