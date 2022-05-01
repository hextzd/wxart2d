IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DGENERALMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

SET( genart_HEADERS
        ${wxart2d_SOURCE_DIR}/include/wx/genart/prfl.h
        ${wxart2d_SOURCE_DIR}/include/wx/genart/prflcnt.h
        ${wxart2d_SOURCE_DIR}/include/wx/genart/prflcnthstg.h
        ${wxart2d_SOURCE_DIR}/include/wx/genart/prflcntmean.h
        ${wxart2d_SOURCE_DIR}/include/wx/genart/prfltmr.h
        ${wxart2d_SOURCE_DIR}/include/wx/genart/prflenbl.h
        ${wxart2d_SOURCE_DIR}/include/wx/genart/imagergba.h
        ${wxart2d_SOURCE_DIR}/include/a2dprec.h
)

SET( genart_INLINES ${genart_INLINES}
)

A2D_PCH()

IF( MSVC )
    ADD_LIBRARY_WXLIKE( wxart2d genart
        ${a2d_PCH_SOURCE_FILE}
        prfl.cpp
        prflcnt.cpp
        prflcntHstg.cpp
        prflcntMean.cpp
        prfltmr.cpp
        imagergba.cpp
        ${genart_HEADERS}
        ${genart_INLINES}
    )

ELSE( MSVC )
    ADD_LIBRARY_WXLIKE( wxart2d genart
        prfl.cpp
        prflcnt.cpp
        prflcnthstg.cpp
        prflcntmean.cpp
        prfltmr.cpp
        imagergba.cpp
        ${genart_HEADERS}
    )
ENDIF( MSVC )

IF( WXART2D_MAKE_SHARED )
    #MESSAGE( "shared lib general")
    #LINK_LIBRARIES( ${wxWidgets_LIBRARIES} )
    TARGET_LINK_LIBRARIES( genart ${EXPAT_LIBRARIES} ${wxWidgets_LIBRARIES} )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${genart_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/genart
)
INSTALL( TARGETS genart DESTINATION lib )

