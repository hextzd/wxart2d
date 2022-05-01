# \file kbool config 
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Erik van der Wal
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.19 2009/06/23 18:56:07 titato Exp $

PROJECT( kboolconfig )

CONFIGURE_FILE( ${kboolconfig_SOURCE_DIR}/kboolConfig.cmake.in
                ${kbool_BINARY_DIR}/share/kbool/kboolConfig.cmake
                @ONLY IMMEDIATE )
                
CONFIGURE_FILE( ${kboolconfig_SOURCE_DIR}/Usekbool.cmake.in
                ${kbool_BINARY_DIR}/share/kbool/Usekbool.cmake
                @ONLY IMMEDIATE )

CONFIGURE_FILE( ${kboolconfig_SOURCE_DIR}/Findkbool.cmake
                ${kbool_BINARY_DIR}/share/kbool/Findkbool.cmake
                @ONLY IMMEDIATE )

                
INSTALL( FILES ${kbool_BINARY_DIR}/share/kbool/kboolConfig.cmake DESTINATION "share/kbool" )
INSTALL( FILES ${kbool_BINARY_DIR}/share/kbool/Usekbool.cmake DESTINATION "share/kbool" )              
INSTALL( FILES ${kbool_BINARY_DIR}/share/kbool/Findkbool.cmake DESTINATION "share/kbool" )              
