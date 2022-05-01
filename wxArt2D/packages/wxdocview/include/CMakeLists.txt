#FILE( WRITE "${wxdocview_BINARY_DIR}/include/wxdocview.h" "// all headers of wxDocview" )
                
# when compiling a module as dll, other modules include files are in WXUSINGDLL mode included
# the module itself it include files or included in MAKINGDLL mode
# thise setting decide it something is exported or not in that module.
IF( wxDocview_MAKE_SHARED )

    SET(ART2D_DOCVIEW_IF "#if defined(A2DDOCVIEWMAKINGDLL) || defined(WXUSINGDLL)")
    SET(ART2D_DOCVIEW_END "#endif //defined(A2DDOCVIEWMAKINGDLL) || defined(WXUSINGDLL)")

ENDIF( wxDocview_MAKE_SHARED )

CONFIGURE_FILE( ${wxdocview_SOURCE_DIR}/include/wxdocview.h.in ${wxdocview_BINARY_DIR}/include/wxdocview.h @ONLY IMMEDIATE)
CONFIGURE_FILE( ${wxdocview_SOURCE_DIR}/include/docviewprivate.h ${wxdocview_BINARY_DIR}/include/docviewprivate.h @ONLY IMMEDIATE)

SET(WXDOCVIEW_OPTION "// all module's options of wxDocview\n")


IF(wxDocview_USE_DOCVIEW)
    SET(WXDOCVIEW_OPTION "${WXDOCVIEW_OPTION} #define wxART2D_USE_DOCVIEW 1\n"  )
ELSE(wxDocview_USE_DOCVIEW)
    SET(WXDOCVIEW_OPTION "${WXDOCVIEW_OPTION} #define wxART2D_USE_DOCVIEW 0\n"  )
ENDIF(wxDocview_USE_DOCVIEW)

CONFIGURE_FILE( ${wxdocview_SOURCE_DIR}/include/docviewsetup.h.in ${wxdocview_BINARY_DIR}/include/docviewsetup.h @ONLY IMMEDIATE)

INSTALL( 
    FILES ${wxdocview_SOURCE_DIR}/include/docviewprec.h
    DESTINATION lib/wxdocview/include/${wxWidgets_SELECTED_CONFIG} 
)

INSTALL( 
    FILES ${wxdocview_BINARY_DIR}/include/docviewsetup.h
    DESTINATION lib/wxdocview/include/${wxWidgets_SELECTED_CONFIG} 
)

INSTALL( 
    FILES ${wxdocview_BINARY_DIR}/include/wxdocview.h
    DESTINATION include/wxdocview-${wxDocview_MAJOR_VERSION}.${wxDocview_MINOR_VERSION}
)

INSTALL( 
    FILES ${wxdocview_SOURCE_DIR}/include/docviewprivate.h
    DESTINATION include/wxdocview-${wxDocview_MAJOR_VERSION}.${wxDocview_MINOR_VERSION}
)

IF( NOT WIN32 )

    #SET( wxDocview_FLAGS ${wxDocview_FLAGS} -include docviewprec.cpp -Winvalid-pch )    
    SET( compile_FLAGS ${wxDocview_FLAGS} ${wxDocview_INCLUDE_DIRS} ${wxDocview_LIBRARY_DIRS} ${wxDocview_LIBRARIES} )
    SEPARATE_ARGUMENTS( compile_FLAGS )
    
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${wxdocview_SOURCE_DIR}/modules/wx/general/docviewprec.cpp
        COMMAND ${CMAKE_CXX_COMPILER}
                ${compile_FLAGS}
                -x c++-header
                -o ${wxdocview_SOURCE_DIR}/modules/wx/general/docviewprec.cpp
                wxdocview.h
        DEPENDS wxdocview.h		
    )
    
    ADD_CUSTOM_TARGET( wxdocview_gch 
        DEPENDS ${wxdocview_SOURCE_DIR}/modules/wx/general/docviewprec.cpp
    )
ENDIF( NOT WIN32 )


 
