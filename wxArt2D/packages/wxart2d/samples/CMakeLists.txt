PROJECT( samples )

MACRO( ADD_MODULE modulesname )
    IF ( EXISTS ${samples_SOURCE_DIR}/${modulesname} )
        add_subdirectory( ${modulesname} )
    ENDIF ( EXISTS ${samples_SOURCE_DIR}/${modulesname} )
ENDMACRO( ADD_MODULE modulesname )

ADD_MODULE( curves )
ADD_MODULE( editor )
ADD_MODULE( canvas )
ADD_MODULE( aggdrawer )
ADD_MODULE( artbase )
