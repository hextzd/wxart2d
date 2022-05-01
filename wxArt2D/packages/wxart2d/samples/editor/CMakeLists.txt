PROJECT( editor )

IF( WXART2D_USE_EDITOR )

    # only for samples, so no A2D_
    INCLUDE_DIRECTORIES( ${editor_SOURCE_DIR}/common )
    
    IF( WXART2D_COMP_SAMP_EDITOR )
        add_subdirectory( common )
        add_subdirectory( single2 )
        add_subdirectory( tuto4 )
        add_subdirectory( vdrawm )
        add_subdirectory( vdraws )
        add_subdirectory( viewmul )
        add_subdirectory( editmdi )
        add_subdirectory( zoom )
        add_subdirectory( wires )
        
        set_target_properties( 
            sample_editor_editmdi 
            sample_editor_single2
            sample_editor_tuto4
            sample_editor_viewmul
            sample_editor_zoom
            sample_editor_vdraws
            sample_editor_vdrawm
            sample_editor_wires
            PROPERTIES FOLDER samples/wxart2d/editor )
        
    ELSE( WXART2D_COMP_SAMP_EDITOR )
        IF( WXART2D_COMP_SAMP_FREQUENT )
            add_subdirectory( vdrawm )
            add_subdirectory( common )
        ENDIF( WXART2D_COMP_SAMP_FREQUENT )
    ENDIF( WXART2D_COMP_SAMP_EDITOR )

    
ENDIF( WXART2D_USE_EDITOR )
