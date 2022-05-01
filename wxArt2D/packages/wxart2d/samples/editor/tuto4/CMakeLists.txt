PROJECT(tuto4)

COMPILE_RC( "${wxWwidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_editor_tuto4 ${WIN32GUI} tuto4.cpp tuto4.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_editor_tuto4 ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )

