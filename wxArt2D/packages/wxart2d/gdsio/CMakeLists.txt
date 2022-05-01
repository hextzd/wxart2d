PROJECT( gdsio )

IF( WXART2D_USE_GDSIO)
         
    IF( WXART2D_COMP_SAMP_GDSIO )
    #    add_subdirectory( src samples )
        add_subdirectory( src )
    ELSE( WXART2D_COMP_SAMP_GDSIO )
        add_subdirectory( src )
    ENDIF( WXART2D_COMP_SAMP_GDSIO )
    
ENDIF ( WXART2D_USE_GDSIO )
