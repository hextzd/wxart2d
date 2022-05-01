# notice the rename here                
INSTALL( DIRECTORY ${wxart2d_SOURCE_DIR}/art/cursors DESTINATION "share/wxart2d/art"  
    FILES_MATCHING PATTERN ".svn" EXCLUDE PATTERN "*.cur" )
INSTALL( DIRECTORY ${wxart2d_SOURCE_DIR}/art/resources DESTINATION "share/wxart2d/art"  
    FILES_MATCHING PATTERN ".svn" EXCLUDE PATTERN "*.xrc" )
INSTALL( DIRECTORY ${wxart2d_SOURCE_DIR}/art/layers DESTINATION "share/wxart2d/art"  
    FILES_MATCHING PATTERN ".svn" EXCLUDE PATTERN "*.*" )
INSTALL( DIRECTORY ${wxart2d_SOURCE_DIR}/art/fonts DESTINATION "share/wxart2d/art"  
    FILES_MATCHING PATTERN ".svn" EXCLUDE PATTERN "*.*" )


