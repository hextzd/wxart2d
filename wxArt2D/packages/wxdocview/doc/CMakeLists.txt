#
# Build the documentation
#

OPTION(BUILD_DOCUMENTATION "Build the documentation (Doxygen)." OFF)
#MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

IF (BUILD_DOCUMENTATION)
    INCLUDE (${CMAKE_ROOT}/Modules/FindDoxygen.cmake OPTIONAL)
     
    CONFIGURE_FILE( ${wxdocview_SOURCE_DIR}/doc/doxygen.in ${wxdocview_BINARY_DIR}/doc/doxygen.doxy @ONLY IMMEDIATE)
    CONFIGURE_FILE( ${wxdocview_SOURCE_DIR}/doc/wxdocview.css ${wxdocview_BINARY_DIR}/doc/wxdocview.css COPYONLY )
                   
    ADD_CUSTOM_TARGET(api-docs ALL ${DOXYGEN}
        ${wxdocview_BINARY_DIR}/doc/doxygen.doxy DEPENDS
        ${wxdocview_SOURCE_DIR}/doc/doxygen.in )  

    SET(doxygen_FILES  
        footer.html
        header.html
        gendoxy.bat
    )

    IF("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )
    ELSE("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )
        FOREACH(file ${doxygen_FILES})
            SET(src "${wxdocview_SOURCE_DIR}/doc/${file}")
            SET(tgt "${wxdocview_BINARY_DIR}/doc/${file}")
            CONFIGURE_FILE(${src} ${tgt} COPYONLY)
        ENDFOREACH(file)   
    ENDIF("${wxdocview_SOURCE_DIR}" MATCHES "^${wxdocview_BINARY_DIR}$" )

ENDIF (BUILD_DOCUMENTATION)

