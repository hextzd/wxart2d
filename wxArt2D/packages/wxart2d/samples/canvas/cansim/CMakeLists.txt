PROJECT(cansim)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_canvas_cansim ${WIN32GUI} cansim.cpp cansim.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_canvas_cansim ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )





