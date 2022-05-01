PROJECT(zoom)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_editor_zoom ${WIN32GUI} zoom.cpp zoom.h ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_editor_zoom ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )


SET(zoom_FILES
	image1.png
	image2.png
	smile.xpm
	mondrian.xpm
)	
COPYFILES( ${zoom_SOURCE_DIR} ${zoom_BINARY_DIR} "${zoom_FILES}" )





