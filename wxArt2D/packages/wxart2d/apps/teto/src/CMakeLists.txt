
#A2D_PCH()

IF( WXART2D_MAKE_SHARED )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

CONFIGURE_FILE( ${teto_SOURCE_DIR}/src/genwxbind.bat.in
               ${teto_BINARY_DIR}/src/genwxbind.bat
               @ONLY IMMEDIATE )

CONFIGURE_FILE( ${teto_SOURCE_DIR}/src/genwxbind.sh.in
${teto_BINARY_DIR}/src/genwxbind.sh
    IMMEDIATE )			   
    
IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
    execute_process( COMMAND cmake -E chdir ${teto_BINARY_DIR}/src chmod a+x genwxbind.sh )
ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
    
CONFIGURE_FILE( ${teto_SOURCE_DIR}/src/teto_rules.lua.in
               ${teto_BINARY_DIR}/src/teto_rules.lua
               ESCAPE_QUOTES IMMEDIATE )

COMPILE_RC( "${wxWidgets_RC}" wx.o )

INCLUDE_DIRECTORIES( ${teto_SOURCE_DIR}/include )

IF( WIN32 )
    ADD_CUSTOM_COMMAND( OUTPUT ${teto_SOURCE_DIR}/src/tetowrap.cpp
    COMMAND ${teto_BINARY_DIR}/src/genwxbind.bat
    DEPENDS
        ${teto_SOURCE_DIR}/src/tetowrap.i
    )
ELSE( WIN32 )
    ADD_CUSTOM_COMMAND( OUTPUT ${teto_SOURCE_DIR}/src/tetowrap.cpp
    COMMAND ${teto_BINARY_DIR}/src/genwxbind.sh
    DEPENDS
        ${teto_SOURCE_DIR}/src/tetowrap.i
    )
ENDIF( WIN32 ) 

SET_SOURCE_FILES_PROPERTIES( ${teto_SOURCE_DIR}/src/tetowrap.cpp PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES( ${teto_SOURCE_DIR}/src/teto_bind.cpp PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES( ${teto_SOURCE_DIR}/include/tetowrap.h PROPERTIES GENERATED 1)
    
ADD_EXECUTABLE( teto  ${WIN32GUI} 
    ${a2d_PCH_SOURCE_FILE}
    tetostrucdlg.cpp ../include/tetostrucdlg.h
    tedit.cpp ../include/tedit.h
    docv.cpp ../include/docv.h 
    view.cpp ../include/view.h
    
    ${teto_SOURCE_DIR}/src/tetowrap.cpp
    ${teto_SOURCE_DIR}/include/tetowrap.h
    ${teto_SOURCE_DIR}/src/teto_bind.cpp
    ${wxWidgets_RC}  
    )

TARGET_LINK_LIBRARIES( teto ${WXART2D_LIBRARIES_WXLIKE} ${WXART2D_LIBRARIES} )
