IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DCANVASMAKINGDLL )
ENDIF( WXART2D_MAKE_SHARED )

SET( keyio_HEADERS
        ${wxart2d_SOURCE_DIR}/include/wx/keyio//keyio.h
)

ADD_LIBRARY_WXLIKE( wxart2d keyio
        keyio.cpp
        ${keyio_HEADERS}
)

INSTALL( 
    FILES ${keyio_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/keyio
)
INSTALL( TARGETS keyio DESTINATION lib )
