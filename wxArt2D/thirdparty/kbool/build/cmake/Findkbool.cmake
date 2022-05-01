# - Locate Kbool libraries
# This module defines
#  kbool_LIBRARIES, the library to link against
#  kbool_FOUND, if false, do not try to link to Kbool
#  kbool_INCLUDE_DIRS, where to find headers.
#
# $kbool_DIR is an environment variable that would
# correspond to the install directory on e.g. windows.

# Created by Klaas Holwerda. 
# Search only if the location is not already known.

  SET( kbool_DIR_BIN kbool_DIR_BIN-NOTFOUND CACHE  INTERNAL "" )

  #
  # Look for an installation or build tree.
  #
  FIND_PATH( kbool_DIR_SHARE kboolConfig.cmake
    # Look for an environment variable kbool_DIR.
    $ENV{kbool_DIR}/share/kbool
    ${kbool_DIR}/share/kbool

    # Look in search path.
    $ENV{PATH}

    NO_DEFAULT_PATH 
    
    # Help the user find it if we cannot.
    DOC "The kbool share dir"
  )
  
IF ( NOT kbool_INSIDE_BUILD )
  FIND_PATH( kbool_DIR_SHARE kboolConfig.cmake

    # Look in standard UNIX install locations.
    /usr/local/share/kbool
    /usr/share/kbool

    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Kbool\\kbool 0.1.1]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\kbool_is1;Inno Setup: App Path]/bin"
 
    # Read from the CMakeSetup registry entries.  It is likely that
    # Kbool will have been recently built.
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
    DOC "The Kbool share dir"
  )
ENDIF()

# If Kbool was found, load the configuration file to get the rest of the
# settings.
IF( kbool_DIR_SHARE )
    INCLUDE(${kbool_DIR_SHARE}/kboolConfig.cmake)
    # at this point  kbool_LIBRARIES kbool_INCLUDE_DIRS etc. are set .
ENDIF( kbool_DIR_SHARE)

# handle the QUIETLY and REQUIRED arguments and set kbool_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( kbool  DEFAULT_MSG  kbool_DIR_SHARE kbool_LIBRARIES  kbool_INCLUDE_DIRS)

MARK_AS_ADVANCED( kbool_DIR_BIN kbool_LIBRARIES kbool_INCLUDE_DIRS)

IF( KBOOL_FOUND)
    IF(NOT kbool_FIND_QUIETLY)
        MESSAGE ( STATUS "kbool_DIR => ${kbool_DIR}" )
        MESSAGE ( STATUS "kbool_DIR_SHARE => ${kbool_DIR_SHARE}" )
        MESSAGE ( STATUS "kbool_INCLUDE_DIRS => ${kbool_INCLUDE_DIRS}" )
        MESSAGE ( STATUS "kbool_LIBRARIES => ${kbool_LIBRARIES}" )
        MESSAGE ( STATUS "kbool_LIBRARY_DIRS => ${kbool_LIBRARY_DIRS}" )
    ENDIF(NOT kbool_FIND_QUIETLY)
ELSE()
    IF(NOT kbool_FIND_QUIETLY)
        IF(kbool_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "Kbool required, please specify its location with kbool_DIR.")
        ELSE(kbool_FIND_REQUIRED)
            MESSAGE(STATUS "Kbool was not found.")
        ENDIF(kbool_FIND_REQUIRED)
    ENDIF(NOT kbool_FIND_QUIETLY)
ENDIF()


