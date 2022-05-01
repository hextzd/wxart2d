##################################################
# Name:        CMakeLists.txt
# Purpose:     Input file for CMake
# Author:      Erik van der Wal
# Licence:     wxWidgets licence
# RCS Id:      $Id: CMakeLists.txt,v 1.4 2008/06/29 21:14:24 titato Exp $
##################################################

PROJECT( drawtest )

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_artbase_drawtest ${WIN32GUI} drawtest.cpp drawtest.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_artbase_drawtest ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )



