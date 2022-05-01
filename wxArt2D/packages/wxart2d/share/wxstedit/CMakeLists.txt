# \file wxsteditshare project
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.19 2009/06/23 18:56:07 titato Exp $

PROJECT( wxsteditshare )

# for non installed version we set the root of wxstedit to $ENV{WXSTEDIT}, unless empty
# in which case "${CMAKE_INSTALL_PREFIX}" is used to find it.
if(WIN32)
   	STRING( REGEX REPLACE "\\\\" "/" where_is_stedit_root "$ENV{WXSTEDIT}" )
    if ( "${where_is_stedit_root}" STREQUAL "" )
   	    MESSAGE( FATAL_ERROR "WXSTEDIT environment variable not set")
    endif ()
else()
    set( where_is_stedit_root "$ENV{WXSTEDIT}" )
    if ( "${where_is_stedit_root}" STREQUAL "" )
	    set( where_is_stedit_root "${CMAKE_INSTALL_PREFIX}" )
    endif ()
endif()

CONFIGURE_FILE( ${wxsteditshare_SOURCE_DIR}/wxSteditConfig.cmake.in
                ${wxsteditshare_BINARY_DIR}/wxSteditConfig.cmake
                @ONLY IMMEDIATE )

# for the installed version we set the root of wxstedit is always relative to the config script itself.
set( where_is_stedit_root "${CMAKE_INSTALL_PREFIX}" )

CONFIGURE_FILE( ${wxsteditshare_SOURCE_DIR}/wxSteditConfig.cmake.in
                ${wxsteditshare_BINARY_DIR}/wxSteditConfigInstalled.cmake
                @ONLY IMMEDIATE )
               
CONFIGURE_FILE( ${wxsteditshare_SOURCE_DIR}/UsewxStedit.cmake.in
                ${wxsteditshare_BINARY_DIR}/UsewxStedit.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${wxsteditshare_SOURCE_DIR}/FindwxStedit.cmake
                ${wxsteditshare_BINARY_DIR}/FindwxStedit.cmake
                @ONLY IMMEDIATE )

INSTALL( FILES ${wxsteditshare_BINARY_DIR}/wxSteditConfigInstalled.cmake DESTINATION "share/wxstedit" RENAME wxSteditConfig.cmake )
INSTALL( FILES ${wxsteditshare_BINARY_DIR}/UsewxStedit.cmake DESTINATION "share/wxstedit" )              
INSTALL( FILES ${wxsteditshare_BINARY_DIR}/FindwxStedit.cmake DESTINATION "share/wxstedit" )              



