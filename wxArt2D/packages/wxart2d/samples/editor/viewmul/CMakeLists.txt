PROJECT(viewmul)

COMPILE_RC( "viewmul.rc" viewmul.o )

ADD_EXECUTABLE( sample_editor_viewmul ${WIN32GUI} doc.cpp docv.cpp viewmul.cpp doc.h docv.h viewmul.h viewmul.rc )

TARGET_LINK_LIBRARIES( sample_editor_viewmul ${A2D_LIBRARY_LIST_WXLIKE}  ${WXART2D_LIBRARIES} )

COPYFILESALL( ${viewmul_SOURCE_DIR} ${viewmul_BINARY_DIR} "ico" )
COPYFILESALL( ${viewmul_SOURCE_DIR} ${viewmul_BINARY_DIR} "xpm" )
