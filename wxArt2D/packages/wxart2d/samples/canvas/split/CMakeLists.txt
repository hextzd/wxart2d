PROJECT( split )

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_canvas_split ${WIN32GUI} split.cpp split.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_canvas_split ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )
