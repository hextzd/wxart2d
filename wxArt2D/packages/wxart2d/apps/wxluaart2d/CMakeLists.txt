# \file apps/wxluaart2d/CMakeLists.txt
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.1 2009/05/07 19:57:19 titato Exp $

PROJECT( wxluaart2d )

ADD_SUBDIRECTORY( src )

    COPYFILESALL( ${wxluaart2d_SOURCE_DIR}/src ${wxluaart2d_BINARY_DIR}/src "*.xpm"  )

