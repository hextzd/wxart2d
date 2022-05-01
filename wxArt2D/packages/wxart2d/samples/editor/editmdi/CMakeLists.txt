PROJECT( editmdi )

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_editor_editmdi ${WIN32GUI}
        editmdi.h editmdi.cpp
        ${wxWidgets_RC} )

TARGET_LINK_LIBRARIES( sample_editor_editmdi ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )

