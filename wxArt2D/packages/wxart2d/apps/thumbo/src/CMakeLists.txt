
#A2D_PCH()

IF( WXART2D_MAKE_SHARED )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

IF( WXART2D_USE_LUA )   

    CONFIGURE_FILE( ${thumbo_SOURCE_DIR}/src/genwxbind.bat.in
                   ${thumbo_BINARY_DIR}/src/genwxbind.bat
                   @ONLY IMMEDIATE )

    CONFIGURE_FILE( ${thumbo_SOURCE_DIR}/src/genwxbind.sh.in
    ${thumbo_BINARY_DIR}/src/genwxbind.sh
        IMMEDIATE )			   
        
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
        execute_process( COMMAND cmake -E chdir ${thumbo_BINARY_DIR}/src chmod a+x genwxbind.sh )
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
        
    CONFIGURE_FILE( ${thumbo_SOURCE_DIR}/src/thumbo_rules.lua.in
                   ${thumbo_BINARY_DIR}/src/thumbo_rules.lua
                   ESCAPE_QUOTES IMMEDIATE )

    IF( WIN32 )
        ADD_CUSTOM_COMMAND( OUTPUT ${thumbo_SOURCE_DIR}/src/thumbowrap.cpp
        COMMAND ${thumbo_BINARY_DIR}/src/genwxbind.bat
        DEPENDS
            ${thumbo_SOURCE_DIR}/src/thumbowrap.i
        )
    ELSE( WIN32 )
        ADD_CUSTOM_COMMAND( OUTPUT ${thumbo_SOURCE_DIR}/src/thumbowrap.cpp
        COMMAND ${thumbo_BINARY_DIR}/src/genwxbind.sh
        DEPENDS
            ${thumbo_SOURCE_DIR}/src/thumbowrap.i
        )
    ENDIF( WIN32 ) 

    SET_SOURCE_FILES_PROPERTIES( ${thumbo_SOURCE_DIR}/src/thumbowrap.cpp PROPERTIES GENERATED 1)
    SET_SOURCE_FILES_PROPERTIES( ${thumbo_SOURCE_DIR}/src/thumbo_bind.cpp PROPERTIES GENERATED 1)
    SET_SOURCE_FILES_PROPERTIES( ${thumbo_SOURCE_DIR}/include/thumbowrap.h PROPERTIES GENERATED 1)
    
    SET( luabindThumbo
        ${thumbo_SOURCE_DIR}/src/thumbowrap.cpp
        ${thumbo_SOURCE_DIR}/include/thumbowrap.h
        ${thumbo_SOURCE_DIR}/src/thumbo_bind.cpp    
    )   
ELSE( WXART2D_USE_LUA )   
    SET( luabindThumbo "" )   
ENDIF( WXART2D_USE_LUA )   

COMPILE_RC( "${wxWidgets_RC}" wx.o )

INCLUDE_DIRECTORIES( ${thumbo_SOURCE_DIR}/include )
 
ADD_EXECUTABLE( thumbo  ${WIN32GUI} 
    ${a2d_PCH_SOURCE_FILE}
    thumboedit.cpp ../include/thumboedit.h
    docv.cpp ../include/docv.h 
    viewcon.cpp ../include/viewcon.h
    
    ${luabindThumbo}
    ${wxWidgets_RC}  
    )

TARGET_LINK_LIBRARIES( thumbo ${WXART2D_LIBRARIES_WXLIKE} ${WXART2D_LIBRARIES} )

#ADD_DEPENDENCIES( thumbo canvas editor generalt ${WXART2D_LIBRARIES_WXLIKE} )
