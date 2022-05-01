##################################################
# Name:        CMakeLists.txt
# Purpose:     Input file for CMake
# Author:      Klaas Holwerda
# Licence:     wxWindows licence   # RCS Id:     $Id: CMakeLists.txt,v 1.184 2009/10/04 19:31:09 titato Exp $
##################################################
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

PROJECT( wxdocview )

if ( NOT THIRDPARTY_PATH )
    set( THIRDPARTY_PATH ${wxdocview_SOURCE_DIR}/../../thirdparty )
endif ( NOT THIRDPARTY_PATH ) 
if ( NOT THIRDPARTY_PATH_BIN )
    set( THIRDPARTY_PATH_BIN ${wxdocview_BINARY_DIR}/../../thirdparty )
endif ( NOT THIRDPARTY_PATH_BIN ) 

SET( wxDocview_VERSION "1.2.0" )
SET( wxDocview_MAJOR_VERSION 1 )
SET( wxDocview_MINOR_VERSION 2 )
SET( wxDocview_BUILD_VERSION 0 )

SET( AsSUBDIR 1 )
IF ( AsSUBDIR )
cmake_minimum_required(VERSION 2.8.12)

#set( XGETTEXT_PATH "c:/data/tools/freetypewin32/bin" )

INCLUDE( FindGettext )

if ( NOT GETTEXT_FOUND )
   MESSAGE( ERROR "GetText not found")
else ()   
   set( XGETTEXT_CMD xgettext )
   set( _xgettext_option_list -C -n -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o )
endif ()


# additional are modified Find routines
SET ( CMAKE_MODULE_PATH "${wxdocview_SOURCE_DIR}/share/wxdocview" )

SET( CMAKE_SUPRESS_REGENERATION 1 )
#SET( CMAKE_DEBUG_POSTFIX "d" )

if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )
    option( wxDocview_USE_STATIC_RUNTIME "Enable Static linking of VC runtime" OFF )
endif ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )

if ( wxDocview_USE_STATIC_RUNTIME )
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
       if(${flag_var} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
          set( ${flag_var} ${${flag_var}} CACHE STRING "MT like" FORCE )     
       endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
else ( wxDocview_USE_STATIC_RUNTIME )
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
       if(${flag_var} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
          set( ${flag_var} ${${flag_var}} CACHE STRING "MD like" FORCE )     
       endif(${flag_var} MATCHES "/MT")
    endforeach(flag_var)
endif ( wxDocview_USE_STATIC_RUNTIME )

#MESSAGE( "generator: ${CMAKE_GENERATOR}")

##################################################
# reset variables that are set incrementally
##################################################

SET( wxDocview_FLAGS "" CACHE STRING "" FORCE )
SET( wxDocview_INCLUDE_DIRS "" )
SET( wxDocview_INCLUDE_DIRS_DEPENDS "" )
SET( wxDocview_INCLUDE_DIRS_AFTER_INSTALL "" )
SET( wxDocview_LIBRARY_DIRS "" )
SET( wxDocview_LIBRARY_DIRS_DEPENDS "" )
SET( wxDocview_LIBRARIES "" )
SET( wxDocview_THIRDPARTY_LIBRARIES "" )
SET( wxDocview_LIBRARIES_WXLIKE "" CACHE STRING "package libraries" FORCE )
SET( wxDocview_LIBRARY_LIST_WXLIKE "" CACHE STRING "package libraries" FORCE )

IF ( NOT CMAKE_INSTALL_PREFIX )
    SET( CMAKE_INSTALL_PREFIX "${wxdocview_BINARY_DIR}" )
    MESSAGE( STATUS ${CMAKE_INSTALL_PREFIX} )
ENDIF ( NOT CMAKE_INSTALL_PREFIX )

##################################################
# include macro definitions
##################################################

INCLUDE( ${wxdocview_SOURCE_DIR}/share/wxdocview/CheckMSVC.cmake )
INCLUDE( ${wxdocview_SOURCE_DIR}/share/wxdocview/wxdocviewMacros.cmake)

##################################################
# Set all includes, flags, libraries, related to wxWidgets
##################################################

#force errors is something not found
SET( wxWidgets_FIND_REQUIRED TRUE )

IF( NOT WXWXINDOWS_CUSTOM )
    SET ( CMAKE_MODULE_PATH "${wxdocview_SOURCE_DIR}/share/wxwidgets" )
    find_package( wxWidgets REQUIRED )
    #INCLUDE(  ${wxdocview_SOURCE_DIR}/share/wxwidgets/FindwxWidgets.cmake)
    IF(wxWidgets_FOUND)
        include( ${wxdocview_SOURCE_DIR}/share/wxwidgets/MacroswxWidgets.cmake )

        DOCVIEW_ADD_CXXFLAGS( "${wxWidgets_CXX_FLAGS}" )
        DOCVIEW_ADD_DEFINITIONS( "${wxWidgets_DEFINITIONS}" )
        DOCVIEW_INCLUDE_DIRECTORIES_DEPENDS( "${wxWidgets_INCLUDE_DIRS}" )
        DOCVIEW_LINK_DIRECTORIES_DEPENDS( "${wxWidgets_LIBRARY_DIRS}" ) 
        DOCVIEW_THIRDPARTY_LIBRARIES( "${wxWidgets_LIBRARIES}" BACK )
    ELSE(wxWidgets_FOUND)
        message( "wxWidgets not found, but is needed")
    ENDIF(wxWidgets_FOUND)
ELSE( NOT WXWXINDOWS_CUSTOM )
    SEPARATE_ARGUMENTS(wxWidgets_DEFINITIONS)
    SEPARATE_ARGUMENTS(wxWidgets_INCLUDE_DIRS)
    SEPARATE_ARGUMENTS(wxWidgets_LIB_DIR)
    SEPARATE_ARGUMENTS(wxWidgets_LIBRARIES)
ENDIF( NOT WXWXINDOWS_CUSTOM )

IF( wxWidgets_USE_DEBUG )
    SET( PFDEBUG "d" )
ENDIF( wxWidgets_USE_DEBUG )

IF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
    IF( CYGWIN OR MINGW )
        IF( CYGWIN )
            SET( wxWidgets_DEFINITIONS ${wxWidgets_DEFINITIONS} -D__GNUWIN32__  -O2 -D_WIN32_IE=0x400 -MMD -Wall  )
        ENDIF( CYGWIN )
    ELSE( CYGWIN OR MINGW )
        # just in case `gtk-config --cflags`does not work
        # SET( wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS}  /usr/include/gtk-2.0 /usr/include/gtk-2.0/include /usr/lib/gtk-2.0/include /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/pango-1.0 /usr/include/atk-1.0 )
        # What about FindGTK.cmake? and what if somebody uses wxMotif?
        
        IF ( ${wxWidgets_PORT} STREQUAL "gtk2"  )
            SET( wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS} `pkg-config gtk+-2.0 --cflags`" )
        ELSE ( ${wxWidgets_PORT} STREQUAL "gtk2" )
            SET( wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS} `gtk-config --cflags`" )
        ENDIF ( ${wxWidgets_PORT} STREQUAL "gtk2" )
        
        #find Xwindows
        INCLUDE( ${CMAKE_ROOT}/Modules/FindX11.cmake )
        SET( wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS}  ${X11_INCLUDE_DIR} )
    ENDIF( CYGWIN OR MINGW )              
ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )

##################################################
# include macro definitions
##################################################

IF( $wxWidgets_FIND_STYLE STREQUAL "UNIX_STYLE_FIND" )
    INCLUDE( ${wxdocview_SOURCE_DIR}/share/wxwidgets/FindPkgConfig.cmake )

    PKGCONFIG("gtk+-2.0 >= 2.2.0")
    IF(PKGCONFIG_FOUND)
      IF(CMAKE_PKGCONFIG_C_FLAGS)
        SET( GTK2_FOUND  "TRUE" )
        #do something with ${PKGCONFIG_LIBRARIES}
      ENDIF(CMAKE_PKGCONFIG_C_FLAGS)
    ELSE(PKGCONFIG_FOUND)
        SET( GTK2_FOUND  "" )
    ENDIF(PKGCONFIG_FOUND)
ENDIF( $wxWidgets_FIND_STYLE STREQUAL "UNIX_STYLE_FIND" )

##################################################
# module options
##################################################  
      
OPTION( wxDocview_MAKE_SHARED "Make shared versions (.so) of wxDocview libraries" OFF )
OPTION( wxDocview_USE_LUA "Enable wxLua usage" OFF )
OPTION( wxDocview_USE_PACK "Package wxDocview" OFF)

# module sample compilation options. If wxDocview_COMP_SAMP is ON all samples
# will be compiled. If not, you can turn on the separate samples below.
OPTION( wxDocview_COMP_SAMP_DOCVIEW "Compile docview module samples" OFF )
OPTION( wxDocview_COMP_SAMP_FREQUENT "Compile most frequently used samples" ON )
OPTION( wxDocview_COMP_SAMP "Compile all module samples" ON )
OPTION( WXART2D_USE_CVGIO "Enable Cvg format module" ON )

MARK_AS_ADVANCED( 
wxDocview_COMP_SAMP_DOCVIEW
wxDocview_COMP_SAMP_FREQUENT
)

##################################################
# option dependancies
##################################################

#choose the type of output libraries (shared static)
IF ( wxDocview_MAKE_SHARED )
    SET( BUILD_SHARED_LIBS:BOOL ON )
ELSE( wxDocview_MAKE_SHARED )
    SET( BUILD_SHARED_LIBS:BOOL OFF )
ENDIF( wxDocview_MAKE_SHARED )

IF ( wxDocview_COMP_SAMP )
    SET( wxDocview_COMP_SAMP_DOCVIEW ON )
ENDIF ( wxDocview_COMP_SAMP )

IF ( WXART2D_USE_CVGIO )
    SET( wxDocview_USE_IRRXML 0 )
    SET( wxDocview_USE_EXPAT 1 )
    SET( WXART2D_USE_XMLPARSE 1 )
ENDIF( WXART2D_USE_CVGIO )

##################################################
# Check existance of some? directories
##################################################

IF ( EXISTS ${wxdocview_SOURCE_DIR}/../wxartbase/xmlparse )
ELSE ( EXISTS ${wxdocview_SOURCE_DIR}../wxartbase/xmlparse )
    SET( WXART2D_USE_XMLPARSE 0 )
ENDIF ( EXISTS ${wxdocview_SOURCE_DIR}/../wxartbase/xmlparse )

IF ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
    SET( wxDocview_USE_DOCVIEW 1 )
ELSE ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
    SET( wxDocview_USE_DOCVIEW 0 )
ENDIF ( EXISTS ${wxdocview_SOURCE_DIR}/docview )

##################################################
# output path definitions
##################################################

SET (LIBRARY_OUTPUT_PATH ${wxdocview_BINARY_DIR}/lib )
#SET (LIBRARY_OUTPUT_PATH ${wxdocview_BINARY_DIR}/lib/ CACHE PATH "Single output directory for building all libraries." FORCE )
#SET (EXECUTABLE_OUTPUT_PATH ${wxdocview_BINARY_DIR}/exe/ CACHE PATH "Single output directory for building all executables.")
#MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

# The next before any other LINK_DIRECTORIES, so after install of wxdocview files,
# a new compile will still take them from here and not form the install directory 
LINK_DIRECTORIES( ${wxdocview_BINARY_DIR}/lib )

##################################################
# Set all includes, flags, libraries
# Please only use the DOCVIEW_ functions to add options in this section,
##################################################

#the place for wxdocview.h which was generated.
INCLUDE_DIRECTORIES( ${wxdocview_BINARY_DIR}/include )

#some files like docviewprivate.h from here from build tree.
DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_SOURCE_DIR}/include )
DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_SOURCE_DIR}/../wxartbase/include )

##################################################
# linking of libraries defined at top level, else order of modules its samples 
# and what is linked at that stage, might cause problems 
##################################################

#
# STLPort support
#
SEPARATE_ARGUMENTS(wxDocview_STLPORT_DEFINES)
SEPARATE_ARGUMENTS(wxDocview_STLPORT_INCLUDES)
SEPARATE_ARGUMENTS(wxDocview_STLPORT_LINKDIRS)
SEPARATE_ARGUMENTS(wxDocview_STLPORT_LIBS)

DOCVIEW_ADD_DEFINITIONS( "${wxDocview_STLPORT_DEFINES}" )
DOCVIEW_INCLUDE_DIRECTORIES_DEPENDS( "${wxDocview_STLPORT_INCLUDES}" )
DOCVIEW_LINK_DIRECTORIES_DEPENDS(  "${wxDocview_STLPORT_LINKDIRS}" )
DOCVIEW_LINK_LIBRARIES( "${wxDocview_STLPORT_LIBS}" FRONT )

SET(wxDocview_FLAGS     "${wxDocview_FLAGS};${wxDocview_STLPORT_DEFINES}")

# set precompiled headers if wanted/possible
SET_PRECOMPILED_HEADER_FLAGS()
# set some flags to prevent mixing up debug wxWidgets and release libraries or visa versa
SET_WX_SYNCLIBRARY_FLAGS()

##################################################
# Set all includes, flags, libraries, related to expat
##################################################

if( wxDocview_USE_IRRXML )
    set( CMAKE_MODULE_PATH ${THIRDPARTY_PATH_BIN}/irrxml-1.2/share/irrxml )
    set( Irrxml_INSIDE_BUILD 1)
    if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( STATUS "searching Irrxml via: ${CMAKE_MODULE_PATH}" )
    else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( ERROR "Module Path for Irrxml not set or Irrxml not compiled in: ${CMAKE_MODULE_PATH}" )    
    endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
    set( Irrxml_DIR ${THIRDPARTY_PATH_BIN}/irrxml-1.2 )
    set( wxDocview_USE_IRRXML 0 )
    find_package( Irrxml )
    if( IRRXML_FOUND )
    	MESSAGE( STATUS "Irrxml was found for wxArt2D package" )
        #include(${Irrxml_USE_FILE})
        DOCVIEW_ADD_DEFINITIONS( "${Irrxml_DEFINITIONS}" )
        DOCVIEW_INCLUDE_DIRECTORIES_DEPENDS( "${Irrxml_INCLUDE_DIRS}" )
        DOCVIEW_THIRDPARTY_LIBRARIES( "${Irrxml_LIBRARIES}" FRONT )
        DOCVIEW_LINK_DIRECTORIES_DEPENDS( "${Irrxml_LIBRARY_DIRS}" )
        set( wxDocview_USE_IRRXML 1 )
    else( IRRXML_FOUND )
        MESSAGE( STATUS "Could not find existing Irrxml library" )
    endif( IRRXML_FOUND )
endif( wxDocview_USE_IRRXML )

# Please keep expat, xmlparse and general as the last three, since mingw is very picky about linking order

# Test if expat exists and set all includes, flags, libraries, related to expat
IF( wxDocview_USE_EXPAT )
    IF(NOT EXPAT_CUSTOM)
        INCLUDE( ${wxdocview_SOURCE_DIR}/share/wxwidgets/FindExpat.cmake )
    ELSE(NOT EXPAT_CUSTOM)
        SEPARATE_ARGUMENTS(EXPAT_INCLUDE_DIR)
        SEPARATE_ARGUMENTS(EXPAT_LINK_DIR)
        SEPARATE_ARGUMENTS(EXPAT_LIBRARIES)
    ENDIF(NOT EXPAT_CUSTOM)

    IF( EXPAT_FOUND )
		STRING( REGEX REPLACE "\\\\" "/"  EXPAT_INCLUDE_DIR "${EXPAT_INCLUDE_DIR}" )
        DOCVIEW_INCLUDE_DIRECTORIES_DEPENDS( "${EXPAT_INCLUDE_DIR}" )
        DOCVIEW_LINK_DIRECTORIES_DEPENDS( "${EXPAT_LINK_DIR}" )
        DOCVIEW_THIRDPARTY_LIBRARIES( "${EXPAT_LIBRARIES}" BACK )
    ELSE( EXPAT_FOUND )
        #MESSAGE( ERROR "Could not find existing expat library or include file" )
    ENDIF( EXPAT_FOUND )
ENDIF( wxDocview_USE_EXPAT )

##################################################
# Include subdirectories
##################################################

IF( NOT wxDocview_MAKE_SHARED )
     DOCVIEW_LINK_LIBRARIES( "${wxDocview_THIRDPARTY_LIBRARIES}" BACK )
ELSE( NOT wxDocview_MAKE_SHARED )
    
ENDIF( NOT wxDocview_MAKE_SHARED )

MACRO( ADD_MODULE modulesname )
    IF ( EXISTS ${wxdocview_SOURCE_DIR}/${modulesname} )
        add_subdirectory( ${modulesname} )
    ENDIF ( EXISTS ${wxdocview_SOURCE_DIR}/${modulesname} )
ENDMACRO( ADD_MODULE modulesname )

#the place for wxdocview.h which was generated.
INCLUDE_DIRECTORIES( ../wxartbase/include )
INCLUDE_DIRECTORIES( ${wxdocview_BINARY_DIR}/wxartbase/include ) 

DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_BINARY_DIR}/wxartbase/include ) 

add_subdirectory( ../wxartbase ${wxdocview_BINARY_DIR}/wxartbase )
   
ADD_MODULE( docview )

##################################################
# Set all includes, flags, libraries
##################################################

SET( wxDocview_LIBRARIES_WXLIKE ${wxDocview_LIBRARIES_WXLIKE} CACHE STRING "package libraries" FORCE )
SET( wxDocview_FLAGS ${wxDocview_FLAGS} CACHE STRING "package flags" FORCE )
SET( wxDocview_INCLUDE_DIRS ${wxDocview_INCLUDE_DIRS} CACHE STRING "package libs include paths" FORCE )
SET( wxDocview_INCLUDE_DIRS_DEPENDS ${wxDocview_INCLUDE_DIRS_DEPENDS} CACHE STRING "package libs include paths dependencies" FORCE )
SET( wxDocview_LIBRARY_DIRS ${wxDocview_LIBRARY_DIRS} CACHE STRING "package libs directory paths" FORCE )
SET( wxDocview_LIBRARY_DIRS_DEPENDS ${wxDocview_LIBRARY_DIRS_DEPENDS} CACHE STRING "external package libs directory paths" FORCE )
SET( wxDocview_LIBRARIES ${wxDocview_LIBRARIES} CACHE STRING "package libraries" FORCE )
SET( wxDocview_THIRDPARTY_LIBRARIES ${wxDocview_THIRDPARTY_LIBRARIES} CACHE STRING "package libs directory path" FORCE )

MARK_AS_ADVANCED( 
wxDocview_LIBRARIES_WXLIKE 
wxDocview_FLAGS 
wxDocview_INCLUDE_DIRS 
wxDocview_INCLUDE_DIRS_DEPENDS
wxDocview_LIBRARY_DIRS
wxDocview_LIBRARY_DIRS_DEPENDS
wxDocview_LIBRARIES 
wxDocview_THIRDPARTY_LIBRARIES
)


#MESSAGE( "all wxlike wxart2d libs: ${wxDocview_LIBRARIES_WXLIKE}")
#MESSAGE( "all flags: ${wxDocview_FLAGS}")
#MESSAGE( "all includes: ${wxDocview_INCLUDE_DIRS}")
#MESSAGE( "external lib dirs: ${wxDocview_LIBRARY_DIRS_DEPENDS}")
#MESSAGE( "all libs: ${wxDocview_LIBRARIES}")

##################################################
# Generate docview-config files for both cmake and unix
##################################################

add_subdirectory( share/wxdocview )

##################################################
# other subdirectories
##################################################

add_subdirectory( include )
add_subdirectory( doc )

add_subdirectory( samples )


if ( EXISTS ${wxdocview_SOURCE_DIR}/myapp )
    include( myapp/myproject.cmake )
    add_subdirectory( myapp )
endif ( EXISTS ${wxdocview_SOURCE_DIR}/myapp )


export( TARGETS FILE "${CMAKE_CURRENT_BINARY_DIR}/dependencies_docview.cmake" )

#-------------------------------------------------------------------
# Build a CPack installer if CPack is available and this is a build 
IF ( wxDocview_USE_PACK )    
    IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wxDocview - Vector Graphics")
        SET(CPACK_PACKAGE_VENDOR "wxDocview")
        #SET(CPACK_PACKAGE_VERSION "2.5.0")
        SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/readme.txt")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wxDocview framework for wxWidgets")
        #SET(CPACK_RESOURCE_FILE_README "/home/andy/vtk/CMake/Templates/CPack.GenericDescription.txt")
        #SET(CPACK_RESOURCE_FILE_WELCOME "/home/andy/vtk/CMake/Templates/CPack.GenericWelcome.txt")
        SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/licence.txt")
        SET(CPACK_PACKAGE_VERSION_MAJOR "${wxDocview_MAJOR_VERSION}")
        SET(CPACK_PACKAGE_VERSION_MINOR "${wxDocview_MINOR_VERSION}")
        SET(CPACK_PACKAGE_VERSION_PATCH "${wxDocview_BUILD_VERSION}")
        SET(CPACK_PACKAGE_INSTALL_DIRECTORY "DOCVIEW_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
        SET(CPACK_PACKAGE_FILE_NAME  
            "wxdocview-${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
        SET(CPACK_SOURCE_PACKAGE_FILE_NAME 
            "wxdocview-dev-${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
        SET(CPACK_PACKAGE_EXECUTABLES
        "wxdocview" "DOCVIEW"
        )
        SET(CPACK_SOURCE_STRIP_FILES "")
        SET(CPACK_STRIP_FILES "bin/ccmake;bin/cmake;bin/cpack;bin/ctest")
        SET(CPACK_PACKAGE_CONTACT "http://www.wxart2d.org" )
        SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Klaas Holwerda" )
 
        IF(WIN32 AND NOT UNIX )
            SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} wxDocview library")
            SET(CPACK_NSIS_HELP_LINK "http://www.wxart2d.org")
            SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.wxart2d.org")
            SET(CPACK_NSIS_CONTACT "http://www.wxart2d.org")
        ENDIF(WIN32 AND NOT UNIX )

        INCLUDE(CPack)
    ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")   
ENDIF ( wxDocview_USE_PACK )


ELSE ( AsSUBDIR )
	SET( wxDocview_FLAGS "" CACHE STRING "" FORCE )
	SET( wxDocview_INCLUDE_DIRS "" )
	SET( wxDocview_INCLUDE_DIRS_DEPENDS "" )
	SET( wxDocview_INCLUDE_DIRS_AFTER_INSTALL "" )
	SET( wxDocview_LIBRARY_DIRS "" )
	SET( wxDocview_LIBRARY_DIRS_DEPENDS "" )
	SET( wxDocview_LIBRARIES "" )
	SET( wxDocview_LIBRARIES_WXLIKE "" )

    INCLUDE( ${wxdocview_SOURCE_DIR}/share/wxdocview/wxdocviewMacros.cmake)

    IF ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
        SET( wxDocview_USE_DOCVIEW 1 )
    ELSE ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
        SET( wxDocview_USE_DOCVIEW 0 )
    ENDIF ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
    
    OPTION( WXART2D_USE_CVGIO "Enable Cvg format module" ON )
    IF ( WXART2D_USE_CVGIO )
        SET( wxDocview_USE_EXPAT 1 )
        SET( WXART2D_USE_XMLPARSE 1 )
    ENDIF( WXART2D_USE_CVGIO )

    ##################################################
    # Check existance of some? directories
    ##################################################

    IF ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
        SET( wxDocview_USE_DOCVIEW 1 )
    ELSE ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
        SET( wxDocview_USE_DOCVIEW 0 )
    ENDIF ( EXISTS ${wxdocview_SOURCE_DIR}/docview )
    
    #some files like docviewprivate.h from here from build tree.
    DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_SOURCE_DIR}/include )
    DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_SOURCE_DIR}/../wxartbase/include )
    DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_BINARY_DIR}/wxartbase/include ) 
    DOCVIEW_INCLUDE_DIRECTORIES( ${wxdocview_BINARY_DIR}/include )

    ##################################################
    # linking of libraries defined at top level, else order of modules its samples 
    # and what is linked at that stage, might cause problems 
    ##################################################
   
    MACRO( ADD_MODULE modulesname )
        IF ( EXISTS ${wxdocview_SOURCE_DIR}/${modulesname} )
            add_subdirectory( ${wxdocview_SOURCE_DIR}/${modulesname} )
        ENDIF ( EXISTS ${wxdocview_SOURCE_DIR}/${modulesname} )
    ENDMACRO( ADD_MODULE modulesname )

    #the place for wxdocview.h which was generated.
    INCLUDE_DIRECTORIES( ${wxdocview_BINARY_DIR}/include )
       
	##################################################
	# Set all includes, flags, libraries
	##################################################

	SET( wxDocview_FLAGS ${wxDocview_FLAGS} CACHE STRING "package flags" FORCE )
	SET( wxDocview_INCLUDE_DIRS ${wxDocview_INCLUDE_DIRS} CACHE STRING "package libs include paths" FORCE )
	SET( wxDocview_INCLUDE_DIRS_DEPENDS ${wxDocview_INCLUDE_DIRS_DEPENDS} CACHE STRING "package libs include paths dependencies" FORCE )
	SET( wxDocview_LIBRARY_DIRS ${wxDocview_LIBRARY_DIRS} CACHE STRING "package libs directory paths" FORCE )
	SET( wxDocview_LIBRARY_DIRS_DEPENDS ${wxDocview_LIBRARY_DIRS_DEPENDS} CACHE STRING "external package libs directory paths" FORCE )
	SET( wxDocview_LIBRARIES ${wxDocview_LIBRARIES} CACHE STRING "package libraries" FORCE )
	SET( wxDocview_THIRDPARTY_LIBRARIES ${wxDocview_THIRDPARTY_LIBRARIES} CACHE STRING "package libs directory path" FORCE )

	MARK_AS_ADVANCED( 
	wxDocview_LIBRARIES_WXLIKE 
	wxDocview_FLAGS 
	wxDocview_INCLUDE_DIRS 
	wxDocview_INCLUDE_DIRS_DEPENDS
	wxDocview_LIBRARY_DIRS
	wxDocview_LIBRARY_DIRS_DEPENDS
	wxDocview_LIBRARIES 
	wxDocview_THIRDPARTY_LIBRARIES
    wxDocview_LIBRARY_LIST_WXLIKE
	)


	#MESSAGE( "all wxlike wxart2d libs: ${wxDocview_LIBRARIES_WXLIKE}")
	#MESSAGE( "all flags: ${wxDocview_FLAGS}")
	#MESSAGE( "all includes: ${wxDocview_INCLUDE_DIRS}")
	#MESSAGE( "external lib dirs: ${wxDocview_LIBRARY_DIRS_DEPENDS}")
	#MESSAGE( "all libs: ${wxDocview_LIBRARIES}")
	   
    ADD_MODULE( docview ) 
    
    add_subdirectory( ${wxdocview_SOURCE_DIR}/include )
    add_subdirectory( ${wxdocview_SOURCE_DIR}/doc )

    add_subdirectory( ${wxdocview_SOURCE_DIR}/samples )
    
ENDIF ( AsSUBDIR )
