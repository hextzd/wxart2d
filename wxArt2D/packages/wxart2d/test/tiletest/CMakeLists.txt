COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( test_canvas_tiletest ${WIN32GUI} tiletest.cpp tiletest.h ${wxWidgets_RC} ) 
TARGET_LINK_LIBRARIES( test_canvas_tiletest ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )



