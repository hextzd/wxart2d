PROJECT(curtuto1)

IF( WXART2D_MAKE_SHARED )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_curves_curtuto1 ${WIN32GUI} curtuto1.cpp curtuto2.cpp curtuto1.h ${wxWidgets_RC} )

    SET(curtuto1_FONT_FILES
    )
      
IF("${curtuto1_SOURCE_DIR}" MATCHES "^${curtuto1_BINARY_DIR}$" )
ELSE("${curtuto1_SOURCE_DIR}" MATCHES "^${curtuto1_BINARY_DIR}$" )

    FOREACH(file ${curtuto1_FONT_FILES})
        SET(src "${curtuto1_SOURCE_DIR}/fonts/${file}")
        SET(tgt "${curtuto1_BINARY_DIR}/fonts/${file}")
        CONFIGURE_FILE(${src} ${tgt} COPYONLY)
    ENDFOREACH(file)

    CONFIGURE_FILE("${curtuto1_SOURCE_DIR}/curtuto1.xrc" "${curtuto1_BINARY_DIR}/curtuto1.xrc" COPYONLY)

ENDIF("${curtuto1_SOURCE_DIR}" MATCHES "^${curtuto1_BINARY_DIR}$" )

TARGET_LINK_LIBRARIES( sample_curves_curtuto1 ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )
