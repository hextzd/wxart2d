PROJECT(single2)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_editor_single2 ${WIN32GUI} single2.cpp single2.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_editor_single2 ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )
