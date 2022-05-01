PROJECT(sample_aggdrawer_directdraw)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_aggdrawer_directdraw ${WIN32GUI} directdrawagg.cpp directdrawagg.h ${wxWidgets_RC} ) 

TARGET_LINK_LIBRARIES( sample_aggdrawer_directdraw ${A2D_LIBRARY_LIST_WXLIKE} ${WXART2D_LIBRARIES} )
#TARGET_LINK_LIBRARIES( sample_aggdrawer_directdraw wxartbase_msw29ud_xmlparse wxartbase_msw29ud_generalt )

	SET(sample_aggdrawer_directdraw_DRAWING_FILES
        pat36.bmp
        arc.bmp
        smaller.png
        tt0208m_.ttf
	)	

IF("${sample_aggdrawer_directdraw_SOURCE_DIR}" MATCHES "^${sample_aggdrawer_directdraw_BINARY_DIR}$" )
ELSE("${sample_aggdrawer_directdraw_SOURCE_DIR}" MATCHES "^${sample_aggdrawer_directdraw_BINARY_DIR}$" )

    FOREACH(file ${sample_aggdrawer_directdraw_DRAWING_FILES})
        SET(src "${sample_aggdrawer_directdraw_SOURCE_DIR}/${file}")
        SET(tgt "${sample_aggdrawer_directdraw_BINARY_DIR}/${file}")
        CONFIGURE_FILE(${src} ${tgt} COPYONLY)
    ENDFOREACH(file)   

ENDIF("${sample_aggdrawer_directdraw_SOURCE_DIR}" MATCHES "^${sample_aggdrawer_directdraw_BINARY_DIR}$" )