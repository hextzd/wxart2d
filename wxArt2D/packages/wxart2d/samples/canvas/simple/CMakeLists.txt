PROJECT(simple)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_canvas_simple ${WIN32GUI} simple.cpp simple.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_canvas_simple ${A2D_LIBRARY_LIST_WXLIKE}  ${WXART2D_LIBRARIES} )
