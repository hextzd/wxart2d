      
IF( WXART2D_USE_AGGDRAWER )
    IF( WXART2D_COMP_SAMP_AGGDRAWER )
    
    
        SET( sample_aggdrawer_folder
        )         
    
        if ( WXART2D_USE_EDITOR )
            add_subdirectory( tutoagg )
            add_subdirectory( directdrawagg )
            SET( sample_aggdrawer_folder sample_aggdrawer_directdraw sample_aggdrawer_tuto ${sample_aggdrawer_folder} )         
        endif( WXART2D_USE_EDITOR )
        
        set_target_properties( 
            ${sample_aggdrawer_folder}
            PROPERTIES FOLDER samples/wxart2d/aggdrawer )      
        
    ENDIF( WXART2D_COMP_SAMP_AGGDRAWER )
ENDIF( WXART2D_USE_AGGDRAWER )

