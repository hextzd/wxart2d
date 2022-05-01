PROJECT(vdraws)

COMPILE_RC( "vdraws.rc" vdraws.o )

ADD_EXECUTABLE( sample_editor_vdraws ${WIN32GUI}
 ../common/screen.cpp ../common/screen.h
 vdraws.cpp vdraws.h vdraws.rc )

TARGET_LINK_LIBRARIES( sample_editor_vdraws ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )

COPYFILESALL( ${vdraws_SOURCE_DIR} ${vdraws_BINARY_DIR} "ico" )
COPYFILESALL( ${vdraws_SOURCE_DIR} ${vdraws_BINARY_DIR} "xpm" )

#ADD_DEPENDENCIES( sample_editor_vdraws
#    xmlparse
#    docview
#    generalt
#)