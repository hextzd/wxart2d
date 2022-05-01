PROJECT( artbase )

IF( WXART2D_COMP_SAMP_ARTBASE )
    add_subdirectory( directdraw )
    add_subdirectory( drawtest )
    
    set_target_properties( 
        sample_artbase_directdraw
        sample_artbase_drawtest
        PROPERTIES FOLDER samples/wxart2d/artbase )
    
ENDIF( WXART2D_COMP_SAMP_ARTBASE )

