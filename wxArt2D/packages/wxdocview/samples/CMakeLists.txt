PROJECT( docviewsamples )

IF( wxDocview_USE_DOCVIEW )
        
    IF( wxDocview_COMP_SAMP_DOCVIEW )

        # only for samples
        INCLUDE_DIRECTORIES( ${docviewsamples_SOURCE_DIR}/common )

        add_subdirectory( drawings  )
        add_subdirectory( singleframe )
        add_subdirectory( mdi  )
        add_subdirectory( splitwin  )
        add_subdirectory( multiframe )
        add_subdirectory( nonmanaged )
        add_subdirectory( tabframe  )
        add_subdirectory( auiframe  )
    
        set_target_properties( 
            sample_docview_aui
            sample_docview_mdi
            sample_docview_multiframe
            sample_docview_nonmanaged
            sample_docview_singleframe
            sample_docview_splitwin
            sample_docview_tabframe
            PROPERTIES FOLDER samples/wxdocview )
    
    ENDIF( wxDocview_COMP_SAMP_DOCVIEW )
    
ENDIF( wxDocview_USE_DOCVIEW )



