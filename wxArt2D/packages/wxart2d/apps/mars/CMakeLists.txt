##################################################
# Name:        CMakeLists.txt
# Purpose:     Input file for CMake
# Author:      Erik van der Wal
# Licence:     wxWidgets licence   
# RCS Id:     $Id: CMakeLists.txt,v 1.14 2008/06/29 21:14:23 titato Exp $
##################################################

PROJECT(mars)

IF( WXART2D_MAKE_SHARED )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

COMPILE_RC( "${wxWidgets_RC}" wx.o )

INCLUDE_DIRECTORIES( 
    ${mars_SOURCE_DIR}/include
)

ADD_EXECUTABLE(mars ${WIN32GUI} 
    ./src/docframe.cpp ./include/docframe.h
    ./src/document.cpp ./include/document.h
    ./src/docv.cpp ./include/docv.h
    ./src/elements.cpp ./include/elements.h
    ./src/mastertool.cpp ./include/mastertool.h
    ./src/object.cpp ./include/object.h
    ./src/page.cpp ./include/page.h
    ./src/part.cpp ./include/part.h
    ./src/view.cpp ./include/view.h
    ${wxWidgets_RC}  
)

IF( WXART2D_MAKE_SHARED )
    TARGET_LINK_LIBRARIES( mars editor)
ELSE( WXART2D_MAKE_SHARED )
    TARGET_LINK_LIBRARIES( mars ${WXART2D_LIBRARIES_WXLIKE} ${WXART2D_LIBRARIES} )
ENDIF( WXART2D_MAKE_SHARED )

FILE( GLOB mars_BITMAPS "bitmaps/*.gif" )
 
CONFIGURE_FILE(${mars_SOURCE_DIR}/marslogo128.png ${mars_BINARY_DIR}/marslogo128.png COPYONLY)
FOREACH(file ${mars_BITMAPS})
    GET_FILENAME_COMPONENT(tgt ${file} NAME)
    SET(tgt "${mars_BINARY_DIR}/bitmaps/${tgt}")
    CONFIGURE_FILE(${file} ${tgt} COPYONLY)
ENDFOREACH(file)   