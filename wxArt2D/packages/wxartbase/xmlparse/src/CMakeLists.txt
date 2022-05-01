IF( WXDOCVIEW_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DGENERALMAKINGDLL )
ENDIF( WXDOCVIEW_MAKE_SHARED )

SET( xmlparse_HEADERS
        ${wxartbase_SOURCE_DIR}/include/wx/xmlparse/genprop.h
        ${wxartbase_SOURCE_DIR}/include/wx/xmlparse/genxmlpars.h
        ${wxartbase_SOURCE_DIR}/include/wx/xmlparse/xmlencode.h
)
SET( xmlparse_SOURCE
    genprop.cpp
    genxmlpars.cpp
    xmlencode.cpp
)

ADD_LIBRARY_WXLIKE( wxartbase xmlparse
    ${xmlparse_SOURCE}
    ${xmlparse_HEADERS}
)

GENERATE_POT( ${wxdocview_SOURCE_DIR}/share/locale xmlparse )

IF( WXDOCVIEW_MAKE_SHARED )
    TARGET_LINK_LIBRARIES( xmlparse "${EXPAT_LIBRARIES}" )
ENDIF( WXDOCVIEW_MAKE_SHARED )

INSTALL( 
    FILES ${xmlparse_HEADERS}
    DESTINATION include/wxartbase-${wxartbase_MAJOR_VERSION}.${wxartbase_MINOR_VERSION}/wx/xmlparse
)

INSTALL( TARGETS xmlparse DESTINATION lib )

