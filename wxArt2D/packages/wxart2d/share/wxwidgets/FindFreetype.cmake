# - Locate FreeType library
# This module defines
#  FREETYPE_LIBRARIES, the library to link against
#  FREETYPE_FOUND, if false, do not try to link to FREETYPE
#  FREETYPE_INCLUDE_DIRS, where to find headers.
#  This is the concatenation of the paths:
#  FREETYPE_INCLUDE_DIR_ft2build
#  FREETYPE_INCLUDE_DIR_freetype2
#
# $FREETYPE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FREETYPE_DIR
# used in building FREETYPE.

# Created by Eric Wing. 
# Modifications by Alexander Neundorf.
# This file has been renamed to "FindFreetype.cmake" instead of the correct
# "FindFreeType.cmake" in order to be compatible with the one from KDE4, Alex.

# Ugh, FreeType seems to use some #include trickery which 
# makes this harder than it should be. It looks like they
# put ft2build.h in a common/easier-to-find location which
# then contains a #include to a more specific header in a 
# more specific location (#include <freetype/config/ftheader.h>).
# Then from there, they need to set a bunch of #define's 
# so you can do something like:
# #include FT_FREETYPE_H
# Unfortunately, using CMake's mechanisms like INCLUDE_DIRECTORIES()
# wants explicit full paths and this trickery doesn't work too well.
# I'm going to attempt to cut out the middleman and hope 
# everything still works.

set( FREETYPE_LIBRARY "FREETYPE_LIBRARY-NOTFOUND" CACHE FILEPATH "Cleared." FORCE)

FIND_PATH(FREETYPE_INCLUDE_DIR_ft2build ft2build.h 
  $ENV{FREETYPE_DIR}/include
  "${FREETYPE_DIR}/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\FreeType-2.5.5-1_is1;Inno Setup: App Path]/include"
  NO_DEFAULT_PATH
)

FIND_PATH(FREETYPE_INCLUDE_DIR_ft2build ft2build.h 
  PATHS
  /usr/local/X11R6/include
  /usr/local/X11/include
  /usr/X11/include
  /sw/include
  /opt/local/include
  /usr/freeware/include
  /usr/include/freetype2
)

FIND_PATH(FREETYPE_INCLUDE_DIR_freetype2 config/ftheader.h 
  $ENV{FREETYPE_DIR}/include/freetype2
  ${FREETYPE_DIR}/include/freetype2
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\FreeType-2.5.5-1_is1;Inno Setup: App Path]/include/freetype2"
  NO_DEFAULT_PATH
)

FIND_PATH(FREETYPE_INCLUDE_DIR_freetype2 config/ftheader.h 
  $ENV{FREETYPE_DIR}/include
  ${FREETYPE_DIR}/include
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\FreeType-2.5.5-1_is1;Inno Setup: App Path]/include/freetype2"
  NO_DEFAULT_PATH
)

FIND_PATH(FREETYPE_INCLUDE_DIR_freetype2 config/ftheader.h 
  PATHS
  /usr/local/X11R6/include
  /usr/local/X11/include
  /usr/X11/include
  /sw/include
  /opt/local/include
  /usr/freeware/include
  /usr/include/freetype2/freetype
  PATH_SUFFIXES freetype2
)

IF( WIN32 )

    # cygwin
    FIND_LIBRARY(FREETYPE_LIBRARY
      NAMES freetype libfreetype freetype255
      PATHS
      $ENV{FREETYPE_DIR}
      ${FREETYPE_DIR}
      ${FREETYPE_DIR}/lib
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib 
    )

    if( MINGW  )           
        FIND_LIBRARY(FREETYPE_LIBRARY
          NAMES freetype.a 
          PATHS
          $ENV{FREETYPE_DIR}
          ${FREETYPE_DIR}
          ${FREETYPE_DIR}/objs
          NO_DEFAULT_PATH
        )
    else( MINGW  )
	
		if ( MSVC12 OR MSVC10 OR MSVC14 OR MSVC15 OR MSVC16 )
		#if ( MSVC10 )
			set( FREETYPE_DIR_ARCH_DIR ${FREETYPE_DIR}/objs/vc2010 )
			if ( CMAKE_CL_64 )
				set( FREETYPE_DIR_ARCH_DIR ${FREETYPE_DIR_ARCH_DIR}/x64 )
			else()
				set( FREETYPE_DIR_ARCH_DIR ${FREETYPE_DIR_ARCH_DIR}/win32 )
			endif()
		else()
			set( FREETYPE_DIR_ARCH_DIR ${FREETYPE_DIR} )
			if ( CMAKE_CL_64 )
				set( FREETYPE_DIR_ARCH_DIR ${FREETYPE_DIR_ARCH_DIR}/objs/x64/vc2008 )
			else()
				set( FREETYPE_DIR_ARCH_DIR ${FREETYPE_DIR_ARCH_DIR}/objs/win32/vc2008 )
			endif()
		endif()
		

        if ( FREETYPE_LIBRARY_DEBUG )
            FIND_LIBRARY(FREETYPE_LIBRARY
              NAMES freetype255d.lib freetype255MTd.lib freetype255MT_D.lib freetype255_D.lib
              PATHS
              $ENV{FREETYPE_DIR}
              ${FREETYPE_DIR}
              ${FREETYPE_DIR_ARCH_DIR}
              NO_DEFAULT_PATH
            )
        else()
            FIND_LIBRARY(FREETYPE_LIBRARY
              NAMES freetype255.lib freetype255MT.lib freetype255MT_D.lib
              PATHS
              $ENV{FREETYPE_DIR}
              ${FREETYPE_DIR}
              ${FREETYPE_DIR_ARCH_DIR}
              NO_DEFAULT_PATH
            )
        endif()
    endif( MINGW  )           

    IF( FREETYPE_LIBRARY )
        if( MINGW  )           
            set( FREETYPE_LIBRARY_DIRS "${FREETYPE_DIR}/objs" )
        else( MINGW  )           
            set( FREETYPE_LIBRARY_DIRS "${FREETYPE_DIR_ARCH_DIR}" )
        endif( MINGW  )         
    ENDIF( FREETYPE_LIBRARY )
ENDIF( WIN32 )

# unix/linux
FIND_LIBRARY(FREETYPE_LIBRARY
  NAMES freetype libfreetype freetype255
  PATHS
  /usr/lib/x86_64-linux-gnu
  /usr/local/X11R6
  /usr/local/X11
  /usr/X11
  /sw
  /usr/freeware
  PATH_SUFFIXES lib64 lib
)

# set the user variables
IF(FREETYPE_INCLUDE_DIR_ft2build AND FREETYPE_INCLUDE_DIR_freetype2)
  SET(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_ft2build};${FREETYPE_INCLUDE_DIR_freetype2}")
ENDIF(FREETYPE_INCLUDE_DIR_ft2build AND FREETYPE_INCLUDE_DIR_freetype2)

set( FREETYPE_LIBRARY ${FREETYPE_LIBRARY} CACHE STRING "Freetype lib"  FORCE )
SET(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set FREETYPE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

if ( WIN32 )
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype  DEFAULT_MSG  FREETYPE_LIBRARIES  FREETYPE_LIBRARY_DIRS FREETYPE_INCLUDE_DIRS)
else ( WIN32 )
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype  DEFAULT_MSG  FREETYPE_LIBRARIES  FREETYPE_INCLUDE_DIRS)
endif ( WIN32 )

MARK_AS_ADVANCED(FREETYPE_LIBRARY FREETYPE_INCLUDE_DIR_freetype2 FREETYPE_INCLUDE_DIR_ft2build)

IF( FREETYPE_FOUND)
    IF(NOT Freetype_FIND_QUIETLY)
        MESSAGE ( STATUS "FREETYPE_DIR => ${FREETYPE_DIR}" )
        MESSAGE ( STATUS "FREETYPE_INCLUDE_DIRS => ${FREETYPE_INCLUDE_DIRS}" )
        MESSAGE ( STATUS "FREETYPE_LIBRARIES => ${FREETYPE_LIBRARIES}" )
        MESSAGE ( STATUS "FREETYPE_LIBRARY_DIRS => ${FREETYPE_LIBRARY_DIRS}" )
    ENDIF(NOT Freetype_FIND_QUIETLY)
ELSE( FREETYPE_FOUND)
    #IF(NOT Freetype_FIND_QUIETLY)
        IF( Freetype_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "FREETYPE required, please specify its location with FREETYPE_DIR.")
        ELSE(Freetype_FIND_REQUIRED)
            MESSAGE(STATUS "FREETYPE was not found.")
        ENDIF(Freetype_FIND_REQUIRED)
    #ENDIF(NOT Freetype_FIND_QUIETLY)
ENDIF( FREETYPE_FOUND)

