# \file bin/CheckMSVC.cmake
# \author Erik van der Wal
#
#     Copyright: 2004 (c) Erik van der Wal
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CheckMSVC.cmake,v 1.2 2004/06/30 14:04:24 kire_putsje Exp $

# This module performs a check if the compiler is the MS visual C compiler.
# This code sets the following variable:
#
# MSVC      - Boolean variable, indicating if MSVC was found.
#
# Description:
# The check is done by looking at the define _MSC_VER, which is
# set by the compiler. A test program is generated and then compiled.
# If it can be compiled, it is MSVC.

#FILE(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/msvctest.cpp
#    "int main() { \n"
#    "#ifdef _MSC_VER\n"
#    "    return 0;\n"
#    "#else\n"
#    "    #error Sorry, this is not Microsoft Visual C\n"
#    "#endif\n"
#    "}\n"
#)
#TRY_COMPILE( MSVC
#    ${CMAKE_BINARY_DIR}/CMakeTmp
#    ${CMAKE_BINARY_DIR}/CMakeTmp/msvctest.cpp
#)

IF( CMAKE_GENERATOR MATCHES "Visual Studio*" )
    SET( MSVC TRUE )
ENDIF( CMAKE_GENERATOR MATCHES "Visual Studio*" )

IF( MSVC )
    #MESSAGE( STATUS "Microsoft Visual C compiler detected." )
ENDIF( MSVC )