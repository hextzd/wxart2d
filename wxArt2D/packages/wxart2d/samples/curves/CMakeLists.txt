PROJECT( curves )

IF( WXART2D_USE_CURVES )
     
    IF( WXART2D_COMP_SAMP_CURVES )
        if ( WXART2D_USE_EDITOR )
            add_subdirectory( curtuto1  )
            set_target_properties( 
                sample_curves_curtuto1
                PROPERTIES FOLDER samples/wxart2d/curves )            
        endif( WXART2D_USE_EDITOR )        
    ENDIF( WXART2D_COMP_SAMP_CURVES )

ENDIF( WXART2D_USE_CURVES )

