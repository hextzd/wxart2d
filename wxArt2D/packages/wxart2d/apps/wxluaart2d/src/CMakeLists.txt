COMPILE_RC( "wxluart2d.rc" wxluart2d.o )

INCLUDE_DIRECTORIES( ${wxluaart2d_SOURCE_DIR}/src )
   
ADD_EXECUTABLE( wxlua_wxart2d  ${WIN32GUI} 
    ${a2d_PCH_SOURCE_FILE}
    lconsoleart2d.cpp lconsoleart2d.h
    editorart2d.h
    wxluaart2d.cpp wxluaart2d.h
    
    wxluaart2d.rc  
    )

TARGET_LINK_LIBRARIES( wxlua_wxart2d ${WXART2D_LIBRARIES_WXLIKE} ${WXART2D_LIBRARIES} )
