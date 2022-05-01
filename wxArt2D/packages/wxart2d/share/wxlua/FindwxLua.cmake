# - Locate wxLua libraries
# This module defines
#  wxLua_LIBRARIES, the library to link against
#  wxLua_FOUND, if false, do not try to link to wxLua
#  wxLua_INCLUDE_DIRS, where to find headers.
#
# $wxLua_DIR is an environment variable that would
# correspond to the install directory on e.g. windows.

# Created by Klaas Holwerda. 
# Search only if the location is not already known.
  
  SET( wxLua_CONFIG wxLua_CONFIG-NOTFOUND CACHE  INTERNAL "" )

  #
  # Look for build tree.
  #
  FIND_PATH(wxLua_CONFIG 
    #NAMES wxLua-config.cmake wxLuaConfig.cmake
    NAMES wxLuaConfig.cmake
    PATHS 
        # Look for an environment variable wxLua_DIR.
        # $ENV{wxLua_DIR}/build
        "@wxart2d_BINARY_DIR@/share/wxlua"
        $ENV{wxLua_DIR}/share/wxlua
        ${wxLua_DIR}/share/wxlua

        # Look in search path.
        $ENV{PATH}
    
    NO_DEFAULT_PATH 
    DOC "wxLua_DIR found"
  )

  # Look for an installation
  FIND_PATH(wxLua_CONFIG 
    NAMES wxLuaConfig.cmake
    PATHS 
        # Look in standard UNIX install locations.
        /usr/local/share/wxlua
        /usr/share/wxlua

        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\wxLua\\wxlua 1.0.1]/share/wxlua"
     
        # Read from the CMakeSetup registry entries.  It is likely that
        # wxLua will have been recently built.
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]/share/wxlua
        [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]/share/wxlua
  )
  
# If wxLua was found, load the configuration file to get the rest of the
# settings.
IF( wxLua_CONFIG )
    INCLUDE(${wxLua_CONFIG}/wxLuaConfig.cmake)
    # at this point  wxLua_LIBRARIES wxLua_INCLUDE_DIRS etc. are set .
	set( wxLua_CONFIG ${wxLua_CONFIG}/wxLuaConfig.cmake )
ENDIF( wxLua_CONFIG )

# handle the QUIETLY and REQUIRED arguments and set wxLua_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( 
    wxLua DEFAULT_MSG  
    wxLua_LIBRARIES  
    wxLua_INCLUDE_DIRS 
    wxLua_LIBRARIES_WXLIKE
    wxLua_OUTPUTLIBPREFIX
    wxLua_OUTPUTLIBPOSTFIX
)

MARK_AS_ADVANCED( wxLua_CONFIG wxLua_LIBRARIES wxLua_INCLUDE_DIRS wxLua_LIBRARY_DIRS wxLua_LIBRARIES_WXLIKE )
set( wxLua_FOUND ${WXLUA_FOUND} )

IF( NOT WXLUA_FOUND)
    IF(NOT wxLua_FIND_QUIETLY)
        IF(wxLua_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "wxLua required, please specify its location with wxLua_DIR.")
        ELSE(wxLua_FIND_REQUIRED)
            MESSAGE(STATUS "wxLua was not found.")
        ENDIF(wxLua_FIND_REQUIRED)
    ENDIF(NOT wxLua_FIND_QUIETLY)
ENDIF()

