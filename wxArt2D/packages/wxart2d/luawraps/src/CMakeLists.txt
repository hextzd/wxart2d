IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DLUAWRAPMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )

SET( luawraps_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/luawraps/luawrap.h
    ${wxart2d_SOURCE_DIR}/include/wx/luawraps/luabind.h
    ${wxart2d_SOURCE_DIR}/include/wx/luawraps/wxledit.h
)

CONFIGURE_FILE( ${luawraps_SOURCE_DIR}/src/genwxbind.bat.in
               ${luawraps_BINARY_DIR}/src/genwxbind.bat
               @ONLY IMMEDIATE )

CONFIGURE_FILE( ${luawraps_SOURCE_DIR}/src/genwxbind.sh.in
               ${luawraps_BINARY_DIR}/src/genwxbind.sh
               IMMEDIATE )

IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
    execute_process( COMMAND cmake -E chdir ${luawraps_BINARY_DIR}/src chmod a+x genwxbind.sh )
ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )

CONFIGURE_FILE( ${luawraps_SOURCE_DIR}/src/luasetup.h.in
               ${wxart2d_BINARY_DIR}/include/luasetup.h
               IMMEDIATE )

CONFIGURE_FILE( ${luawraps_SOURCE_DIR}/src/luawraps_rules.lua.in
               ${luawraps_BINARY_DIR}/src/luawraps_rules.lua
               ESCAPE_QUOTES IMMEDIATE )

CONFIGURE_FILE( ${luawraps_SOURCE_DIR}/src/luawraps.lua
               ${luawraps_BINARY_DIR}/src/luawraps.lua
               IMMEDIATE )
               
CONFIGURE_FILE( ${luawraps_SOURCE_DIR}/src/wxart2d_datatypes.lua
               ${wxart2d_BINARY_DIR}/share/wxart2d/luawraps/src/wxart2d_datatypes.lua
               IMMEDIATE )
               
A2D_PCH()

SET_SOURCE_FILES_PROPERTIES( ${luawraps_SOURCE_DIR}/src/luabind.cpp PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES( ${luawraps_SOURCE_DIR}/src/wxart2d_bind.cpp PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES( ${wxart2d_SOURCE_DIR}/include/wx/luawraps/luabind.h PROPERTIES GENERATED 1)


ADD_LIBRARY_WXLIKE( wxart2d luawraps
    ${a2d_PCH_SOURCE_FILE}
    ${luawraps_SOURCE_DIR}/src/commands.cpp
    ${luawraps_SOURCE_DIR}/src/general.cpp
    ${luawraps_SOURCE_DIR}/src/xmlparse.cpp
    ${luawraps_SOURCE_DIR}/src/artbase.cpp
    ${luawraps_SOURCE_DIR}/src/docview.cpp
    ${luawraps_SOURCE_DIR}/src/canvas.cpp
    ${luawraps_SOURCE_DIR}/src/editor.cpp
    ${luawraps_SOURCE_DIR}/src/luabind.cpp
    ${luawraps_SOURCE_DIR}/src/wxart2d_bind.cpp
    ${luawraps_SOURCE_DIR}/src/wxledit.cpp
    ${luawraps_SOURCE_DIR}/src/luawrap.cpp
    ${luawraps_HEADERS}
)

IF( WXART2D_MAKE_SHARED )
    TARGET_LINK_LIBRARIES( luawraps editor canextobj canvas docview generalt )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${luawraps_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/luawraps
)
INSTALL( 
    FILES wxart2d_datatypes.lua
    DESTINATION share/wxart2d/luawraps/src
)
INSTALL( TARGETS luawraps DESTINATION lib )


IF( WIN32 )
    set( generatescript ${luawraps_BINARY_DIR}/src/genwxbind.bat )
ELSE( WIN32 )
    set( generatescript ${luawraps_BINARY_DIR}/src/genwxbind.sh )
ENDIF( WIN32 )

ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/general.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/general.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/general.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/commands.cpp 
                      
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/commands.cpp 
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/commands.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/xmlparse.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/xmlparse.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/xmlparse.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/artbase.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/artbase.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/artbase.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/docview.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/docview.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/docview.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/canvas.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/canvas.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/canvas.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/editor.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/editor.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
            ${luawraps_SOURCE_DIR}/src/editor.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/wxart2d_bind.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/wxart2d_bind.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/src/luabind.cpp
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/src/luabind.cpp
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
)
ADD_CUSTOM_COMMAND( 
    OUTPUT 
        ${luawraps_SOURCE_DIR}/include/luabind.h
        
    COMMAND cmake -E touch  ${luawraps_SOURCE_DIR}/include/luabind.h
    COMMAND ${generatescript}
    DEPENDS
            ${wxart2d_BINARY_DIR}/include/luasetup.h
            ${luawraps_SOURCE_DIR}/src/luabind.i
)


