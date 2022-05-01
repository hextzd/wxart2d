# \file bin/FindExpat.cmake
# \author Erik van der Wal
#
#     Copyright: 2004 (c) Erik van der Wal
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: FindExpat.cmake,v 1.18 2008/02/17 12:39:38 titato Exp $

# This module finds if expat is installed and determines where the
# include files and libraries are.
# This code sets the following variables:
#
# EXPAT_ROOT_DIR      - Directorie where expat is installed
# EXPAT_INCLUDE_DIR   - Directories to include to use expat
# EXPAT_LIBRARIES     - Files to link against to use expat
# EXPAT_LINK_DIR      - Directories to link to for using expat
# EXPAT_FOUND         - If false, don't try to use expat
#

# Mark advanced settings
SET( EXPAT_LIBRARIES "expat" )

MARK_AS_ADVANCED( FORCE EXPAT_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE EXPAT_LINK_DIR )
MARK_AS_ADVANCED( FORCE EXPAT_FOUND )
        
# Try to find expat from an environment variable WXWIN
# In case of error, continue with the next attempt
IF( NOT EXPAT_FOUND )

    # Starting with wxWidgets-3.1.1, path to sources of expat has been changed.
    IF( ${wxWidgets_VERSION} VERSION_LESS "3.1.1" )
        SET( EXPAT_WXSUBDIR "/src/expat/lib" )
    ELSE()
        SET( EXPAT_WXSUBDIR "/src/expat/expat/lib" )
    ENDIF()

	IF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        FIND_FILE( 
            EXPAT_HDR_OK3
            NAMES
            expat.h
            PATHS 
            "${wxWidgets_ROOT_DIR}${EXPAT_WXSUBDIR}"
        )   
        IF( EXPAT_HDR_OK3 )          
            FIND_LIBRARY( EXPAT_LIB_OK3
                NAMES wxexpat${wxWidgets_DBG}
                PATHS "$ENV{EXPAT}/lib" ${wxWidgets_LIB_DIR}
                NO_DEFAULT_PATH
            )                   
            IF( EXPAT_LIB_OK3 )           
                SET( EXPAT_FOUND TRUE )
                SET( EXPAT_ROOT_DIR "${wxWidgets_ROOT_DIR}" )
                SET( EXPAT_INCLUDE_DIR "${EXPAT_ROOT_DIR}${EXPAT_WXSUBDIR}" )
                SET( EXPAT_LINK_DIR "${EXPAT_ROOT_DIR}/lib" )
                SET( EXPAT_LIBRARIES wxexpat${wxWidgets_DBG} )
            ENDIF( EXPAT_LIB_OK3 )
        ENDIF( EXPAT_HDR_OK3 )
                   
	ELSE( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        FIND_FILE( 
            EXPAT_HDR_OK3
            NAMES
            expat.h
            PATHS 
            "$ENV{WXWIN}${EXPAT_WXSUBDIR}"
            /usr/include
            /usr/local/include
        )   
        IF( EXPAT_HDR_OK3 )
            FIND_LIBRARY( EXPAT_LIB_OK3
                NAMES wxexpat${wxWidgets_DBG}-${wxWidgets_RELEASE_NODOT}
                PATHS "$ENV{EXPAT}/lib" ${wxWidgets_ROOT_DIR}/lib
            )              
            IF( EXPAT_LIB_OK3 )
                SET( EXPAT_FOUND TRUE )
                SET( EXPAT_ROOT_DIR "${wxWidgets_ROOT_DIR}" )
                SET( EXPAT_INCLUDE_DIR "$ENV{WXWIN}${EXPAT_WXSUBDIR}" )
                SET( EXPAT_LINK_DIR "${EXPAT_ROOT_DIR}/lib" )
                SET( EXPAT_LIBRARIES "wxexpat${wxWidgets_DBG}-${wxWidgets_RELEASE_NODOT}" )
            ELSE( EXPAT_LIB_OK3 )
                FIND_LIBRARY( EXPAT_LIB_OK3b
                    NAMES expat
                    PATHS /lib /usr/lib /usr/local/lib "$ENV{EXPAT}/lib" ${wxWidgets_ROOT_DIR}/lib
                )              
                IF( EXPAT_LIB_OK3b )
                    SET( EXPAT_FOUND TRUE )
                    SET( EXPAT_ROOT_DIR "" )
                    SET( EXPAT_INCLUDE_DIR "$ENV{WXWIN}${EXPAT_WXSUBDIR}" )
                    SET( EXPAT_LINK_DIR "" )
                    SET( EXPAT_LIBRARIES "expat" )
                ENDIF( EXPAT_LIB_OK3b )
            ENDIF( EXPAT_LIB_OK3 )                      
        ENDIF( EXPAT_HDR_OK3 )
    
	ENDIF( ${wxWidgets_FIND_STYLE} STREQUAL "WIN32_STYLE_FIND" )
        
ENDIF( NOT EXPAT_FOUND )

DBG_MSG( "EXPAT_ROOT_DIR = ${EXPAT_ROOT_DIR}" )
DBG_MSG( "EXPAT_INCLUDE_DIR = ${EXPAT_INCLUDE_DIR}" )
DBG_MSG( "EXPAT_LIBRARIES = ${EXPAT_LIBRARIES}" )
DBG_MSG( "EXPAT_LINK_DIR = ${EXPAT_LINK_DIR}" )
DBG_MSG( "EXPAT_FOUND = ${EXPAT_FOUND}" )
