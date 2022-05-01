COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_canvas_boolop ${WIN32GUI} boolop.cpp boolop.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_canvas_boolop ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )




