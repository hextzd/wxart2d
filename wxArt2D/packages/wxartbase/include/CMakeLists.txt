                
# when compiling a module as dll, other modules include files are in WXUSINGDLL mode included
# the module itself it include files or included in MAKINGDLL mode
# thise setting decide it something is exported or not in that module.
IF( wxartbase_MAKE_SHARED )

    SET(ART2D_ARTBASE_IF "#if defined(A2DARTBASEMAKINGDLL) || defined(WXUSINGDLL)")
    SET(ART2D_ARTBASE_END "#endif //defined(A2DARTBASEMAKINGDLL) || defined(WXUSINGDLL)")

ENDIF( wxartbase_MAKE_SHARED )

CONFIGURE_FILE( ${wxartbase_SOURCE_DIR}/include/wxartbase.h.in ${wxartbase_BINARY_DIR}/include/wxartbase.h @ONLY IMMEDIATE)
CONFIGURE_FILE( ${wxartbase_SOURCE_DIR}/include/wxartbaseprivate.h ${wxartbase_BINARY_DIR}/include/wxartbaseprivate.h @ONLY IMMEDIATE)

SET( wxartbase_OPTION "// all module's options of wxartbase\n")

IF( WXART2D_USE_XMLPARSE)
    SET( wxartbase_OPTION "${wxartbase_OPTION} #define wxART2D_USE_XMLPARSE 1\n"  )
ELSE()
    SET( wxartbase_OPTION "${wxartbase_OPTION} #define wxART2D_USE_XMLPARSE 0\n"  )
ENDIF()

IF( WXART2D_USE_CVGIO )
    SET(wxartbase_OPTION "${wxartbase_OPTION} #define wxART2D_USE_CVGIO 1\n"  )
ELSE()
    SET(wxartbase_OPTION "${wxartbase_OPTION} #define wxART2D_USE_CVGIO 0\n"  )
ENDIF()

IF( wxDocview_USE_IRRXML )
    SET(wxartbase_OPTION "${wxartbase_OPTION} #define wxDOCVIEW_USE_IRRXML 1\n"  )
ELSE()
    SET(wxartbase_OPTION "${wxartbase_OPTION} #define wxDOCVIEW_USE_IRRXML 0\n"  )
ENDIF()


CONFIGURE_FILE( ${wxartbase_SOURCE_DIR}/include/wxartbasesetup.h.in ${wxartbase_BINARY_DIR}/include/wxartbasesetup.h @ONLY IMMEDIATE)
INSTALL( 
    FILES ${wxartbase_SOURCE_DIR}/include/wxartbaseprec.h
    DESTINATION lib/wxartbase/include/${wxWidgets_SELECTED_CONFIG} 
)

INSTALL( 
    FILES ${wxartbase_BINARY_DIR}/include/wxartbasesetup.h
    DESTINATION lib/wxartbase/include/${wxWidgets_SELECTED_CONFIG} 
)

INSTALL( 
    FILES ${wxartbase_BINARY_DIR}/include/wxartbase.h
    DESTINATION include/wxartbase-${wxartbase_MAJOR_VERSION}.${wxartbase_MINOR_VERSION}
)

