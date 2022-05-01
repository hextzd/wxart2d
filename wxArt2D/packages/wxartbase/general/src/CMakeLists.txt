IF( WXDOCVIEW_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DGENERALMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXDOCVIEW_MAKE_SHARED )

SET( general_HEADERS
        ${wxartbase_SOURCE_DIR}/include/wx/general/comevt.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/gen.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/generaldef.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/smrtptr.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/id.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/a2dmemmgr.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/a2dlist.h
        ${wxartbase_SOURCE_DIR}/include/wx/general/genmod.h
)

SET( general_INLINES 
        ${wxartbase_SOURCE_DIR}/include/wx/general/smrtptr.inl
        ${wxartbase_SOURCE_DIR}/include/wx/general/id.inl
        ${wxartbase_SOURCE_DIR}/include/wx/general/a2dlist.inl
)

SET( general_SOURCE
        comevt.cpp
        a2dmemmgr.cpp
        gen.cpp
        id.cpp
)

GENERATE_POT( ${wxdocview_SOURCE_DIR}/share/locale general )

IF( MSVC )
    ADD_LIBRARY_WXLIKE( wxartbase generalt
        ${general_SOURCE}
        ${general_HEADERS}
        ${general_INLINES}
    )

ELSE( MSVC )
    ADD_LIBRARY_WXLIKE( wxartbase generalt
        ${general_SOURCE}
        ${general_HEADERS}
    )
ENDIF( MSVC )

IF( WXDOCVIEW_MAKE_SHARED )
    #MESSAGE( "shared lib general")
    #LINK_LIBRARIES( ${wxWidgets_LIBRARIES} )
    TARGET_LINK_LIBRARIES( generalt ${EXPAT_LIBRARIES} ${wxWidgets_LIBRARIES} )
ENDIF( WXDOCVIEW_MAKE_SHARED )

INSTALL( 
    FILES ${general_HEADERS} ${general_INLINES}
    DESTINATION include/wxartbase-${wxartbase_MAJOR_VERSION}.${wxartbase_MINOR_VERSION}/wx/general
)

INSTALL( TARGETS generalt DESTINATION lib )

