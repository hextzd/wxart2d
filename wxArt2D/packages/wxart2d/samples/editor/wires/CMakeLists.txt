##################################################
# Name:        CMakeLists.txt
# Purpose:     Input file for CMake
# Author:      Erik van der Wal
# Licence:     wxWidgets licence
# RCS Id:     $Id: CMakeLists.txt,v 1.4 2008/06/29 21:14:26 titato Exp $
##################################################

PROJECT(wires)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_editor_wires ${WIN32GUI}
    wires.cpp wires.h
    elements.cpp elements.h
    ${wxWidgets_RC}
)

TARGET_LINK_LIBRARIES( sample_editor_wires ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES}  )

FILE( GLOB wires_BITMAPS "bitmaps/*.gif" )

FOREACH(file ${wires_BITMAPS})
    GET_FILENAME_COMPONENT(tgt ${file} NAME)
    SET(tgt "${wires_BINARY_DIR}/bitmaps/${tgt}")
    CONFIGURE_FILE(${file} ${tgt} COPYONLY)
ENDFOREACH(file)