# drawing files
  SET(drawings_DRAWING_FILES
        unnamed1.drw
        unnamed2.drw
        unnamed23.drw
        unnamed24.drw
        unnamed3.drw
        unnamed3.drx
        unnamed4.drw
        unnamed4.drx
        unnamed5.drw
        unnamed5.drx
        unnamed50.drw
        unnamed6.drw
        unnamed6.drx
  )
  
IF("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )
ELSE("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )

    FOREACH(file ${drawings_DRAWING_FILES})
        SET(src "${wxdocview_SOURCE_DIR}/samples/drawings/${file}")
        SET(tgt "${wxdocview_BINARY_DIR}/samples/drawings/${file}")
        CONFIGURE_FILE(${src} ${tgt} COPYONLY)
    ENDFOREACH(file)   
  
ENDIF("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )
