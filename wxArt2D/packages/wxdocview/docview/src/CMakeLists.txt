IF( WXDOCVIEW_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DDOCVIEWMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXDOCVIEW_MAKE_SHARED )

SET( mdocview_HEADERS
        ${wxdocview_SOURCE_DIR}/include/wx/docview/docmdiref.h
        ${wxdocview_SOURCE_DIR}/include/wx/docview/docviewref.h
        ${wxdocview_SOURCE_DIR}/include/wx/docview/doccom.h
        ${wxdocview_SOURCE_DIR}/include/wx/docview/xh_a2dmenudoc.h
)

SET( mdocview_SOURCE
        docmdiref.cpp
        docviewref.cpp
        doccom.cpp
        xh_a2dmenudoc.cpp
)

DOCVIEW_PCH()

ADD_LIBRARY_WXLIKE( wxdocview docview
        ${docview_PCH_SOURCE_FILE}
        ${mdocview_SOURCE}
        ${mdocview_HEADERS}
)

GENERATE_POT( ${wxdocview_SOURCE_DIR}/share/locale mdocview )

IF( WXDOCVIEW_MAKE_SHARED )
    #MESSAGE( "shared lib docview")
    IF( WIN32 )
        TARGET_LINK_LIBRARIES( docview xmlparse )
    ELSE( WIN32 )
        TARGET_LINK_LIBRARIES( docview generalt xmlparse )
    ENDIF( WIN32 )
ENDIF( WXDOCVIEW_MAKE_SHARED )

INSTALL( 
    FILES ${mdocview_HEADERS}
    DESTINATION include/wxdocview-${wxDocview_MAJOR_VERSION}.${wxDocview_MINOR_VERSION}/wx/docview
)

INSTALL( TARGETS docview DESTINATION lib )
