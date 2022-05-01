#=====================================================================
# to find a wxWidgets package
# uses variable values from FindwxWidgets.cmake when it was included
#
# Input:
#   packName = name of the package e.g WXLUA ( converted to lowercase is used to search libraries }
#   packDir = The place where the package is searched for.
#   packWxlikeLibs = libraries named in a wxlike fashion and depend on cached variables wxWidgets_PORT wxWidgets_RELEASE(_NODOT) wxWidgets_UNV wxWidgets_UCD wxWidgets_DBG
#   packLibs = libraries names as is
#   packIncludes = include paths to check
#   packNeedfiles = files to check
#
# The following are set after macro is called:
#
#  ${packName}_FOUND             - Set to TRUE if package was found.
#  ${packName}_INCLUDE_DIRS - Include directories 
#  ${packName}_LIBRARIES        - Path to the package libraries.
#  ${packName}_LIBRARY_DIRS  - compile time link dirs
#  ${packName}_ROOT_DIR  - where is the package installed
#
# Sample usage:
#
#   FIND_WX_PACKAGE( WXLUA $ENV{WXLUA} "wxluadebug;wxluasocket;wxlua;wxbind" "lua5.1" "modules/lua/include;modules/wxbind/setup;modules" "" )
#   IF( WXLUA_FOUND )
#     # and for each of your dependant executable/library targets:
#     INCLUDE_DIRECTORIES( ${WXLUA_INCLUDE_DIRS} )
#     LINK_DIRECTORIES( ${WXLUA_LIBRARY_DIRS} )
#     TARGET_LINK_LIBRARIES(<YourTarget> ${WXLUA_LIBRARIES})
#   ENDIF( WXLUA_FOUND )
#
#=====================================================================
MACRO( FIND_WX_PACKAGE p_packName p_packDir p_packWxlikeLibs p_packLibs p_packIncludes p_packNeedfiles )

    STRING( TOLOWER ${p_packName} s_packNameLower )

    IF( IS_DIRECTORY ${p_packDir})
        SET( s_packFound TRUE )                      
    ELSE( IS_DIRECTORY ${p_packDir} )
        SET( s_packFound FALSE )                      
        MESSAGE( FATAL_ERROR "ERROR: package install path: \"${p_packDir}\" not found or is empty" )    
    ENDIF( IS_DIRECTORY ${p_packDir} )
    SET( ${p_packName}_ROOT_DIR ${p_packDir} CACHE STRING "package install path" )

    FOREACH( NEEDFILE ${p_packNeedfiles} )
        FIND_FILE( 
            FOUND_NEEDFILE
            NAMES
            ${NEEDFILE} 
            PATHS 
            ${p_packDir}
            /usr/include/${s_packNameLower}
            /usr/local/include/${s_packNameLower}
            ${CMAKE_INSTALL_PREFIX}/include/${s_packNameLower};
        )
        IF( NOT FOUND_NEEDFILE )
            MESSAGE( "ERROR: file: ${NEEDFILE} not found" )    
        ENDIF( NOT FOUND_NEEDFILE )
        MARK_AS_ADVANCED( FOUND_NEEDFILE )
    ENDFOREACH( NEEDFILE )    

    # form the naming 
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        IF( wxWidgets_USE_STATIC )
            SET( s_packLibraryDirs ${p_packDir}/lib/${s_wxCompiler}_lib )
        ELSE( wxWidgets_USE_STATIC )
            SET( s_packLibraryDirs ${p_packDir}/lib/${s_wxCompiler}_dll )
        ENDIF( wxWidgets_USE_STATIC )    
    ELSE( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )    
        SET( s_packLibraryDirs "${p_packDir}/lib;${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib;/usr/lib" )        
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
   
    SET( s_packLibraries "" )     
    # Find package libraries with naming like wxWidgets libraries
    FOREACH( packLib ${p_packWxlikeLibs} )
        IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )      
            SET( wxlikelib ${s_packNameLower}_${wxWidgets_PORT}${wxWidgets_RELEASE_NODOT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_${packLib} )
            SET( wxLikeLibAlt wx${wxWidgets_PORT}${wxWidgets_RELEASE_NODOT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_${packLib} )
        ELSE( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )    
            SET( wxlikelib ${s_packNameLower}_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_${packLib}-${wxWidgets_RELEASE} )
            SET( wxLikeLibAlt wx_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_${packLib}-${wxWidgets_RELEASE} )            
        ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        FIND_LIBRARY( ${p_packName}_${packLib}
            NAMES
            ${wxlikelib}
            PATHS ${s_packLibraryDirs}
            PATHS ${s_packLibraryDirs}/Debug
            PATHS ${s_packLibraryDirs}/Release
            NO_DEFAULT_PATH
        )   
        IF( NOT ${p_packName}_${packLib} )
            # try other naming scheme
            FIND_LIBRARY( ${p_packName}_alt_${packLib}
                NAMES
                ${wxLikeLibAlt}
                PATHS ${p_packDir}/lib
                PATHS ${p_packDir}/lib/Debug
                PATHS ${p_packDir}/lib/Release
                NO_DEFAULT_PATH
            )   
            IF( NOT ${p_packName}_alt_${packLib} )
                MESSAGE( "ERROR: ${packLib} as ${wxlikelib} or ${wxLikeLibAlt} not found in ${s_packLibraryDirs}" )    
            ELSE( NOT ${p_packName}_alt_${packLib} )
                GET_FILENAME_COMPONENT( ${p_packName}_${packLib} ${${p_packName}_alt_${packLib}} NAME_WE )
                SET( s_packLibraries ${s_packLibraries} ${${p_packName}_${packLib}} )                     
                MARK_AS_ADVANCED( ${p_packName}_${packLib} ) 
                SET( s_packLibraryDirs ${s_packLibraryDirs} ${p_packDir}/lib )        
            ENDIF( NOT ${p_packName}_alt_${packLib} )
        ELSE( NOT ${p_packName}_${packLib} )
            IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
            GET_FILENAME_COMPONENT( ${p_packName}_${packLib} ${${p_packName}_${packLib}} NAME_WE )
            ELSE( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
                STRING(REGEX REPLACE "\\.a" "" ${p_packName}_${packLib} "${${p_packName}_${packLib}}")
                GET_FILENAME_COMPONENT( ${p_packName}_${packLib}  ${${p_packName}_${packLib}} NAME )
                STRING(REGEX REPLACE "^lib" "" ${p_packName}_${packLib} "${${p_packName}_${packLib}}")
            ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )

            SET( s_packLibraries ${s_packLibraries} ${${p_packName}_${packLib}} )     
            MARK_AS_ADVANCED( ${p_packName}_${packLib} )           
        ENDIF( NOT ${p_packName}_${packLib} )
    ENDFOREACH(packLib)        
  
    # Find package libraries with any name
    FOREACH( packLib ${p_packLibs} )
        FIND_LIBRARY( ${p_packName}_${packLib}
            NAMES
            ${packLib}
            PATHS ${s_packLibraryDirs}
            NO_DEFAULT_PATH
        )   
        IF( NOT ${p_packName}_${packLib} )
            MESSAGE( "ERROR: ${packLib} not found in ${s_packLibraryDirs}" )    
        ENDIF( NOT ${p_packName}_${packLib} )
        GET_FILENAME_COMPONENT( ${p_packName}_${packLib} ${${p_packName}_${packLib}} NAME )
        SET( s_packLibraries ${s_packLibraries} ${${p_packName}_${packLib}} )              
        MARK_AS_ADVANCED( ${p_packName}_${packLib} )
    ENDFOREACH( packLib )    

    FOREACH( INCLUDE ${p_packIncludes} )
        IF( EXISTS ${p_packDir}/${INCLUDE} )
            SET( s_packIncludeDirs ${s_packIncludeDirs} ${p_packDir}/${INCLUDE} )                      
        ELSE( EXISTS ${p_packDir}/${INCLUDE} )
            MESSAGE( "ERROR: ${INCLUDE} not found in ${p_packDir}/${INCLUDE}" )    
        ENDIF( EXISTS ${p_packDir}/${INCLUDE} )
    ENDFOREACH( INCLUDE )    

    SET( ${p_packName}_LIBRARY_DIRS ${s_packLibraryDirs} CACHE STRING "package libs directory path" FORCE )
    SET( ${p_packName}_FOUND ${s_packFound} CACHE STRING "package was found" FORCE )                      
    SET( ${p_packName}_LIBRARIES ${s_packLibraries} CACHE STRING "package libs" FORCE )              
    SET( ${p_packName}_INCLUDE_DIRS ${s_packIncludeDirs} CACHE STRING "package include paths" FORCE )    
    MARK_AS_ADVANCED( ${p_packName}_FOUND )
    MARK_AS_ADVANCED( ${p_packName}_ROOT_DIR )
    MARK_AS_ADVANCED( ${p_packName}_LIBRARIES )
    MARK_AS_ADVANCED( ${p_packName}_INCLUDE_DIRS )
    MARK_AS_ADVANCED( ${p_packName}_LIBRARY_DIRS )
    
ENDMACRO( FIND_WX_PACKAGE )

#=====================================================================
# *.rc files are not automatically compiled by cmake on mingw/cygwin
#=====================================================================
MACRO( COMPILE_RC p_srcFile p_dstFile )
    IF( CYGWIN OR MINGW )
        ADD_CUSTOM_COMMAND(
            OUTPUT ${p_dstFile}
            COMMAND windres
            ARGS -i ${p_srcFile} -o ${p_dstFile} --include-dir ${wxWidgets_ROOT_DIR}/include
        )
    ENDIF( CYGWIN OR MINGW )
ENDMACRO( COMPILE_RC )

#=====================================================================
# After an ADD_LIBRARY command, this can be used to set the right properties, to generate wxWidgets like naming
# This is based on information found in the wxWidegts detection fase.
#=====================================================================
MACRO( SET_AS_WXLIKE_LIBRARY p_wxpackage p_libraryname )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        SET( OUTPUTLIBPREFIX ${p_wxpackage}_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_RELEASE_NODOT}${wxWidgets_UCD}${wxWidgets_DBG}_ )
        SET( OUTPUTLIBPOSTFIX "" )
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
        # how will output libraries be  prefixed
        SET( OUTPUTLIBPREFIX ${p_wxpackage}_${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}_ )
        SET( OUTPUTLIBPOSTFIX "-${wxWidgets_RELEASE}" )      
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
    
    SET_TARGET_PROPERTIES( ${p_libraryname} PROPERTIES OUTPUT_NAME ${OUTPUTLIBPREFIX}${p_libraryname}${OUTPUTLIBPOSTFIX} )
ENDMACRO( SET_AS_WXLIKE_LIBRARY )

#=====================================================================
# To set library output path for wx packages, in the unix or windows manner
#=====================================================================
MACRO( SET_WXLIKE_LIBRARY_PATH p_rootdir )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        IF( wxWidgets_USE_STATIC )
            SET ( LIBRARY_OUTPUT_PATH ${p_rootdir}/lib/${s_wxCompiler}_lib CACHE PATH "output directory for building library" FORCE )
        ELSE( wxWidgets_USE_STATIC )
            SET ( LIBRARY_OUTPUT_PATH ${p_rootdir}/lib/${s_wxCompiler}_dll CACHE PATH "output directory for building library" FORCE )
        ENDIF( wxWidgets_USE_STATIC )
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
    IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
        SET ( LIBRARY_OUTPUT_PATH ${p_rootdir}/lib CACHE PATH "output directory for building library" FORCE )
    ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
ENDMACRO( SET_WXLIKE_LIBRARY_PATH )

##################################################
# Set flags of wrong configuration to something erroneous, this will give compiler errors when still used.
# Understand that the configuration is always only suitable for one configuration ( meaning choosing
# another configuration then the one wanted in VC IDE does not make sence ).
##################################################
MACRO( SET_WX_SYNCLIBRARY_FLAGS )
    IF( wxWidgets_USE_DEBUG )
        SET( CMAKE_CONFIGURATION_TYPES "Debug" )
        SET (CMAKE_BUILD_TYPE  "Debug" )
        SET( CMAKE_CXX_FLAGS_RELEASE
    "THIS_IS_A_DEBUG_CONFIGURATION_AND_YOU_ARE_DOING_A_RELEASE_BUILD" )
        SET( CMAKE_CXX_FLAGS_MINSIZEREL
    "THIS_IS_A_DEBUG_CONFIGURATION_AND_YOU_ARE_DOING_A_RELEASE_BUILD" )
        SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "THIS_IS_A_DEBUG_CONFIGURATION_AND_YOU_ARE_DOING_A_RELEASE_BUILD" )
    ELSE( wxWidgets_USE_DEBUG )
        SET( CMAKE_CXX_FLAGS_DEBUG
    "THIS_IS_A_RELEASE_CONFIGURATION_AND_YOU_ARE_DOING_A_DEBUG_BUILD" )
        SET( CMAKE_CONFIGURATION_TYPES "Release" )
        SET (CMAKE_BUILD_TYPE  "Release" )
    ENDIF( wxWidgets_USE_DEBUG )
ENDMACRO( SET_WX_SYNCLIBRARY_FLAGS )

##################################################
# Enable precompiled headers for MSVC
##################################################

MACRO( SET_PRECOMPILED_HEADER_FLAGS )
    IF ( MSVC  )
        OPTION( WX_USE_PCH "Use precompiled headers" ON )   
        IF ( WX_USE_PCH )
            SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc" )
            SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3" )
        ELSE ( WX_USE_PCH )
            ADD_DEFINITIONS( -DNOPCH )
        ENDIF ( WX_USE_PCH )
    ELSE ( MSVC  )
            ADD_DEFINITIONS( -DNOPCH )	
    ENDIF ( MSVC )
ENDMACRO( SET_PRECOMPILED_HEADER_FLAGS )



