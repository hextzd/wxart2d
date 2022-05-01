# \file apps/teto/CMakeLists.txt
# \author Klaas Holwerda
#
#     Copyright: 2004 (c) Klaas Holwerda
#
#     Licence: wxWidgets Licence
#
#     RCS-ID: $Id: CMakeLists.txt,v 1.23 2005/09/09 18:57:07 titato Exp $

PROJECT( teto )

ADD_SUBDIRECTORY( src )

# copy files

    COPYFILESALL( ${teto_SOURCE_DIR}/drawings ${teto_BINARY_DIR}/drawings "*"  )
    COPYFILESALL( ${teto_SOURCE_DIR}/config ${teto_BINARY_DIR}/config  "*" )
    COPYFILESALL( ${teto_SOURCE_DIR}/process ${teto_BINARY_DIR}/process  "*" )

