# \file wxluashare project
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.19 2009/06/23 18:56:07 titato Exp $

PROJECT( wxluashare )


# for non installed version we set the root of wxLua to $ENV{WXLUA}, unless empty
# in which case "${CMAKE_INSTALL_PREFIX}" is used to find it.
if(WIN32)
   	STRING( REGEX REPLACE "\\\\" "/" where_is_wxlua_root "$ENV{WXLUA}" )
    if ( "${where_is_wxlua_root}" STREQUAL "" )
   	    MESSAGE( FATAL_ERROR "WXLUA environment variable not set")
    endif ()
else()
    set( where_is_wxlua_root "$ENV{WXLUA}" )
    if ( "${where_is_wxlua_root}" STREQUAL "" )
	    set( where_is_wxlua_root "${CMAKE_INSTALL_PREFIX}" )
    endif ()
endif()

CONFIGURE_FILE( ${wxluashare_SOURCE_DIR}/wxLuaConfig.cmake.in
                ${wxluashare_BINARY_DIR}/wxLuaConfig.cmake
                @ONLY IMMEDIATE )

# for the installed version we set the root of wxstedit is always relative to the config script itself.
set( where_is_stedit_root "${CMAKE_INSTALL_PREFIX}" )

CONFIGURE_FILE( ${wxluashare_SOURCE_DIR}/wxLuaConfig.cmake.in
                ${wxluashare_BINARY_DIR}/wxLuaConfigInstalled.cmake
                @ONLY IMMEDIATE )
               
CONFIGURE_FILE( ${wxluashare_SOURCE_DIR}/UsewxLua.cmake.in
                ${wxluashare_BINARY_DIR}/UsewxLua.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${wxluashare_SOURCE_DIR}/FindwxLua.cmake
                ${wxluashare_BINARY_DIR}/FindwxLua.cmake
                @ONLY IMMEDIATE )

INSTALL( FILES ${wxluashare_BINARY_DIR}/wxLuaConfigInstalled.cmake DESTINATION "share/wxlua" RENAME wxLuaConfig.cmake )
INSTALL( FILES ${wxluashare_BINARY_DIR}/UsewxLua.cmake DESTINATION "share/wxlua" )              
INSTALL( FILES ${wxluashare_BINARY_DIR}/FindwxLua.cmake DESTINATION "share/wxlua" )              



