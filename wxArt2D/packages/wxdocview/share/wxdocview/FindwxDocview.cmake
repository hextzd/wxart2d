# - Locate wxDocview libraries
# This module defines
#  wxDocview_LIBRARIES, the library to link against
#  wxDocview_FOUND, if false, do not try to link to wxDocview
#  wxDocview_INCLUDE_DIRS, where to find headers.
#
# $wxDocview_DIR is an environment variable that would
# correspond to the install directory on e.g. windows.

# Created by Klaas Holwerda. 
# Search only if the location is not already known.
  
  SET( wxDocview_DIR_SHARE wxDocview_DIR_SHARE-NOTFOUND CACHE  INTERNAL "" )
  
  #
  # Look for build tree.
  #
  FIND_PATH(wxDocview_DIR_SHARE wxDocviewConfig.cmake
    # Look for an environment variable wxDocview_DIR.
    $ENV{wxDocview_DIR}/share/wxdocview
    ${wxDocview_DIR}/share/wxdocview

    # Look in search path.
    $ENV{PATH}
    
    NO_DEFAULT_PATH 
    DOC "wxDocview_DIR found"
  )

IF ( NOT wxDocview_INSIDE_BUILD )
  # Look for an installation
  FIND_PATH(wxDocview_DIR_SHARE wxDocviewConfig.cmake

    # Look in standard UNIX install locations.
    /usr/local/share/wxdocview
    /usr/share/wxdocview

    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\wxDocview\\wxdocview 1.0.1]/share/wxdocview"
 
    # Read from the CMakeSetup registry entries.  It is likely that
    # wxDocview will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]/share/wxdocview
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]/share/wxdocview

  )
ENDIF ()
  
# If wxDocview was found, load the configuration file to get the rest of the
# settings.
IF( wxDocview_DIR_SHARE )
    INCLUDE(${wxDocview_DIR_SHARE}/wxDocviewConfig.cmake)
    # at this point  wxDocview_LIBRARIES wxDocview_INCLUDE_DIRS etc. are set .
ENDIF( wxDocview_DIR_SHARE )

# handle the QUIETLY and REQUIRED arguments and set wxDocview_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( wxDocview DEFAULT_MSG  wxDocview_LIBRARIES  wxDocview_INCLUDE_DIRS wxDocview_LIBRARIES_WXLIKE )

MARK_AS_ADVANCED( wxDocview_DIR_SHARE wxDocview_LIBRARIES wxDocview_INCLUDE_DIRS wxDocview_LIBRARY_DIRS wxDocview_LIBRARIES_WXLIKE )

IF( WXDOCVIEW_FOUND)
    IF(NOT wxDocview_FIND_QUIETLY)
        MESSAGE ( STATUS "wxDocview_DIR => ${wxDocview_DIR}" )
        MESSAGE ( STATUS "wxDocview_DIR_SHARE => ${wxDocview_DIR_SHARE}" )
        MESSAGE ( STATUS "wxDocview_INCLUDE_DIRS => ${wxDocview_INCLUDE_DIRS}" )
        MESSAGE ( STATUS "wxDocview_LIBRARIES => ${wxDocview_LIBRARIES}" )
        MESSAGE ( STATUS "wxDocview_LIBRARY_DIRS => ${wxDocview_LIBRARY_DIRS}" )
        MESSAGE ( STATUS "wxDocview_LIBRARIES_WXLIKE => ${wxDocview_LIBRARIES_WXLIKE}" )
    ENDIF(NOT wxDocview_FIND_QUIETLY)
ELSE( WXDOCVIEW_FOUND)
    IF(NOT wxDocview_FIND_QUIETLY)
        IF(wxDocview_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "wxDocview required, please specify its location with wxDocview_DIR.")
        ELSE(wxDocview_FIND_REQUIRED)
            MESSAGE(STATUS "wxDocview was not found.")
        ENDIF(wxDocview_FIND_REQUIRED)
    ENDIF(NOT wxDocview_FIND_QUIETLY)
ENDIF( WXDOCVIEW_FOUND)
