# - Locate Agg libraries
# This module defines
#  Agg_LIBRARIES, the library to link against
#  Agg_FOUND, if false, do not try to link to AGG
#  Agg_INCLUDE_DIRS, where to find headers.
#
# $Agg_DIR is an environment variable that would
# correspond to the install directory on e.g. windows.

# Created by Klaas Holwerda. 
# Search only if the location is not already known.

  SET( Agg_DIR_BIN Agg_DIR_BIN-NOTFOUND CACHE  INTERNAL "" )

  #
  # Look for an installation or build tree.
  #
  FIND_PATH(Agg_DIR_BIN AggConfig.cmake
    # Look for an environment variable Agg_DIR.
    $ENV{Agg_DIR}/bin
    ${Agg_DIR}/bin

    # Look in search path.
    $ENV{PATH}

    NO_DEFAULT_PATH 
    DOC "WXART2D_DIR found"
    
    # Help the user find it if we cannot.
    DOC "The Agg bin dir"
  )
  
  
IF ( NOT Agg_INSIDE_BUILD )
  FIND_PATH(Agg_DIR_BIN AggConfig.cmake

    # Look in standard UNIX install locations.
    /usr/local/bin
    /usr/local/lib/agg
    /usr/lib/agg
    /usr/local/include/agg
    /usr/local/include
    /usr/include
    /usr/local/agg
    /usr/X11R6/include

    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Agg\\antigrain 0.1.1]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Agg_is1;Inno Setup: App Path]/bin"
 
    # Read from the CMakeSetup registry entries.  It is likely that
    # AGG will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]/bin
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]/bin

    # Help the user find it if we cannot.
    DOC "The Agg bin dir"
  )
ENDIF()

# If AGG was found, load the configuration file to get the rest of the
# settings.
IF( Agg_DIR_BIN )

  # Check if AGG was built using CMake
    IF(EXISTS ${Agg_DIR_BIN}/AggConfig.cmake)
    SET(Agg_BUILT_WITH_CMAKE 1)
    ENDIF(EXISTS ${Agg_DIR_BIN}/AggConfig.cmake)

  IF(Agg_BUILT_WITH_CMAKE)
        INCLUDE(${Agg_DIR_BIN}/AggConfig.cmake)
        # at this point  Agg_LIBRARIES Agg_INCLUDE_DIRS etc. are set .
  ELSE(Agg_BUILT_WITH_CMAKE)
  
    # oh no! Oke lets do things the hard way.
  
    FIND_PATH(Agg_INCLUDE_DIR agg_config.h 
      ${Agg_DIR}/include
      NO_DEFAULT_PATH
    )

    SET( Agg_INCLUDE_DIRS ${Agg_INCLUDE_DIRS} ${Agg_INCLUDE_DIR} )

    FIND_LIBRARY(Agg_LIBRARY_agg
      NAMES agg
      PATHS
      ${Agg_DIR}
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib
    )

    SET( Agg_LIBRARIES ${Agg_LIBRARIES} ${Agg_LIBRARY_agg} )

    FIND_LIBRARY(Agg_LIBRARY_aggctrl
      NAMES aggctrl
      PATHS
      ${Agg_DIR}
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib
    )

    SET( Agg_LIBRARIES ${Agg_LIBRARIES} ${Agg_LIBRARY_aggctrl} )

    FIND_LIBRARY(Agg_LIBRARY_aggfontfreetype
      NAMES aggfontfreetype
      PATHS
      ${Agg_DIR}
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib
    )

    SET( Agg_LIBRARIES ${Agg_LIBRARIES} ${Agg_LIBRARY_aggfontfreetype} )

    FIND_LIBRARY(Agg_LIBRARY_aggplatform
      NAMES aggplatform
      PATHS
      ${Agg_DIR}
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib
    )

    SET( Agg_LIBRARIES ${Agg_LIBRARIES} ${Agg_LIBRARY_aggplatform} )

    FIND_LIBRARY(Agg_LIBRARY_gpc
      NAMES gpc
      PATHS
      ${Agg_DIR}
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib
    )

    SET( Agg_LIBRARIES ${Agg_LIBRARIES} ${Agg_LIBRARY_gpc} )
    SET( Agg_INCLUDE_DIRS ${Agg_INCLUDE_DIR} )
  
  ENDIF(Agg_BUILT_WITH_CMAKE)
	
ENDIF( Agg_DIR_BIN)

# handle the QUIETLY and REQUIRED arguments and set Agg_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Agg  DEFAULT_MSG  Agg_DIR_BIN Agg_LIBRARIES  Agg_INCLUDE_DIRS)

MARK_AS_ADVANCED( Agg_DIR_BIN Agg_LIBRARIES Agg_INCLUDE_DIRS)

IF( AGG_FOUND)
    IF(NOT Agg_FIND_QUIETLY)
        MESSAGE ( STATUS "Agg_DIR => ${Agg_DIR}" )
        MESSAGE ( STATUS "Agg_DIR_BIN => ${Agg_DIR_BIN}" )
        MESSAGE ( STATUS "Agg_DIR_SHARE => ${Agg_DIR_SHARE}" )
        MESSAGE ( STATUS "Agg_INCLUDE_DIRS => ${Agg_INCLUDE_DIRS}" )
        MESSAGE ( STATUS "Agg_LIBRARIES => ${Agg_LIBRARIES}" )
        MESSAGE ( STATUS "Agg_LIBRARY_DIRS => ${Agg_LIBRARY_DIRS}" )
    ENDIF(NOT Agg_FIND_QUIETLY)
ELSE( AGG_FOUND)
    IF(NOT Agg_FIND_QUIETLY)
        IF(Agg_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "Agg required, please specify its location with Agg_DIR.")
        ELSE(Agg_FIND_REQUIRED)
            MESSAGE(STATUS "Agg was not found.")
        ENDIF(Agg_FIND_REQUIRED)
    ENDIF(NOT Agg_FIND_QUIETLY)
ENDIF( AGG_FOUND)


