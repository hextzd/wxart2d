IF ( WXART2D_USE_EDITOR )

    IF( WXART2D_USE_LUA )   
        IF ( WXART2D_USE_TETO )
            ADD_SUBDIRECTORY( teto )
        ENDIF ( WXART2D_USE_TETO )

        IF ( EXISTS ${wxart2d_SOURCE_DIR}/apps/wxluaart2d )
            ADD_SUBDIRECTORY( wxluaart2d )
        ENDIF ( EXISTS ${wxart2d_SOURCE_DIR}/apps/wxluaart2d )

    ENDIF( WXART2D_USE_LUA )   

    IF ( EXISTS ${wxart2d_SOURCE_DIR}/apps/thumbo )
        ADD_SUBDIRECTORY( thumbo )
    ENDIF ( EXISTS ${wxart2d_SOURCE_DIR}/apps/thumbo )
    
    IF ( EXISTS ${wxart2d_SOURCE_DIR}/apps/mars )
        ADD_SUBDIRECTORY( mars )
    ENDIF ( EXISTS ${wxart2d_SOURCE_DIR}/apps/mars )

ENDIF( WXART2D_USE_EDITOR )


