# - Locate wxArt2D libraries
# This module defines
#  WXART2D_LIBRARIES, the library to link against
#  WXART2D_FOUND, if false, do not try to link to wxArt2D
#  WXART2D_INCLUDE_DIRS, where to find headers.
#
# $WXART2D_DIR is an environment variable that would
# correspond to the install directory on e.g. windows.

# Created by Klaas Holwerda. 
# Search only if the location is not already known.
  
  SET( WXART2D_DIR_SHARE WXART2D_DIR_SHARE-NOTFOUND CACHE  INTERNAL "" )
  
  #
  # Look for build tree.
  #
  FIND_PATH(WXART2D_DIR_SHARE wxArt2DConfig.cmake
    # Look for an environment variable WXART2D_DIR.
    $ENV{WXART2D_DIR}/share/wxart2d
    ${WXART2D_DIR}/share/wxart2d

    # Look in search path.
    $ENV{PATH}
    
    NO_DEFAULT_PATH 
    DOC "WXART2D_DIR found"
  )

IF ( NOT WXART2D_INSIDE_BUILD )
  # Look for an installation
  FIND_PATH(WXART2D_DIR_SHARE wxArt2DConfig.cmake

    # Look in standard UNIX install locations.
    /usr/local/share/wxart2d
    /usr/share/wxart2d

    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\wxArt2D\\wxart2d 1.0.1]/share/wxart2d"
 
    # Read from the CMakeSetup registry entries.  It is likely that
    # wxArt2D will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]/share/wxart2d
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]/share/wxart2d

  )
ENDIF ()
  
# If WXART2D was found, load the configuration file to get the rest of the
# settings.
IF( WXART2D_DIR_SHARE )
    INCLUDE(${WXART2D_DIR_SHARE}/wxArt2DConfig.cmake)
    # at this point  WXART2D_LIBRARIES WXART2D_INCLUDE_DIRS etc. are set .
ENDIF( WXART2D_DIR_SHARE )

# handle the QUIETLY and REQUIRED arguments and set WXART2D_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( wxArt2D DEFAULT_MSG  WXART2D_LIBRARIES  WXART2D_INCLUDE_DIRS)

MARK_AS_ADVANCED( WXART2D_DIR_SHARE WXART2D_LIBRARIES WXART2D_INCLUDE_DIRS WXART2D_LIBRARY_DIRS WXART2D_LIBRARIES_WXLIKE )

IF( WXART2D_FOUND)
    IF(NOT wxArt2D_FIND_QUIETLY)
        MESSAGE ( STATUS "WXART2D_DIR => ${WXART2D_DIR}" )
        MESSAGE ( STATUS "WXART2D_DIR_SHARE => ${WXART2D_DIR_SHARE}" )
        MESSAGE ( STATUS "WXART2D_INCLUDE_DIRS => ${WXART2D_INCLUDE_DIRS}" )
        MESSAGE ( STATUS "WXART2D_LIBRARIES => ${WXART2D_LIBRARIES}" )
        MESSAGE ( STATUS "WXART2D_LIBRARY_DIRS => ${WXART2D_LIBRARY_DIRS}" )
        MESSAGE ( STATUS "WXART2D_LIBRARIES_WXLIKE => ${WXART2D_LIBRARIES_WXLIKE}" )
    ENDIF()
ELSE( WXART2D_FOUND)
    IF(NOT wxArt2D_FIND_QUIETLY)
        IF(wxArt2D_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "WXART2D required, please specify its location with WXART2D_DIR.")
        ELSE()
            MESSAGE(STATUS "WXART2D was not found.")
        ENDIF()
    ENDIF()
ENDIF( WXART2D_FOUND)

