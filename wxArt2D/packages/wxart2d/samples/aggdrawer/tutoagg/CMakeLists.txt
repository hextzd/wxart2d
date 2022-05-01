PROJECT(tutoagg)

COMPILE_RC( "${wxWwidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_aggdrawer_tuto ${WIN32GUI} tutoagg.cpp tutoagg.h ${wxWidgets_RC} ) 

TARGET_LINK_LIBRARIES( sample_aggdrawer_tuto ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )

	SET(tutoagg_DRAWING_FILES
        arc.bmp
        pat36.bmp
        smaller.png
        transp.png
	)	

IF("${tutoagg_SOURCE_DIR}" MATCHES "^${tutoagg_BINARY_DIR}$" )
ELSE("${tutoagg_SOURCE_DIR}" MATCHES "^${tutoagg_BINARY_DIR}$" )

    FOREACH(file ${tutoagg_DRAWING_FILES})
        SET(src "${tutoagg_SOURCE_DIR}/${file}")
        SET(tgt "${tutoagg_BINARY_DIR}/${file}")
        CONFIGURE_FILE(${src} ${tgt} COPYONLY)
    ENDFOREACH(file)   

ENDIF("${tutoagg_SOURCE_DIR}" MATCHES "^${tutoagg_BINARY_DIR}$" )
