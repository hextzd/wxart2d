##################################################
# Name:        CMakeLists.txt
# Purpose:     Input file for CMake
# Author:      Klaas Holwerda
# Licence:     wxWindows licence   # RCS Id:     $Id: CMakeLists.txt,v 1.184 2009/10/04 19:31:09 titato Exp $
##################################################

option( BUILD_USE_SOLUTION_FOLDERS "Use solution folders to group projects in MSVC Gui" ON )
set_property( GLOBAL PROPERTY USE_FOLDERS ${BUILD_USE_SOLUTION_FOLDERS} )

PROJECT( wxart2d )


message( STATUS "CMAKE_CL_64: ${CMAKE_CL_64}")


if ( NOT THIRDPARTY_PATH )
    set( THIRDPARTY_PATH ${wxart2d_SOURCE_DIR}/../../thirdparty )
endif ( NOT THIRDPARTY_PATH ) 
if ( NOT THIRDPARTY_PATH_BIN )
    set( THIRDPARTY_PATH_BIN ${wxart2d_BINARY_DIR}/../../thirdparty )
endif ( NOT THIRDPARTY_PATH_BIN ) 

set( WXART2D_VERSION "1.2.0" )
set( WXART2D_MAJOR_VERSION 1 )
set( WXART2D_MINOR_VERSION 2 )
set( WXART2D_BUILD_VERSION  )

cmake_minimum_required(VERSION 2.8.12)

INCLUDE( FindGettext )

if ( NOT GETTEXT_FOUND )
   MESSAGE( ERROR "GetText not found")
else ()   
   set( XGETTEXT_CMD xgettext )
   MESSAGE( STATUS "xgettext set to ${XGETTEXT_CMD}")
   set( _xgettext_option_list --from-code=UTF-8 -C -n -k_ -kwxPLURAL:1,2 -kwxTRANSLATE -o )
endif ()

# additional are modified Find routines
SET ( CMAKE_MODULE_PATH "${wxart2d_SOURCE_DIR}/share/wxwidgets" )

set( CMAKE_SUPRESS_REGENERATION 1 )
#set( CMAKE_DEBUG_POSTFIX "d" )

if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )
    option( WXART2D_USE_STATIC_RUNTIME "Enable Static linking of VC runtime" OFF )
endif ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )

if ( WXART2D_USE_STATIC_RUNTIME )
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
else ( WXART2D_USE_STATIC_RUNTIME )
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
endif ( WXART2D_USE_STATIC_RUNTIME )


#MESSAGE( "generator: ${CMAKE_GENERATOR}")

##################################################
# reset variables that are set incrementally
##################################################

set( WXART2D_FLAGS "" CACHE STRING "" FORCE )
set( WXART2D_INCLUDE_DIRS "" )
set( WXART2D_INCLUDE_DIRS_DEPENDS "" )
set( WXART2D_INCLUDE_DIRS_AFTER_INSTALL "" )
set( WXART2D_LIBRARY_DIRS "" )
set( WXART2D_LIBRARY_DIRS_DEPENDS "" )
set( WXART2D_LIBRARIES "" )
set( WXART2D_LIBRARIES_WXLIKE "" CACHE STRING "package libraries" FORCE )
set( WXART2D_THIRDPARTY_LIBRARIES "" CACHE STRING "package libs directory path" FORCE )
set( WXART2D_POT_FILES "" CACHE STRING "lang files" FORCE )
 
if ( NOT CMAKE_INSTALL_PREFIX )
    set( CMAKE_INSTALL_PREFIX "${wxart2d_BINARY_DIR}" )
    MESSAGE( STATUS ${CMAKE_INSTALL_PREFIX} )
endif ( NOT CMAKE_INSTALL_PREFIX )

##################################################
# include macro definitions
##################################################

INCLUDE( ${wxart2d_SOURCE_DIR}/share/wxwidgets/CheckMSVC.cmake )
INCLUDE( ${wxart2d_SOURCE_DIR}/share/wxart2d/wxart2dMacros.cmake)

##################################################
# set all includes, flags, libraries, related to wxWidgets
##################################################

#force errors if something not found
set( wxWidgets_FIND_REQUIRED TRUE )

option( WXART2D_USE_OPENGL "Use Opengl Drawing context" OFF)

if( NOT WXWXINDOWS_CUSTOM )
    SET ( CMAKE_MODULE_PATH "${wxart2d_SOURCE_DIR}/share/wxwidgets" )
    #When need OpenGl use find_package( wxWidgets REQUIRED COMPONENTS stc gl )
    IF(WXART2D_USE_OPENGL)
        find_package( wxWidgets REQUIRED COMPONENTS stc gl )
    ELSE(WXART2D_USE_OPENGL)
        find_package( wxWidgets REQUIRED COMPONENTS stc )
    ENDIF(WXART2D_USE_OPENGL)
    #INCLUDE(  ${wxart2d_SOURCE_DIR}/share/wxwidgets/FindwxWidgets.cmake)
    IF(wxWidgets_FOUND)
        include( ${wxart2d_SOURCE_DIR}/share/wxwidgets/MacroswxWidgets.cmake )

        A2D_ADD_DEFINITIONS( "${wxWidgets_DEFINITIONS}" )
        A2D_ADD_CXXFLAGS( "${wxWidgets_CXX_FLAGS}" )
        A2D_INCLUDE_DIRECTORIES_DEPENDS( "${wxWidgets_INCLUDE_DIRS}" )
        A2D_THIRDPARTY_LIBRARIES( "${wxWidgets_LIBRARIES}" BACK )
        A2D_LINK_DIRECTORIES_DEPENDS( "${wxWidgets_LIBRARY_DIRS}" )

        if( WXART2D_USE_GRAPHICS_CONTEXT )
            A2D_ADD_DEFINITIONS( -DwxUSE_GRAPHICS_CONTEXT=1 )
        endif( WXART2D_USE_GRAPHICS_CONTEXT )

    ELSE(wxWidgets_FOUND)
        message( "wxWidgets not found, but is needed")
    ENDIF(wxWidgets_FOUND)
else( NOT WXWXINDOWS_CUSTOM )
    separate_arguments(wxWidgets_DEFINITIONS)
    separate_arguments(wxWidgets_INCLUDE_DIRS)
    separate_arguments(wxWidgets_LIB_DIR)
    separate_arguments(wxWidgets_LIBRARIES)
endif( NOT WXWXINDOWS_CUSTOM )

if( wxWidgets_USE_DEBUG )
    set( PFDEBUG "d" )
endif( wxWidgets_USE_DEBUG )

if( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )
    if( CYGWIN OR MINGW )
        if( CYGWIN )
            set( wxWidgets_DEFINITIONS ${wxWidgets_DEFINITIONS} -D__GNUWIN32__  -O2 -D_WIN32_IE=0x400 -MMD -Wall  )
        endif( CYGWIN )
    else( CYGWIN OR MINGW )
        # just in case `gtk-config --cflags`does not work
        # set( wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS}  /usr/include/gtk-2.0 /usr/include/gtk-2.0/include /usr/lib/gtk-2.0/include /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/pango-1.0 /usr/include/atk-1.0 )
        # What about FindGTK.cmake? and what if somebody uses wxMotif?
        
        set( wxWidgets_pkgFlags "" )
        if ( ${wxWidgets_PORT} STREQUAL "gtk2"  )
            execute_process(
                COMMAND pkg-config gtk+-2.0 --cflags
                OUTPUT_VARIABLE wxWidgets_pkgFlags
                RESULT_VARIABLE RET
                OUTPUT_STRIP_TRAILING_WHITESPACE        
            )                                 
            A2D_ADD_DEFINITIONS( "${wxWidgets_pkgFlags}")
            #set( wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS} `pkg-config gtk+-2.0 --cflags`" )
        elseif ( ${wxWidgets_PORT} STREQUAL "gtk3"  )
            execute_process(
                COMMAND pkg-config gtk+-3.0 --cflags
                OUTPUT_VARIABLE wxWidgets_pkgFlags
                RESULT_VARIABLE RET
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE        
            )           
            A2D_ADD_DEFINITIONS( "${wxWidgets_pkgFlags}")
            #set( wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS} `pkg-config gtk+-3.0 --cflags`" )
        else ( ${wxWidgets_PORT} STREQUAL "gtk2" )
            A2D_ADD_DEFINITIONS( "`gtk-config --cflags`" )
            #set( wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS} `gtk-config --cflags`" )
        endif ( ${wxWidgets_PORT} STREQUAL "gtk2" )
        
        #find Xwindows
        include( ${CMAKE_ROOT}/Modules/FindX11.cmake )
        A2D_INCLUDE_DIRECTORIES_DEPENDS( "${X11_INCLUDE_DIR}" )
        #set( wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS}  ${X11_INCLUDE_DIR} )
    endif( CYGWIN OR MINGW )              
endif( ${wxWidgets_FIND_STYLE} STREQUAL "UNIX_STYLE_FIND" )

##################################################
# include macro definitions
##################################################

if( $wxWidgets_FIND_STYLE STREQUAL "UNIX_STYLE_FIND" )
    include( ${wxart2d_SOURCE_DIR}/share/wxwidgets/FindPkgConfig.cmake )

    PKGCONFIG("gtk+-2.0 >= 2.2.0")
    if(PKGCONFIG_FOUND)
      if(CMAKE_PKGCONFIG_C_FLAGS)
        set( GTK2_FOUND  "TRUE" )
        #do something with ${PKGCONFIG_LIBRARIES}
      endif(CMAKE_PKGCONFIG_C_FLAGS)
    else(PKGCONFIG_FOUND)
        set( GTK2_FOUND  "" )
    endif(PKGCONFIG_FOUND)
endif( $wxWidgets_FIND_STYLE STREQUAL "UNIX_STYLE_FIND" )

################################################## 
# check option Agg
##################################################

set( WXART2D_USE_AGGDRAWER_AVAILABLE OFF )
set( CMAKE_MODULE_PATH ${THIRDPARTY_PATH_BIN}/agg-2.4/bin )
set( Agg_INSIDE_BUILD 1)
if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
    set( Agg_DIR ${THIRDPARTY_PATH_BIN}/agg-2.4 )
    set( WXART2D_USE_AGG 0 )
    find_package( Agg QUIET )
    if( AGG_FOUND )
        set( WXART2D_USE_AGGDRAWER_AVAILABLE ON )
    else( AGG_FOUND )
        message( STATUS "dis-abled Agg since could not be found using: ${Agg_DIR}" )
    endif( AGG_FOUND )
else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
    message( STATUS "dis-abled Agg since could not be found using: ${CMAKE_MODULE_PATH}" )
endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )   

################################################## 
# check option Freetype
##################################################


set( WXART2D_USE_FREETYPE_AVAILABLE OFF )
set( FREETYPE_LIBRARY_DEBUG ${wxWidgets_USE_DEBUG} )
set( CMAKE_MODULE_PATH "${wxart2d_SOURCE_DIR}/share/wxwidgets" )
set( FREETYPE_DIR ${THIRDPARTY_PATH}/freetype-2.5.5 )
if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
    FIND_PACKAGE( Freetype )                    
    IF( FREETYPE_FOUND )
        set( WXART2D_USE_FREETYPE_AVAILABLE ON )
    ELSE( FREETYPE_FOUND )
        #message( STATUS "dis-abled Freetype since could not be found using: ${FREETYPE_DIR}" )
        message( FATAL_ERROR "Freetype could not be found using: ${FREETYPE_DIR}" )
    ENDIF( FREETYPE_FOUND )
else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
    #message( STATUS "dis-abled Freetype since could not be found using: ${CMAKE_MODULE_PATH}" )
    message( FATAL_ERROR "Freetype could not be found using: ${CMAKE_MODULE_PATH}" )
endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
       

##################################################
# module options
##################################################  
      
option( WXART2D_MAKE_SHARED "Make shared versions (.so) of wxArt2D libraries" OFF )
if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )
option( WXART2D_USE_GDIPLUSDRAWER "Enable GdiPlus Library GdiPlusDrawer" OFF )
endif ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )
# on windows needs gdiplus, on linux --enable-graphics_ctx
if( wxWidgets_USE_GDIPLUS )
    option( WXART2D_USE_GRAPHICS_CONTEXT "Graphics Context" ON )
else( wxWidgets_USE_GDIPLUS  )
    option( WXART2D_USE_GRAPHICS_CONTEXT "Graphics Context" OFF )
endif( wxWidgets_USE_GDIPLUS )
option( WXART2D_USE_SVGIO "Enable SVG format module" ON )
option( WXART2D_USE_GDSIO "Enable GDS format module" ON )
option( WXART2D_USE_KEYIO "Enable KEY format module" OFF )
option( WXART2D_USE_CANEXTOBJ "Extended canvas objects" ON )
option( WXART2D_USE_CURVES "Enable Curves module" ON )
option( WXART2D_USE_EDITOR "Enable Editor module (requires and uses wxdocview module)" ON )
option( WXART2D_USE_FREETYPE "Enable Freetype text module" ${WXART2D_USE_FREETYPE_AVAILABLE} )
option( WXART2D_USE_AGGDRAWER "Enable Antigrain Library AggDrawer" ${WXART2D_USE_AGGDRAWER_AVAILABLE} )
option( WXART2D_USE_KBOOL "Enable polygon boolean algorithm" ON )
option( WXART2D_USE_KBOOLNEW "choose new polygon boolean algorithm" OFF )
option( WXART2D_USE_LUA "Enable wxLua usage" OFF )
option( WXART2D_USE_PACK "Package wxArt2D" OFF)

# module sample compilation options. If WXART2D_COMP_SAMP is ON all samples
# will be compiled. If not, you can turn on the separate samples below.
option( WXART2D_COMP_SAMP_AGGDRAWER "Compile Antigrain Library AggDrawer samples" OFF )
option( WXART2D_COMP_SAMP_ARTBASE "Compile artbase samples" OFF )
option( WXART2D_COMP_SAMP_BOOLEAN "Compile polygon boolean algorithm samples" OFF )
option( WXART2D_COMP_SAMP_SVGIO "Compile SVG format module samples" OFF )
option( WXART2D_COMP_SAMP_GDSIO "Compile GDS format module samples" OFF )
option( WXART2D_COMP_SAMP_CANVAS "Compile canvas samples" OFF )
option( WXART2D_COMP_SAMP_CURVES "Compile curves module samples" OFF )
option( WXART2D_COMP_SAMP_EDITOR "Compile editor module samples" OFF )
option( WXART2D_COMP_SAMP_FREQUENT "Compile most frequently used samples" ON )
option( WXART2D_COMP_SAMP "Compile all module samples" ON )

mark_as_advanced( 
WXART2D_COMP_SAMP_AGGDRAWER 
WXART2D_COMP_SAMP_ARTBASE 
WXART2D_COMP_SAMP_BOOLEAN 
WXART2D_COMP_SAMP_SVGIO 
WXART2D_COMP_SAMP_GDSIO 
WXART2D_COMP_SAMP_CANVAS
WXART2D_COMP_SAMP_CURVES
WXART2D_COMP_SAMP_EDITOR
WXART2D_COMP_SAMP_DOCVIEW
WXART2D_COMP_SAMP_FREQUENT
)

set( WXART2D_USE_CANEXTOBJ ON )
#WXART2D_DEPENDENT_option( WXART2D_USE_CANEXTOBJ "Enable extended canvasobject module" ON
#                    "WXART2D_USE_EDITOR;WXART2D_USE_CURVES" OFF )
#by default CVG is on but if set off by user, it is switched on if WXART2D_USE_EDITOR is ON
#set( WXART2D_USE_CVGIO ON )
#WXART2D_DEPENDENT_option( WXART2D_USE_CVGIO "Enable Cvg format module" ON
#                    "WXART2D_USE_EDITOR" OFF )
option( WXART2D_USE_CVGIO "Enable Cvg format module" ON )
option( WXART2D_USE_XMLPARSE "Enable Xml parsing" ON )

##################################################
# option dependancies
##################################################

#choose the type of output libraries (shared static)
if ( WXART2D_MAKE_SHARED )
    set( BUILD_SHARED_LIBS:BOOL ON )
else( WXART2D_MAKE_SHARED )
    set( BUILD_SHARED_LIBS:BOOL OFF )
endif( WXART2D_MAKE_SHARED )

if ( WXART2D_USE_EDITOR )
    set( WXART2D_USE_TETO 1 )
else( WXART2D_USE_EDITOR )
    set( WXART2D_USE_TETO 0 )
endif( WXART2D_USE_EDITOR )

if ( WXART2D_COMP_SAMP )
    set( WXART2D_COMP_SAMP_AGGDRAWER ON )
    set( WXART2D_COMP_SAMP_ARTBASE ON )
    set( WXART2D_COMP_SAMP_BOOLEAN ON )
    set( WXART2D_COMP_SAMP_SVGIO ON )
    set( WXART2D_COMP_SAMP_GDSIO ON )
    set( WXART2D_COMP_SAMP_CANVAS ON )
    set( WXART2D_COMP_SAMP_CURVES ON )
    set( WXART2D_COMP_SAMP_EDITOR ON )
    set( WXART2D_COMP_SAMP_DOCVIEW ON )
endif ( WXART2D_COMP_SAMP )

if ( WXART2D_USE_CVGIO OR WXART2D_USE_SVGIO  )
    set( WXART2D_USE_EXPAT 1 )
    set( WXART2D_USE_XMLPARSE 1 )
endif(  WXART2D_USE_CVGIO OR WXART2D_USE_SVGIO )

##################################################
# Check existance of some? directories
##################################################

if ( EXISTS ${wxart2d_SOURCE_DIR}/artbase )
    set( WXART2D_USE_ARTBASE 1 )
else ( EXISTS ${wxart2d_SOURCE_DIR}/artbase )
    set( WXART2D_USE_ARTBASE 0 )
endif ( EXISTS ${wxart2d_SOURCE_DIR}/artbase )

if ( EXISTS ${wxart2d_SOURCE_DIR}/canvas )
    set( WXART2D_USE_CANVAS 1 )
else ( EXISTS ${wxart2d_SOURCE_DIR}/canvas )
    set( WXART2D_USE_CANVAS 0 )
endif ( EXISTS ${wxart2d_SOURCE_DIR}/canvas )

##################################################
# output path definitions
##################################################

set (LIBRARY_OUTPUT_PATH ${wxart2d_BINARY_DIR}/lib )
#set (LIBRARY_OUTPUT_PATH ${wxart2d_BINARY_DIR}/lib/ CACHE PATH "Single output directory for building all libraries." FORCE )
#set (EXECUTABLE_OUTPUT_PATH ${wxart2d_BINARY_DIR}/exe/ CACHE PATH "Single output directory for building all executables.")
#mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

# The next before any other LINK_DIRECTORIES, so after install of wxart2d files,
# a new compile will still take them from here and not form the install directory 
LINK_DIRECTORIES( ${wxart2d_BINARY_DIR}/lib )

##################################################
# set all includes, flags, libraries
# Please only use the A2D_ functions to add options in this section,
##################################################

#the place for wxart2d.h which was generated.
INCLUDE_DIRECTORIES( ${wxart2d_BINARY_DIR}/include )

if(WIN32)
    set( WXART2D_INCLUDE_ROOT include/wxart2d )
else(WIN32)
    set( WXART2D_INCLUDE_ROOT include/wxart2d )
endif(WIN32)

#some files like a2dprivate.h from here from build tree.
A2D_INCLUDE_DIRECTORIES( include )

##################################################
# linking of libraries defined at top level, else order of modules its samples 
# and what is linked at that stage, might cause problems 
##################################################

if( WXART2D_USE_KBOOL )
    if( WXART2D_USE_KBOOLNEW )
        set( CMAKE_MODULE_PATH ${THIRDPARTY_PATH_BIN}/kbooln/share/kbool )
    else()
        set( CMAKE_MODULE_PATH ${THIRDPARTY_PATH_BIN}/kbool/share/kbool )
    endif()    
    set( kbool_INSIDE_BUILD 1)
    if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( STATUS "searching kbool via: ${CMAKE_MODULE_PATH}" )
    else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( ERROR "Module Path for kbool not set or kbool not compiled in: ${CMAKE_MODULE_PATH}" )    
    endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
    if( WXART2D_USE_KBOOLNEW )
        set( kbool_DIR ${THIRDPARTY_PATH_BIN}/kbooln )
    else()
        set( kbool_DIR ${THIRDPARTY_PATH_BIN}/kbool )
    endif()    
    set( WXART2D_USE_KBOOL 0 )
    find_package( kbool )
    if( KBOOL_FOUND )
        MESSAGE( STATUS "Kbool was found for wxArt2D package" )
        #include(${Kbool_USE_FILE})
        A2D_ADD_DEFINITIONS( "${kbool_DEFINITIONS}" )
        A2D_INCLUDE_DIRECTORIES_DEPENDS( "${kbool_INCLUDE_DIRS}" )
        A2D_THIRDPARTY_LIBRARIES( "${kbool_LIBRARIES}" FRONT )
        A2D_LINK_DIRECTORIES_DEPENDS( "${kbool_LIBRARY_DIRS}" )
        set( WXART2D_USE_KBOOL 1 )
    else( KBOOL_FOUND )
        MESSAGE( STATUS "Could not find existing Kbool library" )
    endif( KBOOL_FOUND )
endif( WXART2D_USE_KBOOL )

# Test if antigrain exists and set all includes, flags, libraries, related to Antigrain

if( WXART2D_USE_AGGDRAWER )
    set( CMAKE_MODULE_PATH ${THIRDPARTY_PATH_BIN}/agg-2.4/bin )
    if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( STATUS "searching Agg via: ${CMAKE_MODULE_PATH}" )
    else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( ERROR "Module Path for Agg not set or Agg not compiled in: ${CMAKE_MODULE_PATH}" )    
    endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
    set( Agg_DIR ${THIRDPARTY_PATH_BIN}/agg-2.4 )
    set( WXART2D_USE_AGG 0 )
    find_package( Agg )
    if( AGG_FOUND )
        MESSAGE( STATUS "Agg was found for wxArt2D package" )
        #include(${Agg_USE_FILE})
        A2D_ADD_DEFINITIONS( "${Agg_DEFINITIONS}" )
        A2D_INCLUDE_DIRECTORIES_DEPENDS( "${Agg_INCLUDE_DIRS}" )
        A2D_THIRDPARTY_LIBRARIES( "${Agg_LIBRARIES}" FRONT )
        A2D_LINK_DIRECTORIES_DEPENDS( "${Agg_LIBRARY_DIRS}" )
        set( WXART2D_USE_AGG 1 )
    else( AGG_FOUND )
        MESSAGE( STATUS "Could not find existing AGG library" )
    endif( AGG_FOUND )
endif( WXART2D_USE_AGGDRAWER )

# Test if freetype exists and set all includes, flags, libraries, related to FreeType
if( WXART2D_USE_FREETYPE )
    set( CMAKE_MODULE_PATH "${wxart2d_SOURCE_DIR}/share/wxwidgets" )
    if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( STATUS "searching Freetype via: ${CMAKE_MODULE_PATH}" )
    else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
        message( ERROR "Module Path for Freetype not set or Freetype not compiled in: ${CMAKE_MODULE_PATH}" )    
    endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
    set( FREETYPE_DIR ${THIRDPARTY_PATH}/freetype-2.5.5 )
    FIND_PACKAGE( Freetype )                    
    IF( FREETYPE_FOUND )
        message( STATUS "FREETYPE Debug version: ${FREETYPE_LIBRARY_DEBUG}"  )
        A2D_INCLUDE_DIRECTORIES_DEPENDS( "${FREETYPE_INCLUDE_DIRS}" )
        A2D_THIRDPARTY_LIBRARIES( "${FREETYPE_LIBRARIES}" FRONT )
        A2D_LINK_DIRECTORIES_DEPENDS( "${FREETYPE_LINK_DIR}" )
    ELSE( FREETYPE_FOUND )
        MESSAGE(SEND_ERROR "freetype not found")
    ENDIF( FREETYPE_FOUND )
endif( WXART2D_USE_FREETYPE )

#
# STLPort support
#
separate_arguments(WXART2D_STLPORT_DEFINES)
separate_arguments(WXART2D_STLPORT_INCLUDES)
separate_arguments(WXART2D_STLPORT_LINKDIRS)
separate_arguments(WXART2D_STLPORT_LIBS)

A2D_ADD_DEFINITIONS( "${WXART2D_STLPORT_DEFINES}" )
A2D_INCLUDE_DIRECTORIES_DEPENDS( "${WXART2D_STLPORT_INCLUDES}" )
A2D_LINK_DIRECTORIES_DEPENDS(  "${WXART2D_STLPORT_LINKDIRS}" )
A2D_LINK_LIBRARIES( "${WXART2D_STLPORT_LIBS}" FRONT )

set(WXART2D_FLAGS     "${WXART2D_FLAGS};${WXART2D_STLPORT_DEFINES}")
A2D_INCLUDE_DIRECTORIES_DEPENDS( "${WXART2D_STLPORT_INCLUDES}" )
A2D_LINK_DIRECTORIES_DEPENDS( "${WXART2D_STLPORT_LINKDIRS}" )
A2D_THIRDPARTY_LIBRARIES( "${WXART2D_STLPORT_LIBS}" FRONT )

# set precompiled headers if wanted/possible
SET_PRECOMPILED_HEADER_FLAGS()
# set some flags to prevent mixing up debug wxWidgets and release libraries or visa versa
SET_WX_SYNCLIBRARY_FLAGS()

##################################################
# Set all includes, flags, libraries, related to expat
##################################################

# Please keep expat, xmlparse and general as the last three, since mingw is very picky about linking order

# Test if expat exists and set all includes, flags, libraries, related to expat
if( WXART2D_USE_EXPAT )
    if(NOT EXPAT_CUSTOM)
        include( ${wxart2d_SOURCE_DIR}/share/wxwidgets/FindExpat.cmake )
    else(NOT EXPAT_CUSTOM)
        separate_arguments(EXPAT_INCLUDE_DIR)
        separate_arguments(EXPAT_LINK_DIR)
        separate_arguments(EXPAT_LIBRARIES)
    endif(NOT EXPAT_CUSTOM)

    if( EXPAT_FOUND )
        STRING( REGEX REPLACE "\\\\" "/"  EXPAT_INCLUDE_DIR "${EXPAT_INCLUDE_DIR}" )
        A2D_INCLUDE_DIRECTORIES_DEPENDS( "${EXPAT_INCLUDE_DIR}" )
        A2D_LINK_DIRECTORIES_DEPENDS( "${EXPAT_LINK_DIR}" )
        A2D_THIRDPARTY_LIBRARIES( "${EXPAT_LIBRARIES}" BACK )
    else( EXPAT_FOUND )
        #MESSAGE( ERROR "Could not find existing expat library or include file" )
    endif( EXPAT_FOUND )
endif( WXART2D_USE_EXPAT )


##################################################
# configure wxlua and wxstedit search
##################################################

if( WXART2D_USE_LUA )
    if(WIN32)
        STRING( REGEX REPLACE "\\\\" "/" WXLUA_PATH "$ENV{WXLUA}" )
        if ( "${WXLUA_PATH}" STREQUAL "" )
            MESSAGE( FATAL_ERROR "WXLUA environment variable not set")
        endif ()
        set( wxLua_DIR ${WXLUA_PATH} )

           STRING( REGEX REPLACE "\\\\" "/" WXSTEDIT_PATH "$ENV{WXSTEDIT}" )
        if ( "${WXSTEDIT_PATH}" STREQUAL "" )
            #set( WXSTEDIT_PATH ${WXLUA_PATH}/modules/wxstedit )
               MESSAGE( FATAL_ERROR "WXSTEDIT environment variable not set")
        endif ()
        set( wxStedit_DIR ${WXSTEDIT_PATH} )
    else()

        set( CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX} )

        #set( wxLua_DIR "$ENV{WXLUA}" )
        set( wxLua_DIR "${wxart2d_BINARY_DIR}/share/wxlua" )

        set( wxStedit_DIR "$ENV{WXSTEDIT}" )
        set( wxStedit_DIR "${wxart2d_BINARY_DIR}/share/wxstedit" )
    endif()

    add_subdirectory( share/wxlua )
    add_subdirectory( share/wxstedit )

##################################################
# Set all includes, flags, libraries, related to wxlua
##################################################

    if ( 1 ) #wxWidgets_RELEASE MATCHES "[0-2].[0-8].*" )
        set( CMAKE_MODULE_PATH "${wxart2d_BINARY_DIR}/share/wxlua" )
        if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
            message( STATUS "searching wxLua via: ${CMAKE_MODULE_PATH}" )
        else()
            message( ERROR "Module Path for wxLua not set or wxLua not compiled in: ${CMAKE_MODULE_PATH}" )    
        endif()          
        find_package( wxLua REQUIRED 
        #CONFIGS wxLua-config.cmake
        #PATH_SUFFIXES  build
        )                    
        if( wxLua_FOUND )
            message( STATUS "wxLua config found at: ${wxLua_CONFIG}" )
            get_filename_component( wxLua_CONFIG_PATH "${wxLua_CONFIG}" PATH ABSOLUTE )
            #include( ${wxLua_CONFIG_PATH}/${wxLua_USE_FILE} )
            A2D_LINK_DIRECTORIES_DEPENDS( "${wxLua_LIBRARY_DIRS}" )
            A2D_THIRDPARTY_LIBRARIES( "${wxLua_LIBRARIES}" FRONT )
            A2D_INCLUDE_DIRECTORIES_DEPENDS( "${wxLua_INCLUDE_DIRS}" )
        else()
            MESSAGE(SEND_ERROR "wxLua not found")
        endif()

        set( CMAKE_MODULE_PATH "${wxart2d_BINARY_DIR}/share/wxstedit" )
        if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
            message( STATUS "searching wxStedit via: ${CMAKE_MODULE_PATH}" )
        else()
            message( ERROR "Module Path for wxStedit not set or wxStedit not compiled in: ${CMAKE_MODULE_PATH}" )    
        endif()          
        
        find_package( wxStedit REQUIRED )     
        if( wxStedit_FOUND )
            message( STATUS "wxStedit config found at: ${wxStedit_CONFIG}" )
            get_filename_component( wxStedit_CONFIG_PATH "${wxStedit_CONFIG}" PATH ABSOLUTE )
            #include( ${wxStedit_CONFIG_PATH}/${wxStedit_USE_FILE} )
            A2D_LINK_DIRECTORIES_DEPENDS( "${wxStedit_LIBRARY_DIRS}" )
            A2D_THIRDPARTY_LIBRARIES( "${wxStedit_LIBRARIES}" FRONT )
            A2D_INCLUDE_DIRECTORIES_DEPENDS( "${wxStedit_INCLUDE_DIRS}" )
        else()
            MESSAGE(SEND_ERROR "wxStedit not found")
        endif()
        
    else()          
        message( STATUS "searching wxLua via: ${wxLua_DIR}" )
        set(wxLua_DIR ${wxLua_DIR} CACHE PATH "Set the path to the root directory of wxLua containing the CMakeLists.txt file")
        IF (EXISTS ${wxLua_DIR}/CMakeLists.txt)
            # Note that wxLua could be anywhere
            ADD_SUBDIRECTORY(${wxLua_DIR} ${CMAKE_BINARY_DIRECTORY}/wxLua)
        ELSE()
            MESSAGE(WARNING "Invalid path to the wxLua library : '${wxLua_DIR}'")
        ENDIF()
        
        set( wxStedit_DIR ${wxStedit_DIR} CACHE PATH "Root dir of wxStEdit" FORCE)

        if (EXISTS ${wxStedit_DIR}/CMakeLists.txt AND EXISTS ${wxStedit_DIR}/include/wx/stedit/stedit.h)
            if (NOT TARGET wxStEditLib)
                add_subdirectory(${wxStedit_DIR} ${CMAKE_BINARY_DIR}/wxStEdit)
            endif()
        else()
            MESSAGE(STATUS "* WARNING: wxStedit_DIR directory does not exist: '${wxStedit_DIR}'")
            #return()
        endif()
        
    endif()
   

       
        
endif( WXART2D_USE_LUA )

##################################################
# package wxDocview
##################################################  

if ( WXART2D_USE_EDITOR )
    IF ( 0 )

        INCLUDE( ${wxart2d_SOURCE_DIR}/../wxdocview/share/wxdocview/wxdocviewMacros.cmake)

        IF ( EXISTS ${wxart2d_SOURCE_DIR}/../wxartbase/xmlparse )
        ELSE ( EXISTS ${wxart2d_SOURCE_DIR}../wxartbase/xmlparse )
            SET( wxDocview_USE_XMLPARSE 0 )
        ENDIF ( EXISTS ${wxart2d_SOURCE_DIR}/../wxartbase/xmlparse )

        IF ( WXART2D_USE_CVGIO )
            SET( wxDocview_USE_EXPAT 1 )
            SET( wxDocview_USE_XMLPARSE 1 )
        ENDIF( WXART2D_USE_CVGIO )

        IF ( EXISTS ${wxart2d_SOURCE_DIR}/../wxartbase/xmlparse )
        ELSE ( EXISTS ${wxart2d_SOURCE_DIR}../wxartbase/xmlparse )
            SET( wxDocview_USE_XMLPARSE 0 )
        ENDIF ( EXISTS ${wxart2d_SOURCE_DIR}/../wxartbase/xmlparse )

        IF( wxDocview_USE_XMLPARSE )
            DOCVIEW_LINK_LIBRARIES_WXLIKE( xmlparse )
        ENDIF( wxDocview_USE_XMLPARSE )

        DOCVIEW_LINK_LIBRARIES_WXLIKE( generalt )

        INCLUDE_DIRECTORIES( ../wxartbase/include )
        
        add_subdirectory( ../wxartbase ${wxart2d_BINARY_DIR}/wxartbase )
        
        add_subdirectory( ${wxart2d_SOURCE_DIR}/../wxdocview ${wxart2d_BINARY_DIR}/wxdocview )   
    ELSE ()

        SET ( CMAKE_MODULE_PATH "${wxart2d_BINARY_DIR}/../wxdocview/share/wxdocview" )
        if( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
            message( STATUS "searching wxDocview via: ${CMAKE_MODULE_PATH}" )
        else( IS_DIRECTORY ${CMAKE_MODULE_PATH} )
            message( ERROR "Module Path for wxDocview not set or not compiled in: ${CMAKE_MODULE_PATH}" )    
        endif( IS_DIRECTORY ${CMAKE_MODULE_PATH} )          
        set( wxDocview_DIR ${wxart2d_BINARY_DIR}/../wxdocview )
        set( wxDocview_INSIDE_BUILD 1)
        find_package( wxDocview )
        if( WXDOCVIEW_FOUND )
            MESSAGE( STATUS "wxDocview was found for wxArt2D package${wxDocview_wxWidgets_SELECTED_CONFIG}" )
            #include( ${wxDocview_USE_FILE} )
            string( TOLOWER ${wxDocview_wxWidgets_SELECTED_CONFIG} wxDocview_wxWidgets_SELECTED_CONFIG )
            string( TOLOWER ${wxWidgets_SELECTED_CONFIG} wxWidgets_SELECTED_CONFIG )
            if ( NOT "${wxDocview_wxWidgets_SELECTED_CONFIG}" STREQUAL "${wxWidgets_SELECTED_CONFIG}" )
                MESSAGE( FATAL_ERROR "The wxDocview found was compiled for a different wxWidget setting as the one choosen here for the wxart2d package.\nwxdocview has: ${wxDocview_wxWidgets_SELECTED_CONFIG}\nwxart2d has: ${wxWidgets_SELECTED_CONFIG}\nChoose the same wxWidget option as was done for the wxdocview module" )
            endif ( NOT "${wxDocview_wxWidgets_SELECTED_CONFIG}" STREQUAL "${wxWidgets_SELECTED_CONFIG}" )
            
            A2D_ADD_DEFINITIONS( "${wxDocview_FLAGS}" )
            A2D_INCLUDE_DIRECTORIES_DEPENDS( "${wxDocview_INCLUDE_DIRS}" )
            A2D_INCLUDE_DIRECTORIES_DEPENDS( "${wxDocview_INCLUDE_DIRS_DEPENDS}" )
            A2D_THIRDPARTY_LIBRARIES( "${wxDocview_LIBRARIES}" FRONT )
            A2D_LINK_DIRECTORIES_DEPENDS( "${wxDocview_LIBRARY_DIRS}" )
            # next is the one from wxWidgets, and is already added
            A2D_LINK_DIRECTORIES_DEPENDS( "${wxDocview_LIBRARY_DIRS_DEPENDS}" )

            #MESSAGE ( "wxDocview_DIR => ${wxDocview_DIR}" )
            #MESSAGE ( "wxDocview_DIR_SHARE => ${wxDocview_DIR_SHARE}" )
            #MESSAGE ( "wxDocview_INCLUDE_DIRS => ${wxDocview_INCLUDE_DIRS}" )
            #MESSAGE ( "wxDocview_LIBRARIES => ${wxDocview_LIBRARIES}" )
            #MESSAGE ( "wxDocview_LIBRARY_DIRS => ${wxDocview_LIBRARY_DIRS}" )
            #MESSAGE ( "wxDocview_LIBRARIES_WXLIKE => ${wxDocview_LIBRARIES_WXLIKE}" )
            
        else( WXDOCVIEW_FOUND )
            MESSAGE( ERROR "first compile the wxdocview module" )
        endif( WXDOCVIEW_FOUND )
    ENDIF ()
else( WXART2D_USE_EDITOR )
    #the place for wxdocview.h which was generated.
    A2D_INCLUDE_DIRECTORIES( ../wxartbase/include ) 
    A2D_INCLUDE_DIRECTORIES( ${wxart2d_BINARY_DIR}/wxartbase/include ) 
    INCLUDE_DIRECTORIES( ../wxartbase/include )
    add_subdirectory( ../wxartbase ${wxart2d_BINARY_DIR}/wxartbase ) 
    A2D_LINK_LIBRARIES_WXLIKE( wxartbase xmlparse )
    A2D_LINK_LIBRARIES_WXLIKE( wxartbase generalt ) 
endif ( WXART2D_USE_EDITOR )

##################################################
# Include subdirectories
##################################################

if( NOT WXART2D_MAKE_SHARED )
     A2D_LINK_LIBRARIES( "${WXART2D_THIRDPARTY_LIBRARIES}" BACK )
else( NOT WXART2D_MAKE_SHARED )
    
endif( NOT WXART2D_MAKE_SHARED )

MACRO( ADD_MODULE modulesname )
    if ( EXISTS ${wxart2d_SOURCE_DIR}/${modulesname} )
        add_subdirectory( ${modulesname} )
    endif ( EXISTS ${wxart2d_SOURCE_DIR}/${modulesname} )
ENDMACRO( ADD_MODULE modulesname )


    ADD_MODULE( genart )
    ADD_MODULE( aggdrawer )
    ADD_MODULE( artbase )
    ADD_MODULE( gdiplusdrawer )
    ADD_MODULE( canvas )
    ADD_MODULE( canextobj )
    ADD_MODULE( curves )    
    if ( WXART2D_USE_EDITOR )  
        ADD_MODULE( svgio )
        ADD_MODULE( gdsio )
        ADD_MODULE( keyio )
        ADD_MODULE( editor )
        ADD_MODULE( luawraps )   
    endif ( WXART2D_USE_EDITOR )

##################################################
# add the wxWidgets libraries at the end of the list, meaning link in first.
# this is needed, else linking errors.
##################################################

A2D_LINK_LIBRARIES( "${wxWidgets_LIBRARIES}" BACK )
#
if( CYGWIN OR MINGW )
    A2D_LINK_LIBRARIES( "${EXPAT_LIBRARIES}" BACK )
endif( CYGWIN OR MINGW )

##################################################
# Set all includes, flags, libraries
##################################################

set( WXART2D_FLAGS ${WXART2D_FLAGS} CACHE STRING "package flags" FORCE )
set( WXART2D_INCLUDE_DIRS ${WXART2D_INCLUDE_DIRS} CACHE STRING "package libs include paths" FORCE )
set( WXART2D_INCLUDE_DIRS_DEPENDS ${WXART2D_INCLUDE_DIRS_DEPENDS} CACHE STRING "package libs include paths dependencies" FORCE )
set( WXART2D_LIBRARY_DIRS ${WXART2D_LIBRARY_DIRS} CACHE STRING "package libs directory paths" FORCE )
set( WXART2D_LIBRARY_DIRS_DEPENDS ${WXART2D_LIBRARY_DIRS_DEPENDS} CACHE STRING "external package libs directory paths" FORCE )
set( WXART2D_LIBRARIES ${WXART2D_LIBRARIES} CACHE STRING "package libraries" FORCE )
set( WXART2D_THIRDPARTY_LIBRARIES ${WXART2D_THIRDPARTY_LIBRARIES} CACHE STRING "package libs directory path" FORCE )

set( WXART2D_POT_FILES ${WXART2D_POT_FILES} CACHE STRING "lang files" FORCE )

mark_as_advanced( 
WXART2D_LIBRARIES_WXLIKE 
WXART2D_FLAGS  a2dprec.h:
WXART2D_INCLUDE_DIRS 
WXART2D_INCLUDE_DIRS_DEPENDS
WXART2D_LIBRARY_DIRS
WXART2D_LIBRARY_DIRS_DEPENDS
WXART2D_LIBRARIES 
WXART2D_THIRDPARTY_LIBRARIES
)


MESSAGE( STATUS "WXART2D_LIBRARIES_WXLIKE: ${WXART2D_LIBRARIES_WXLIKE}")
MESSAGE( STATUS "WXART2D_FLAGS: ${WXART2D_FLAGS}")
MESSAGE( STATUS "WXART2D_INCLUDE_DIRS: ${WXART2D_INCLUDE_DIRS}")
MESSAGE( STATUS "WXART2D_LIBRARY_DIRS_DEPENDS: ${WXART2D_LIBRARY_DIRS_DEPENDS}")
MESSAGE( STATUS "WXART2D_LIBRARIES: ${WXART2D_LIBRARIES}")

##################################################
# Generate wxart2d-config files for both cmake and unix
##################################################

add_subdirectory( share/wxart2d )

##################################################
# other subdirectories
##################################################

add_subdirectory( include )
add_subdirectory( doc )

add_subdirectory( test )
add_subdirectory( samples )

add_subdirectory( art )


CAT_POT( ${wxart2d_SOURCE_DIR}/share/locale wxart2d "${wxart2d_SOURCE_DIR}/../wxdocview/share/locale/*.pot ${wxart2d_SOURCE_DIR}/share/locale/*.pot" )

if ( EXISTS ${wxart2d_SOURCE_DIR}/apps )
    add_subdirectory( apps )
endif ( EXISTS ${wxart2d_SOURCE_DIR}/apps )

if ( EXISTS ${wxart2d_SOURCE_DIR}/myapp )
    include( myapp/myproject.cmake )
    add_subdirectory( myapp )
endif ( EXISTS ${wxart2d_SOURCE_DIR}/myapp )

export( TARGETS FILE "${CMAKE_CURRENT_BINARY_DIR}/dependencies_wxart2d.cmake" )

#-------------------------------------------------------------------
# Build a CPack installer if CPack is available and this is a build 
if ( WXART2D_USE_PACK )    
    if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wxArt2D - Vector Graphics")
        set(CPACK_PACKAGE_VENDOR "wxArt2D")
        #set(CPACK_PACKAGE_VERSION "2.5.0")
        set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/readme.txt")
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wxArt2D 2D drawing library and framework for wxWidgets")
        #set(CPACK_RESOURCE_FILE_README "/home/andy/vtk/CMake/Templates/CPack.GenericDescription.txt")
        #set(CPACK_RESOURCE_FILE_WELCOME "/home/andy/vtk/CMake/Templates/CPack.GenericWelcome.txt")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/licence.txt")
        set(CPACK_PACKAGE_VERSION_MAJOR "${WXART2D_MAJOR_VERSION}")
        set(CPACK_PACKAGE_VERSION_MINOR "${WXART2D_MINOR_VERSION}")
        set(CPACK_PACKAGE_VERSION_PATCH "${WXART2D_BUILD_VERSION}")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "WXART2D_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
        set(CPACK_PACKAGE_FILE_NAME  
            "wxart2d-${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
        set(CPACK_SOURCE_PACKAGE_FILE_NAME 
            "wxart2d-dev-${wxWidgets_PORT}${wxWidgets_UNV}${wxWidgets_UCD}${wxWidgets_DBG}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
        set(CPACK_PACKAGE_EXECUTABLES
        "wxart2d" "WXART2D"
        )
        set(CPACK_SOURCE_STRIP_FILES "")
        set(CPACK_STRIP_FILES "bin/ccmake;bin/cmake;bin/cpack;bin/ctest")
 
        if(WIN32 AND NOT UNIX )
            set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} wxArt2D library")
            set(CPACK_NSIS_HELP_LINK "http://www.wxart2d.org")
            set(CPACK_NSIS_URL_INFO_ABOUT "http://www.wxart2d.org")
            set(CPACK_NSIS_CONTACT "http://www.wxart2d.org")
        endif(WIN32 AND NOT UNIX )

        include(CPack)
    endif(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")   
endif ( WXART2D_USE_PACK )


# when using this application outside wxart2d tree, this can be handy 
if( wxWidgets_USE_DEBUG )
    IF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" )

        IF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio 10" OR 
		     ${CMAKE_GENERATOR} MATCHES "Visual Studio 12" OR 
		     ${CMAKE_GENERATOR} MATCHES "Visual Studio 14" OR 
		     ${CMAKE_GENERATOR} MATCHES "Visual Studio 15" OR 
			 ${CMAKE_GENERATOR} MATCHES "Visual Studio 16" )
            if ( WXART2D_USE_EDITOR )
                include_external_msproject( docview ${wxart2d_BINARY_DIR}/../wxdocview/docview/src/docview.vcxproj )
                include_external_msproject( general ${wxart2d_BINARY_DIR}/../wxdocview/wxartbase/general/src/generalt.vcxproj )
                include_external_msproject( xmlparse ${wxart2d_BINARY_DIR}/../wxdocview/wxartbase/xmlparse/src/xmlparse.vcxproj )
                if ( WXART2D_USE_KBOOL )
                    if ( WXART2D_USE_KBOOLNEW )
                            include_external_msproject( kbool ${wxart2d_BINARY_DIR}/../../thirdparty/kbooln/src/kbool.vcxproj )
                        else()
                            include_external_msproject( kbooln ${wxart2d_BINARY_DIR}/../../thirdparty/kbool/src/kbool.vcxproj )
                    endif()
                endif()
                if ( wxDocview_USE_IRRXML )
                    include_external_msproject( irrxml ${wxart2d_BINARY_DIR}/../../thirdparty/irrxml-1.2/src/irrxml.vcproj )
                endif()     
            else()
                #include_external_msproject( general ${wxart2d_BINARY_DIR}/wxartbase/general/src/generalt.vcxproj )
                #include_external_msproject( xmlparse ${wxart2d_BINARY_DIR}/wxartbase/xmlparse/src/xmlparse.vcxproj )
            endif()        
            include_external_msproject( freetype ${wxart2d_SOURCE_DIR}/../../thirdparty/freetype-2.5.5/builds/windows/vc2010/freetype.vcxproj )
        else ( )
            if ( WXART2D_USE_EDITOR )
                include_external_msproject( docview ${wxart2d_BINARY_DIR}/../wxdocview/docview/src/docview.vcproj )
            endif()        
            if ( WXART2D_USE_EDITOR )
                include_external_msproject( docview ${wxart2d_BINARY_DIR}/../wxdocview/docview/src/docview.vcproj )
                include_external_msproject( general ${wxart2d_BINARY_DIR}/../wxdocview/wxartbase/general/src/generalt.vcproj )
                include_external_msproject( xmlparse ${wxart2d_BINARY_DIR}/../wxdocview/wxartbase/xmlparse/src/xmlparse.vcproj )
                if ( WXART2D_USE_KBOOL )
                    if ( WXART2D_USE_KBOOLNEW )
                        include_external_msproject( kbool ${wxart2d_BINARY_DIR}/../../thirdparty/kbooln/src/kbool.vcproj )
                    else()
                        include_external_msproject( kbool ${wxart2d_BINARY_DIR}/../../thirdparty/kbool/src/kbool.vcproj )
                    endif()
                endif()
                if ( wxDocview_USE_IRRXML )
                    include_external_msproject( irrxml ${wxart2d_BINARY_DIR}/../../thirdparty/irrxml-1.2/irrxml.vcproj )
                endif()     
            else()
                #include_external_msproject( general ${wxart2d_BINARY_DIR}/wxartbase/general/src/generalt.vcproj )
                #include_external_msproject( xmlparse ${wxart2d_BINARY_DIR}/wxartbase/xmlparse/src/xmlparse.vcproj )
            endif()        
            include_external_msproject( freetype ${wxart2d_SOURCE_DIR}/../../thirdparty/freetype-2.5.5/builds/windows/vc2010/freetype.vcproj )
        ENDIF ()

    ENDIF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio.*" ) 
endif ()
 

