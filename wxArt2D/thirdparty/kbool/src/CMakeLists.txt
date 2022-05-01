IF(WIN32)
    ADD_DEFINITIONS( -D_MSWVC_ )
ELSE(WIN32)
    ADD_DEFINITIONS( -D__UNIX__  )
ENDIF(WIN32)
 
SET( kbool_HEADERS
    ${kbool_SOURCE_DIR}/include/kbool/booleng.h
    ${kbool_SOURCE_DIR}/include/kbool/graph.h
    ${kbool_SOURCE_DIR}/include/kbool/graphlst.h
    ${kbool_SOURCE_DIR}/include/kbool/line.h
    ${kbool_SOURCE_DIR}/include/kbool/link.h
    ${kbool_SOURCE_DIR}/include/kbool/lpoint.h
    ${kbool_SOURCE_DIR}/include/kbool/node.h
    ${kbool_SOURCE_DIR}/include/kbool/record.h
    ${kbool_SOURCE_DIR}/include/kbool/scanbeam.h
    ${kbool_SOURCE_DIR}/include/kbool/_dl_itr.h
    ${kbool_SOURCE_DIR}/include/kbool/_lnk_itr.h
)
 
ADD_LIBRARY( kbool
    booleng.cpp
    graph.cpp
    graphlst.cpp
    line.cpp
    link.cpp
    lpoint.cpp
    node.cpp
    record.cpp
    scanbeam.cpp
    ${kbool_HEADERS}
)

SET_TARGET_PROPERTIES( kbool PROPERTIES OUTPUT_NAME kbool${PFDEBUG} VERSION 0.0.0 SOVERSION 0 )

INSTALL( FILES ${kbool_HEADERS} DESTINATION include/kbool )
INSTALL( FILES 
    ${kbool_SOURCE_DIR}/include/kbool/_dl_itr.cpp
    ${kbool_SOURCE_DIR}/include/kbool/_lnk_itr.cpp
DESTINATION include/kbool )
    
INSTALL( TARGETS kbool DESTINATION lib )

