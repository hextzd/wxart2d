#
# Build the documentation
#

OPTION(BUILD_DOCUMENTATION "Build the documentation (Doxygen)." OFF)
#MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

IF (BUILD_DOCUMENTATION)
    INCLUDE (${CMAKE_ROOT}/Modules/FindDoxygen.cmake OPTIONAL)
     
    CONFIGURE_FILE( ${wxart2d_SOURCE_DIR}/doc/doxygen.in ${wxart2d_BINARY_DIR}/doc/doxygen.doxy @ONLY IMMEDIATE)
    CONFIGURE_FILE( ${wxart2d_SOURCE_DIR}/doc/wxart2d.css ${wxart2d_BINARY_DIR}/doc/wxart2d.css COPYONLY )
                   
    ADD_CUSTOM_TARGET(api-docs ALL ${DOXYGEN}
        ${wxart2d_BINARY_DIR}/doc/doxygen.doxy DEPENDS
        ${wxart2d_SOURCE_DIR}/doc/doxygen.in )  

    SET(doxygen_FILES  
        footer.html
        header.html
        gendoxy.bat
    )

    IF("${wxart2d_SOURCE_DIR}" MATCHES "^${wxart2d_BINARY_DIR}$" )
    ELSE("${wxart2d_SOURCE_DIR}" MATCHES "^${wxart2d_BINARY_DIR}$" )
        FOREACH(file ${doxygen_FILES})
            SET(src "${wxart2d_SOURCE_DIR}/doc/${file}")
            SET(tgt "${wxart2d_BINARY_DIR}/doc/${file}")
            CONFIGURE_FILE(${src} ${tgt} COPYONLY)
        ENDFOREACH(file)   
    ENDIF("${wxart2d_SOURCE_DIR}" MATCHES "^${wxart2d_BINARY_DIR}$" )

ENDIF (BUILD_DOCUMENTATION)

