# \file apps/thumbo /CMakeLists.txt
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.23 2005/09/09 18:57:07 titato Exp $

PROJECT( thumbo )

ADD_SUBDIRECTORY( src )

# copy files

    COPYFILESALL( ${thumbo_SOURCE_DIR}/drawings ${thumbo_BINARY_DIR}/data "*"  )
    COPYFILESALL( ${thumbo_SOURCE_DIR}/config ${thumbo_BINARY_DIR}/config  "*" )
    COPYFILESALL( ${thumbo_SOURCE_DIR}/process ${thumbo_BINARY_DIR}/process  "*" )

