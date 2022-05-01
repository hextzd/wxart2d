PROJECT(directdraw)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_artbase_directdraw ${WIN32GUI} directdraw.cpp directdraw.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_artbase_directdraw ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )

