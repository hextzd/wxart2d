IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DCURVESMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

SET( curves_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/curves/meta.h
    ${wxart2d_SOURCE_DIR}/include/wx/curves/curve.h
    ${wxart2d_SOURCE_DIR}/include/wx/curves/curvegroup.h
    ${wxart2d_SOURCE_DIR}/include/wx/curves/marker.h
    ${wxart2d_SOURCE_DIR}/include/wx/curves/zoomcur.h
    ${wxart2d_SOURCE_DIR}/include/wx/curves/axis.h
    ${wxart2d_SOURCE_DIR}/include/wx/curves/plotbox.h
)

A2D_PCH()

ADD_LIBRARY_WXLIKE( wxart2d curves
    ${a2d_PCH_SOURCE_FILE}
    plotbox.cpp
    curve.cpp
    curvegroup.cpp
    marker.cpp
    zoomcur.cpp
    axis.cpp
    ${curves_HEADERS}
)

#TARGET_LINK_LIBRARIES( curvus wxartbase_msw29ud_editor )
#TARGET_LINK_LIBRARIES( curves editor )

INSTALL( 
    FILES ${curves_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/curves
)
INSTALL( TARGETS curves DESTINATION lib )
