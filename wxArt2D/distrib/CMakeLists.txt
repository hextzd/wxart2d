#
# Build the distribution
#

STRING( REGEX REPLACE "D:" "/d" wxart2d_SOURCE_DIR_UNIXED ${wxart2d_SOURCE_DIR} )
STRING( REGEX REPLACE "D:" "/d" wxart2d_BINARY_DIR_UNIXED ${wxart2d_BINARY_DIR} )
CONFIGURE_FILE( ${wxart2d_SOURCE_DIR}/distrib/makedist.sh.in ${wxart2d_BINARY_DIR}/distrib/makedist.sh @ONLY IMMEDIATE)
               

