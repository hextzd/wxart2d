PROJECT(vdrawm)

COMPILE_RC( "vdrawm.rc" vdrawm.o )

ADD_EXECUTABLE( sample_editor_vdrawm  ${WIN32GUI}
    ../common/screen.cpp ../common/screen.h
    vdrawm.cpp view.cpp vdrawm.h view.h
    vdrawm.rc
)

TARGET_LINK_LIBRARIES( sample_editor_vdrawm ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )

COPYFILESALL( ${vdrawm_SOURCE_DIR} ${vdrawm_BINARY_DIR} "ico" )
COPYFILESALL( ${vdrawm_SOURCE_DIR} ${vdrawm_BINARY_DIR} "xpm" )
