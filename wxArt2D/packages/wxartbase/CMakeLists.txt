##################################################
# Name:        CMakeLists.txt
# Purpose:     Input file for CMake
# Author:      Klaas Holwerda
# Licence:     wxWindows licence   # RCS Id:     $Id: CMakeLists.txt,v 1.184 2009/10/04 19:31:09 titato Exp $
##################################################

PROJECT( wxartbase )

SET( wxartbase_VERSION "1.2.0" )
SET( wxartbase_MAJOR_VERSION 1 )
SET( wxartbase_MINOR_VERSION 2 )
SET( wxartbase_BUILD_VERSION 0 ) 

add_subdirectory( ${wxartbase_SOURCE_DIR}/include ) 
add_subdirectory( general )
add_subdirectory( xmlparse )



