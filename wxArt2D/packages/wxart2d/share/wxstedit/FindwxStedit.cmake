# - Locate wxStedit libraries
# This module defines
#  wxStedit_LIBRARIES, the library to link against
#  wxStedit_FOUND, if false, do not try to link to wxStedit
#  wxStedit_INCLUDE_DIRS, where to find headers.
#
# $wxStedit_DIR is an environment variable that would
# correspond to the install directory on e.g. windows.

# Created by Klaas Holwerda. 
# Search only if the location is not already known.
  
  SET( wxStedit_CONFIG wxStedit_CONFIG-NOTFOUND CACHE  INTERNAL "" )

  #
  # Look for build tree.
  #
  FIND_PATH(wxStedit_CONFIG wxSteditConfig.cmake
    # Look for an environment variable wxStedit_DIR.
    "@wxart2d_BINARY_DIR@/share/wxstedit"
    $ENV{wxStedit_DIR}/share/wxstedit
    ${wxStedit_DIR}/share/wxstedit

    # Look in search path.
    $ENV{PATH}
    
    NO_DEFAULT_PATH 
    DOC "wxStedit_DIR found"
  )

  # Look for an installation
  FIND_PATH(wxStedit_CONFIG wxSteditConfig.cmake

    # Look in standard UNIX install locations.
    /usr/local/share/wxstedit
    /usr/share/wxstedit

    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\wxStedit\\wxstedit 1.0.1]/share/wxstedit"
 
    # Read from the CMakeSetup registry entries.  It is likely that
    # wxStedit will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]/share/wxstedit
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]/share/wxstedit

  )
  
# If wxStedit was found, load the configuration file to get the rest of the
# settings.
IF( wxStedit_CONFIG )
    INCLUDE(${wxStedit_CONFIG}/wxSteditConfig.cmake)
    # at this point  wxStedit_LIBRARIES wxStedit_INCLUDE_DIRS etc. are set .
	set( wxStedit_CONFIG ${wxStedit_CONFIG}/wxSteditConfig.cmake )
ENDIF( wxStedit_CONFIG )

# handle the QUIETLY and REQUIRED arguments and set wxStedit_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
if ( wxWidgets_RELEASE MATCHES "[0-2].[0-8].*" )
    FIND_PACKAGE_HANDLE_STANDARD_ARGS( wxStedit DEFAULT_MSG  wxStedit_LIBRARIES  wxStedit_INCLUDE_DIRS wxStedit_LIBRARIES_WXLIKE )
else ()
    FIND_PACKAGE_HANDLE_STANDARD_ARGS( wxStedit DEFAULT_MSG  wxStedit_LIBRARIES  wxStedit_INCLUDE_DIRS )
endif()

MARK_AS_ADVANCED( wxStedit_CONFIG wxStedit_LIBRARIES wxStedit_INCLUDE_DIRS wxStedit_LIBRARY_DIRS wxStedit_LIBRARIES_WXLIKE )
set( wxStedit_FOUND ${WXSTEDIT_FOUND} )

IF( NOT WXSTEDIT_FOUND)
    IF(NOT wxStedit_FIND_QUIETLY)
        IF(wxStedit_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "wxStedit required, please specify its location with wxStedit_DIR.")
        ELSE(wxStedit_FIND_REQUIRED)
            MESSAGE(STATUS "wxStedit was not found.")
        ENDIF(wxStedit_FIND_REQUIRED)
    ENDIF(NOT wxStedit_FIND_QUIETLY)
ENDIF()
