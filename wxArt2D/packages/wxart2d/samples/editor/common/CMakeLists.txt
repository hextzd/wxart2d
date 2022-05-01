PROJECT(common)

# drawing files
  SET(common_DRAWING_FILES
        arc.svg
        bigtest.cal
        layers.cvg
        layersdefault.cvg
        layersdefault2.cvg
        lion10.svg
        m782506.cal
        m782506.cvg
        nestedrecs.cvg
        one.cal
        polygons.cvg
        reccir.cvg
        relative.svg
        screen10.svg
        screen6.svg
        tiger.svg
        transform.svg
        two.cal
        url.svg
        vpath.cvg
        vpath.svg
        vpath2.svg  
  )
   
    SET(common_FONT_FILES  
        nen.chr
    )

IF("${common_SOURCE_DIR}" MATCHES "^${common_BINARY_DIR}$" )
ELSE("${common_SOURCE_DIR}" MATCHES "^${common_BINARY_DIR}$" )

    FOREACH(file ${common_DRAWING_FILES})
        SET(src "${common_SOURCE_DIR}/drawings/${file}")
        SET(tgt "${common_BINARY_DIR}/drawings/${file}")
        CONFIGURE_FILE(${src} ${tgt} COPYONLY)
    ENDFOREACH(file)   
   
    FOREACH(file ${common_FONT_FILES})
        SET(src "${common_SOURCE_DIR}/fonts/${file}")
        SET(tgt "${common_BINARY_DIR}/fonts/${file}")
        CONFIGURE_FILE(${src} ${tgt} COPYONLY)
    ENDFOREACH(file)     
   
    COPYFILESALL( ${common_SOURCE_DIR}/images ${common_BINARY_DIR}/images "*"  )
  
ENDIF("${common_SOURCE_DIR}" MATCHES "^${common_BINARY_DIR}$" )
