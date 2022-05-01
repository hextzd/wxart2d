IF( WXART2D_MAKE_SHARED )
    SET(BUILD_SHARED_LIBS = 1)
    ADD_DEFINITIONS ( -DA2DGDIPLUSDRAWERMAKINGDLL )
ENDIF( WXART2D_MAKE_SHARED )

SET( gdiplusdrawer_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/gdiplusdrawer/gdiplusdrawer.h
)

ADD_LIBRARY_WXLIKE( wxart2d gdiplusdrawer
    gdiplusdrawer.cpp
    ${gdiplusdrawer_HEADERS}
)

IF( WXART2D_MAKE_SHARED )
    TARGET_LINK_LIBRARIES( gdiplusdrawer artbase gdiplus )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${gdiplusdrawer_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/gdiplusdrawer
)
INSTALL( TARGETS gdiplusdrawer DESTINATION lib )
