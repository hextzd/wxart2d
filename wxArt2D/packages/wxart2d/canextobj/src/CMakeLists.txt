IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DEDITORMAKINGDLL )
#    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )


SET( canextobj_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/canextobj/canext.h    
    ${wxart2d_SOURCE_DIR}/include/wx/canextobj/canobj3d.h  
    ${wxart2d_SOURCE_DIR}/include/wx/canextobj/rendimg.h 
    ${wxart2d_SOURCE_DIR}/include/wx/canextobj/imageioh.h 
    ${wxart2d_SOURCE_DIR}/include/wx/canextobj/canextmod.h
    ${wxart2d_SOURCE_DIR}/include/wx/canextobj/sttoolext.h
)

A2D_PCH()

ADD_LIBRARY_WXLIKE( wxart2d canextobj
    ${a2d_PCH_SOURCE_FILE}
    canext.cpp
    canobj3d.cpp
    rendimg.cpp 
    imageioh.cpp
    sttoolext.cpp
    
    ${canextobj_HEADERS}
)

INSTALL( 
    FILES ${canextobj_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/canextobj
)
INSTALL( TARGETS canextobj DESTINATION lib )
