IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DCANVASMAKINGDLL )
ENDIF( WXART2D_MAKE_SHARED )

SET( gdsio_HEADERS
        ${wxart2d_SOURCE_DIR}/include/wx/gdsio/gdsio.h
        ${wxart2d_SOURCE_DIR}/include/wx/gdsio/gdserr.h
)

ADD_LIBRARY_WXLIKE( wxart2d gdsio
        gdsio.cpp  gdserr.cpp
        ${gdsio_HEADERS}
)

INSTALL( 
    FILES ${gdsio_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/gdsio
)
INSTALL( TARGETS gdsio DESTINATION lib )

