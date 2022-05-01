IF( WXART2D_MAKE_SHARED )
    SET(BUILD_SHARED_LIBS = 1)
    ADD_DEFINITIONS ( -DA2DAGGDRAWERMAKINGDLL )
ENDIF( WXART2D_MAKE_SHARED )

SET( aggdrawer_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/aggdrawer/aggdrawer.h
    ${wxart2d_SOURCE_DIR}/include/wx/aggdrawer/graphicagg.h
)

ADD_LIBRARY_WXLIKE( wxart2d aggdrawer
    aggdrawer.cpp
    graphicagg.cpp
    ${aggdrawer_HEADERS}
)

IF( WXART2D_MAKE_SHARED )
    TARGET_LINK_LIBRARIES( aggdrawer artbase antigrain )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${aggdrawer_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/aggdrawer
)
INSTALL( TARGETS aggdrawer DESTINATION lib )

