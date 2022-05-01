IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DARTBASEMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

SET( artbase_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/drawer2d.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/dcdrawer.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/liner.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/afmatrix.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/bbox.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/stylebase.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/polyver.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/artglob.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/artmod.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/graphica.h
    ${wxart2d_SOURCE_DIR}/include/wx/artbase/ogldrawer2d.h
)

SET( artbase_SOURCE
    drawer2d.cpp
    dcdrawer.cpp
    liner.cpp
    afmatrix.cpp
    bbox.cpp
    stylebase.cpp
    polyver.cpp
    artglob.cpp
    graphica.cpp
    ogldrawer2d.cpp
)

IF( WIN32 )
    SET( artbase_SOURCE ${artbase_SOURCE} mswfont.cpp)
ELSE( WIN32 )
    IF( MINGW OR CYGWIN )
        SET( artbase_SOURCE ${artbase_SOURCE} mswfont.cpp)
    ENDIF( MINGW OR CYGWIN )
ENDIF( WIN32 )

A2D_PCH()

ADD_LIBRARY_WXLIKE( wxart2d artbase
    ${a2d_PCH_SOURCE_FILE}
    ${artbase_SOURCE}
    ${artbase_HEADERS}
 )

GENERATE_POT( ${wxart2d_SOURCE_DIR}/share/locale artbase )
 
IF( WXART2D_MAKE_SHARED )
    IF(WXART2D_USE_FREETYPE)
        SET (ARTBASE_LINK_LIBS ${ARTBASE_LINK_LIBS} freetype)
    ENDIF(WXART2D_USE_FREETYPE)
    TARGET_LINK_LIBRARIES( artbase ${ARTBASE_LINK_LIBS} )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${artbase_HEADERS}
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/artbase
)

INSTALL( TARGETS artbase DESTINATION lib )

