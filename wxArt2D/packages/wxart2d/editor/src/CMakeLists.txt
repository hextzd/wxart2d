IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DEDITORMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

SET( editor_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/editor/canedit.h
    ${wxart2d_SOURCE_DIR}/include/wx/editor/candocproc.h
    ${wxart2d_SOURCE_DIR}/include/wx/editor/canpropedit.h
    ${wxart2d_SOURCE_DIR}/include/wx/editor/editmod.h
    ${wxart2d_SOURCE_DIR}/include/wx/editor/orderdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/editor/candoc.h
    ${wxart2d_SOURCE_DIR}/include/wx/editor/cancom.h    
    ${wxart2d_SOURCE_DIR}/include/wx/editor/recurdoc.h        
    ${wxart2d_SOURCE_DIR}/include/wx/editor/xmlparsdoc.h    
    ${wxart2d_SOURCE_DIR}/include/wx/editor/strucdlgdoc.h    
    ${wxart2d_SOURCE_DIR}/include/wx/editor/edsttool.h    
    )

SET( editor_SOURCE    
    canedit.cpp
    orderdlg.cpp
    candocproc.cpp
    canpropedit.cpp
    candoc.cpp  
    cancom.cpp
    recurdoc.cpp      
    xmlparsdoc.cpp    
    strucdlgdoc.cpp    
    edsttool.cpp
)
    
A2D_PCH()

ADD_LIBRARY_WXLIKE( wxart2d editor
    ${a2d_PCH_SOURCE_FILE}
    ${editor_SOURCE}
    ${editor_HEADERS}
)

GENERATE_POT( ${wxart2d_SOURCE_DIR}/share/locale editor )

IF( WXART2D_MAKE_SHARED )
    IF( WIN32 )
        TARGET_LINK_LIBRARIES( editor canvas luawraps )
    ELSE( WIN32 )
        TARGET_LINK_LIBRARIES( editor canvas canextobj luawraps )
    ENDIF( WIN32 )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${editor_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/editor
)
INSTALL( TARGETS editor DESTINATION lib )
