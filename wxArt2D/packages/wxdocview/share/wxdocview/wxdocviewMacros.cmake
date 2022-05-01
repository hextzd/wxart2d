# Macro to provide an option only if a set of other variables are ON.
# Example invocation:
#
#  WXDOCVIEW_DEPENDENT_OPTION(USE_FOO "Use Foo" ON "USE_BAR;USE_ZOT" OFF)
#
# If both USE_BAR and USE_ZOT are true, this provides an option called
# USE_FOO that defaults to ON.  Otherwise, it sets USE_FOO to OFF.  If
# the status of USE_BAR or USE_ZOT ever changes, any value for the
# USE_FOO option is saved so that when the option is re-enabled it
# retains its old value.
#
MACRO(WXDOCVIEW_DEPENDENT_OPTION option doc default depends force)
  SET(${option}_AVAILABLE 1)

  # Test for each required variable.
  FOREACH(d ${depends})
    IF(NOT ${d})
      SET(${option}_AVAILABLE 0)
    ENDIF(NOT ${d})
  ENDFOREACH(d)
  IF(${option}_AVAILABLE)
    # The option is available.
    OPTION(${option} "${doc}" "${default}")

    # Show the cache value to the user.
    SET(${option} "${${option}}" CACHE BOOL "${doc}" FORCE)
  ELSE(${option}_AVAILABLE)
    # Force the option value to be that specified.
    SET(${option} ${force})
  ENDIF(${option}_AVAILABLE)
ENDMACRO(WXDOCVIEW_DEPENDENT_OPTION)

MACRO( DOCVIEW_INCLUDE_DIRECTORIES_DEPENDS dirs )
    INCLUDE_DIRECTORIES( ${dirs} )
    FOREACH(d ${dirs})
        SET( wxDocview_INCLUDE_DIRS_DEPENDS ${wxDocview_INCLUDE_DIRS_DEPENDS} ${d})
    ENDFOREACH(d ${dirs})        
ENDMACRO( DOCVIEW_INCLUDE_DIRECTORIES_DEPENDS )

MACRO( DOCVIEW_INCLUDE_DIRECTORIES dirs )
    INCLUDE_DIRECTORIES( ${dirs} )
    FOREACH(d ${dirs})
        SET( wxDocview_INCLUDE_DIRS ${wxDocview_INCLUDE_DIRS} ${d})
    ENDFOREACH(d ${dirs})        
ENDMACRO( DOCVIEW_INCLUDE_DIRECTORIES )

MACRO( DOCVIEW_LINK_DIRECTORIES_DEPENDS dirs )
    LINK_DIRECTORIES( ${dirs} )
    FOREACH(d ${dirs})
        SET( wxDocview_LIBRARY_DIRS_DEPENDS ${wxDocview_LIBRARY_DIRS_DEPENDS} ${d})
    ENDFOREACH(d ${dirs})        
ENDMACRO( DOCVIEW_LINK_DIRECTORIES_DEPENDS )

MACRO( DOCVIEW_ADD_CXXFLAGS flags )
    FOREACH(d ${flags})
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${d}"  CACHE STRING "include wxWidgets flags"  FORCE)
    ENDFOREACH(d)
    SET( wxDocview_FLAGS ${wxDocview_FLAGS} ${flags})
ENDMACRO( DOCVIEW_ADD_CXXFLAGS )

MACRO( DOCVIEW_ADD_DEFINITIONS defines )
    FOREACH(d ${defines})
        add_definitions( ${d} )
    ENDFOREACH(d)
    SET( wxDocview_FLAGS ${wxDocview_FLAGS} ${defines})
ENDMACRO( DOCVIEW_ADD_DEFINITIONS )

MACRO( DOCVIEW_THIRDPARTY_LIBRARIES linklibs FRONT )
    if ( NOT "${linklibs}" STREQUAL "" )
    	set( linkthis "" )
        FOREACH(d ${linklibs})
            SET( LINK TRUE )
            FOREACH(lib ${wxDocview_THIRDPARTY_LIBRARIES})
                IF (${d} STREQUAL "${lib}" )
                    SET( LINK FALSE )
                ENDIF ()
            ENDFOREACH(lib ${wxDocview_THIRDPARTY_LIBRARIES})
            
            IF (  LINK MATCHES TRUE )   
                set( linkthis ${linkthis} ${d} )
            ENDIF ()   
        ENDFOREACH(d)
        if ( FRONT STREQUAL "FRONT" )
            SET( wxDocview_THIRDPARTY_LIBRARIES ${linkthis} ${wxDocview_THIRDPARTY_LIBRARIES} )         
        else ()
            SET( wxDocview_THIRDPARTY_LIBRARIES ${wxDocview_THIRDPARTY_LIBRARIES} ${linkthis} )         
        endif ()
    endif ()
ENDMACRO( DOCVIEW_THIRDPARTY_LIBRARIES )

MACRO( DOCVIEW_LINK_LIBRARIES linklibs FRONT )
    if ( NOT "${linklibs}" STREQUAL "" )
    	set( linkthis "" )
        FOREACH(d ${linklibs})
            SET( LINK TRUE )
            FOREACH(lib ${wxDocview_LIBRARIES})
                IF (${d} STREQUAL "${lib}" )
                    SET( LINK FALSE )
                ENDIF ()
            ENDFOREACH(lib ${wxDocview_LIBRARIES})
            
            IF (  LINK MATCHES TRUE )   
                set( linkthis ${linkthis} ${d} )
            ENDIF ()   
        ENDFOREACH(d)
        if ( FRONT STREQUAL "FRONT" )
            SET( wxDocview_LIBRARIES ${linkthis} ${wxDocview_LIBRARIES} )   
        else ()
            SET( wxDocview_LIBRARIES ${wxDocview_LIBRARIES} ${linkthis} )  
        endif ()
    endif ()
ENDMACRO( DOCVIEW_LINK_LIBRARIES )

MACRO( DOCVIEW_LINK_LIBRARIES_DEPENDS linklibs )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        SET( OUTPUTLIBPREFIX wxdocview_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_RELEASE_NODOT}${wxWidgets_UCD}${wxWidgets_DBG}_ )
        SET( OUTPUTLIBPOSTFIX "" )
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
        # how will output libraries be  prefixed
        SET( OUTPUTLIBPREFIX wxdocview_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_ )
        SET( OUTPUTLIBPOSTFIX "-${wxWidgets_RELEASE}" )      
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
  
    FOREACH(d ${linklibs})
        set( wanted ${OUTPUTLIBPREFIX}${d}${OUTPUTLIBPOSTFIX} )
        SET( LINK TRUE )
        FOREACH(lib ${wxDocview_LIBRARIES_DEPENDS})
            IF (${wanted} STREQUAL "${lib}" )
                SET( LINK FALSE )
            ENDIF (${wanted} STREQUAL "${lib}" )
        ENDFOREACH(lib ${wxDocview_LIBRARIES_DEPENDS})
        
        IF (  LINK MATCHES TRUE )   
            SET( wxDocview_LIBRARIES_DEPENDS ${wanted} ${wxDocview_LIBRARIES_DEPENDS} )   
        ENDIF (  LINK MATCHES TRUE )   
    ENDFOREACH(d ${linklibs})

ENDMACRO( DOCVIEW_LINK_LIBRARIES_DEPENDS linklibs )

MACRO( LINK_LIBRARIES_WXLIKE basename linklibs )

    IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        SET( OUTPUTLIBPREFIX ${basename}_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_RELEASE_NODOT}${wxWidgets_UCD}${wxWidgets_DBG}_ )
        SET( OUTPUTLIBPOSTFIX "" )
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
        # how will output libraries be  prefixed
        SET( OUTPUTLIBPREFIX ${basename}_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_ )
        SET( OUTPUTLIBPOSTFIX "-${wxWidgets_RELEASE}" )      
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )

    FOREACH(d ${linklibs})
        set( wanted ${OUTPUTLIBPREFIX}${d}${OUTPUTLIBPOSTFIX} )
        SET( LINK TRUE )
        FOREACH( lib ${wxDocview_LIBRARIES_WXLIKE})
            IF (${wanted} STREQUAL "${lib}" )
                SET( LINK FALSE )
            ENDIF (${wanted} STREQUAL "${lib}" )
        ENDFOREACH( lib ${wxDocview_LIBRARIES_WXLIKE})
        
        IF (  LINK MATCHES TRUE )
            SET( wxDocview_LIBRARIES_WXLIKE ${wxDocview_LIBRARIES_WXLIKE} ${wanted} CACHE INTERNAL "")
        ENDIF (  LINK MATCHES TRUE )   
    ENDFOREACH(d ${linklibs})
                
ENDMACRO( LINK_LIBRARIES_WXLIKE )

# This macro is called by modules that use precompiled headers. It sets
# docview_PCH_SOURCE_FILE to the source file used as precompiled header source
# by MSVC, or empty if target is not MSVC.
MACRO( DOCVIEW_PCH )
    IF ( WX_USE_PCH )
        # disable precompiled headers for vs.net, to be solved.
        IF (CMAKE_GENERATOR MATCHES "NET")
        ELSE (CMAKE_GENERATOR MATCHES "NET")
            ADD_DEFINITIONS( /Yu"docviewprec.h" )
            SET( docview_PCH_SOURCE_FILE
                ${wxdocview_SOURCE_DIR}/docview/src/dummy.cpp
            )
            SET_SOURCE_FILES_PROPERTIES( ${wxdocview_SOURCE_DIR}/docview/src/dummy.cpp PROPERTIES COMPILE_FLAGS "/Yc\"docviewprec.h\"")
        ENDIF (CMAKE_GENERATOR MATCHES "NET")
    ELSE ( WX_USE_PCH  )
        SET( docview_PCH_SOURCE_FILE ""
        )
    ENDIF ( WX_USE_PCH )
ENDMACRO( DOCVIEW_PCH )

# *.rc files are not automatically compiled by cmake on mingw/cygwin
MACRO( COMPILE_RC SRCFILE DSTFILE )
    IF( CYGWIN OR MINGW )
        ADD_CUSTOM_COMMAND(
            OUTPUT ${DSTFILE}
            COMMAND windres
            ARGS -i ${SRCFILE} -o ${DSTFILE} --include-dir ${wxWidgets_ROOT_DIR}/include
        )
    ENDIF( CYGWIN OR MINGW )
ENDMACRO( COMPILE_RC )

# copy files in FILES from SRCDIR to DESTDIR, unless they are those same
MACRO( COPYFILES SRCDIR DESTDIR FILES )
    IF("${SRCDIR}" MATCHES "^${DESTDIR}$" )
    ELSE("${SRCDIR}" MATCHES "^${DESTDIR}$" )
        
        FOREACH(file ${FILES})
            SET(src "${SRCDIR}/${file}")
            SET(tgt "${DESTDIR}/${file}")
            CONFIGURE_FILE(${src} ${tgt} COPYONLY )
        ENDFOREACH(file)  
      
    ENDIF("${SRCDIR}" MATCHES "^${DESTDIR}$" )
ENDMACRO( COPYFILES SRCDIR DESTDIR FILES )

# copy files from SRCDIR to DESTDIR, unless they are those same
MACRO( COPYFILESALL SRCDIR DESTDIR  FILTER )
    IF("${SRCDIR}" MATCHES "^${DESTDIR}$" )
    ELSE("${SRCDIR}" MATCHES "^${DESTDIR}$" )
        
        FILE(GLOB FILES "${SRCDIR}/${FILTER}" )
                                
        FOREACH(file ${FILES})        
            GET_FILENAME_COMPONENT(filename ${file} NAME)
            IF ( "${SRCDIR}/${filename}" MATCHES ".svn" )
            ELSE( "${SRCDIR}/${filename}" MATCHES ".svn" )
            SET(src "${SRCDIR}/${filename}")
            SET(tgt "${DESTDIR}/${filename}")
            CONFIGURE_FILE(${src} ${tgt} COPYONLY)
            ENDIF ( "${SRCDIR}/${filename}" MATCHES ".svn" )
        ENDFOREACH(file)  
      
    ENDIF("${SRCDIR}" MATCHES "^${DESTDIR}$" )
ENDMACRO( COPYFILESALL SRCDIR DESTDIR  )

MACRO( GENERATE_POT path name )
if ( GETTEXT_FOUND )
    SET( _potFile ${name}.pot)

    ADD_CUSTOM_COMMAND(
        OUTPUT ${_potFile}
        COMMAND ${XGETTEXT_CMD} ${_xgettext_option_list} ${path}/${_potFile} ${${name}_SOURCE} ${${name}_HEADERS}
        DEPENDS ${${name}_SOURCE} ${${name}_HEADERS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    ADD_CUSTOM_TARGET( ${name}_pot_file ALL
      DEPENDS ${_potFile}
    ) 
    
    SET( wxDocview_POT_FILES ${wxDocview_POT_FILES} ${name}.pot  CACHE INTERNAL ""  )
    
endif ()
ENDMACRO()

# ---------------------------------------------------------------------------
# Replace the ADD_LIBRARY(name ...) function to remember what
# library targets were added so we can do things with them.
# ---------------------------------------------------------------------------

UNSET(wxDocview_LIBRARY_LIST CACHE)
UNSET(wxDocview_LIBRARY_DIR_LIST CACHE)
UNSET(wxDocview_LIBRARY_DIR_NAME_LIST CACHE) 
UNSET(wxDocview_LIBRARY_LIST_WXLIKE CACHE)
UNSET(wxDocview_LIBRARY_DIR_LIST_WXLIKE CACHE)
UNSET(wxDocview_LIBRARY_DIR_NAME_LIST_WXLIKE CACHE) 

#UNSET(wxDocview_LIBRARIES_WXLIKE CACHE)

FUNCTION( ADD_LIBRARY name)
    _ADD_LIBRARY(${name} ${ARGN})

    # Get the last part of the directory we're currently in
    STRING(REPLACE "${CMAKE_HOME_DIRECTORY}/" "" PROJ_DIR_TEMP ${CMAKE_CURRENT_SOURCE_DIR})

    SET( wxDocview_LIBRARY_LIST          ${wxDocview_LIBRARY_LIST}          "${name}" CACHE INTERNAL "")
    SET( wxDocview_LIBRARY_DIR_LIST      ${wxDocview_LIBRARY_DIR_LIST}      "${PROJ_DIR_TEMP}" CACHE INTERNAL "")
    SET( wxDocview_LIBRARY_DIR_NAME_LIST ${wxDocview_LIBRARY_DIR_NAME_LIST} "${PROJ_DIR_TEMP}/${name}" CACHE INTERNAL "")
ENDFUNCTION()

FUNCTION( ADD_LIBRARY_WXLIKE basename name )
    _ADD_LIBRARY(${name} ${ARGN})

    # Get the last part of the directory we're currently in
    STRING(REPLACE "${CMAKE_HOME_DIRECTORY}/" "" PROJ_DIR_TEMP ${CMAKE_CURRENT_SOURCE_DIR})

    SET( wxDocview_LIBRARY_LIST_WXLIKE          ${wxDocview_LIBRARY_LIST_WXLIKE}          "${name}" CACHE INTERNAL "")
    SET( wxDocview_LIBRARY_DIR_LIST_WXLIKE      ${wxDocview_LIBRARY_DIR_LIST_WXLIKE}      "${PROJ_DIR_TEMP}" CACHE INTERNAL "")
    SET( wxDocview_LIBRARY_DIR_NAME_LIST_WXLIKE ${wxDocview_LIBRARY_DIR_NAME_LIST_WXLIKE} "${PROJ_DIR_TEMP}/${name}" CACHE INTERNAL "")
    
    LINK_LIBRARIES_WXLIKE( ${basename} ${name} )
    SET_AS_WXLIKE_LIBRARY( ${basename} ${name} )
ENDFUNCTION()

