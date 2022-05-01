PROJECT( canvas )

SET( WXART2D_USE_CANVAS 1 )

IF( WXART2D_USE_CANVAS )
    
    IF( WXART2D_COMP_SAMP_CANVAS )
        SET( sample_canvas_folder
            sample_canvas_boolop
            sample_canvas_cansim
            sample_canvas_simple
            sample_canvas_split
        )         

        add_subdirectory( simple  )
        add_subdirectory( split )
        add_subdirectory( cansim )
        add_subdirectory( boolop )  
        
        set_target_properties( 
            ${sample_canvas_folder}
            PROPERTIES FOLDER samples/wxart2d/canvas )
        
    ENDIF( WXART2D_COMP_SAMP_CANVAS )

ENDIF( WXART2D_USE_CANVAS )

