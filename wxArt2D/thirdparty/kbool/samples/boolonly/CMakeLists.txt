ADD_EXECUTABLE(boolonly 
    boolonly.cpp
    keyinout.cpp
    boolonly.h
    keyinout.h
)

IF(WIN32)
    ADD_DEFINITIONS( -D_MSWVC_ )
ELSE(WIN32)
    ADD_DEFINITIONS( -D__UNIX__  )
ENDIF(WIN32)	

INCLUDE_DIRECTORIES( ${kbool_SOURCE_DIR}/..  )

TARGET_LINK_LIBRARIES( boolonly kbool${PFDEBUG} )

