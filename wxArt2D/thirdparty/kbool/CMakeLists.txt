PROJECT( kbool )

IF( NOT DEFINED WXART2D_COMP_SAMP_BOOLEAN )
	SET(  WXART2D_COMP_SAMP_BOOLEAN 1 )
ENDIF( NOT DEFINED WXART2D_COMP_SAMP_BOOLEAN )

# additional are modified Find routines
SET ( CMAKE_MODULE_PATH "${kbool_SOURCE_DIR}/build/cmake" )

cmake_minimum_required(VERSION 2.8.12)

SET(kbool_MAJOR_VERSION 9 )
SET(kbool_MINOR_VERSION 1 )
SET(kbool_BUILD_VERSION 0 )

SET( kbool_FLAGS "" )
SET( kbool_INCLUDE_DIRS "" )
SET( kbool_LIBRARY_DIRS "" )
SET( kbool_LIBRARIES "" )

SET (LIBRARY_OUTPUT_PATH ${kbool_BINARY_DIR}/lib CACHE PATH "Output directory for building kbool library." FORCE )
SET( kbool_LIBRARY_DIRS  lib )
SET (EXECUTABLE_OUTPUT_PATH ${kbool_BINARY_DIR}/bin CACHE PATH "Output directory for building all executables.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

LINK_DIRECTORIES(  ${kbool_BINARY_DIR}/lib )

OPTION( kbool_MAKE_SHARED "Make shared version (.so) of Kbool libraries" OFF )
OPTION( kbool_USE_DEBUG   "Make debug version of Kbool libraries" OFF )

IF( kbool_USE_DEBUG )
    SET( PFDEBUG "d" )
    ADD_DEFINITIONS( -D_DEBUG_ )
    SET( kbool_FLAGS -D_DEBUG_ )
    #SET( CMAKE_DEBUG_POSTFIX "d" )
ELSE( kbool_USE_DEBUG )
    SET( PFDEBUG "" )
    ADD_DEFINITIONS( -D_NDEBUG_ )
    SET( kbool_FLAGS -D_NDEBUG_ )
ENDIF( kbool_USE_DEBUG )

# for the moment this decides the platform code.
IF(WIN32)
    ADD_DEFINITIONS( -D_MSWVC_ -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE )
    SET( WIN32GUI WIN32 )    
ENDIF(WIN32)

IF(UNIX)
    ADD_DEFINITIONS( -D__UNIX__  )
    SET( WIN32GUI "" )
ENDIF(UNIX)

# more specific set platform code part to use for different compilers/tool sets
IF ( ${CMAKE_GENERATOR} STREQUAL "MSYS Makefiles" )
	SET (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING
		"Flags used by the compiler during release builds" FORCE)
	SET (CMAKE_CX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING
		"Flags used by the compiler during release builds" FORCE)		
ENDIF ( ${CMAKE_GENERATOR} STREQUAL "MSYS Makefiles" )

IF ( ${CMAKE_GENERATOR} STREQUAL "MinGW Makefiles" )
	
ENDIF ( ${CMAKE_GENERATOR} STREQUAL "MinGW Makefiles" )

IF ( ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles" )
    IF( CYGWIN OR MINGW )
		SET (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING
			"Flags used by the compiler during release builds" FORCE)
		SET (CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE STRING
			"Flags used by the compiler during release builds" FORCE)
    ENDIF( CYGWIN OR MINGW )	
ENDIF ( ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles" )

IF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )
	
ENDIF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )

IF ( ${CMAKE_GENERATOR} MATCHES "Borland Makefiles" )

ENDIF ( ${CMAKE_GENERATOR} MATCHES "Borland Makefiles" )

# the main include dir of Kbool
INCLUDE_DIRECTORIES( ${kbool_SOURCE_DIR}/include )
SET( kbool_INCLUDE_DIRS ${kbool_INCLUDE_DIRS} include )    


SET( kbool_FLAGS ${kbool_FLAGS} CACHE STRING "Kbool package flags" FORCE )
SET( kbool_INCLUDE_DIRS include CACHE STRING "Kbool package libs include paths" FORCE )
SET( kbool_LIBRARY_DIRS ${kbool_BINARY_DIR}/lib CACHE STRING "Kbool package libs directory paths" FORCE )
SET( kbool_LIBRARIES kbool${PFDEBUG} CACHE STRING "Kbool package libraries" FORCE )

ADD_SUBDIRECTORY( src )

IF( kbool_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DKBOOLMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( kbool_MAKE_SHARED )
        
IF( WXART2D_COMP_SAMP_BOOLEAN )
	ADD_SUBDIRECTORY( samples )
ENDIF( WXART2D_COMP_SAMP_BOOLEAN )

add_subdirectory( build/cmake )

#-------------------------------------------------------------------
# Build a CPack installer if CPack is available and this is a build 
IF ( kbool_USE_PACK )    
    IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Kbool boolean operations")
        SET(CPACK_PACKAGE_VENDOR "Kbool")
        SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/copying")
        SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/copying")
        SET(CPACK_PACKAGE_VERSION_MAJOR "${kbool_MAJOR_VERSION}")
        SET(CPACK_PACKAGE_VERSION_MINOR "${kbool_MINOR_VERSION}")
        SET(CPACK_PACKAGE_VERSION_PATCH "${kbool_BUILD_VERSION}")
        SET(CPACK_PACKAGE_INSTALL_DIRECTORY "kbool_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
        SET(CPACK_SOURCE_PACKAGE_FILE_NAME "kbool-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
        SET(CPACK_PACKAGE_EXECUTABLES
        "kbool" "KBOOL"
        )
        SET(CPACK_SOURCE_STRIP_FILES "")
        SET(CPACK_STRIP_FILES "bin/ccmake;bin/cmake;bin/cpack;bin/ctest")
 

        IF(WIN32)
            SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
            SET(CPACK_NSIS_HELP_LINK "http://www.klaasholwerda.nl")
            SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.klaasholwerda.nl")
            SET(CPACK_NSIS_CONTACT "http://www.klaasholwerda.nl")
        ENDIF(WIN32)

        INCLUDE(CPack)
    ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")   
ENDIF ( kbool_USE_PACK )
