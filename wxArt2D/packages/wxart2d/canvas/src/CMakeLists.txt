IF( WXART2D_MAKE_SHARED )
    ADD_DEFINITIONS ( -DA2DCANVASMAKINGDLL )
    SET(BUILD_SHARED_LIBS = 1)
ENDIF( WXART2D_MAKE_SHARED )


IF ( WXART2D_USE_KBOOL )
  SET( canvas_BOOL_SOURCE booloper.cpp )
ELSE ( WXART2D_USE_KBOOL )
  SET( canvas_BOOL_SOURCE )
ENDIF ( WXART2D_USE_KBOOL )

SET( canvas_HEADERS
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/eval.h    
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/cameleon.h
	${wxart2d_SOURCE_DIR}/include/wx/canvas/cansymboledit.h
	${wxart2d_SOURCE_DIR}/include/wx/canvas/camcreate.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/candefs.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canobj.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canobj1.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canobj2.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canvas.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canvasogl.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/cansim.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/layerinf.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/polygon.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/vpath.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/drawing.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/drawer.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/recur.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/xmlpars.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canglob.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canprim.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canpin.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/cantext.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canmod.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canprop.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/objlist.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/tools.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/styleprop.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/restrict.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canwidget.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/booloper.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/hittest.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/wire.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/rectangle.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canimage.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/algos.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/algos2.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/connectgen.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/edit.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/sttool.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/sttool2.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/sttoolmes.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/mastertool.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/mastertoolgroups.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/route.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/canorderdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/identifydlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/pathsettings.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/strucdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/styledialg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/tooldlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/layerdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/fontdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/groupdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/transdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/settingsdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/snap.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/execdlg.h
    ${wxart2d_SOURCE_DIR}/include/wx/canvas/xh_a2dmenu.h    
)

SET( canvas_SOURCE
    cameleon.cpp
	cansymboledit.cpp
	camcreate.cpp	
    canprim.cpp 
    canpin.cpp 
    cantext.cpp 
    canobj.cpp
    canobj2.cpp
    layerinf.cpp 
    polygon.cpp 
    canvas.cpp
	canvasogl.cpp
    cansim.cpp 
    recur.cpp
    drawer.cpp 
    objlist.cpp 
    vpath.cpp 
    tools.cpp 
    drawing.cpp
    canprop.cpp 
    xmlpars.cpp 
    canglob.cpp 
    styleprop.cpp
    restrict.cpp 
    canwidget.cpp 
    hittest.cpp 
    wire.cpp 
    rectangle.cpp
    canimage.cpp 
    algos.cpp 
    connectgen.cpp
    edit.cpp
    sttool.cpp
    sttool2.cpp
    sttoolmes.cpp
    mastertool.cpp
    mastertoolgroups.cpp
    mastertoolgroups2.cpp
    mastertoolgroups3.cpp
    mastertoolgroups4.cpp
	  route.cpp
    eval.cpp
    canorderdlg.cpp
    strucdlg.cpp
    styledialg.cpp
    tooldlg.cpp
    layerdlg.cpp
    identifydlg.cpp
	  pathsettings.cpp
	  transdlg.cpp
	  settingsdlg.cpp
    snap.cpp
    fontdlg.cpp
    fontdlg2.cpp
    groupdlg.cpp
    execdlg.cpp
    xh_a2dmenu.cpp
    
    ${canvas_BOOL_SOURCE}
)

A2D_PCH()

# I put some frequently changing files first, so that they are compiled first

ADD_LIBRARY_WXLIKE( wxart2d canvas
    ${a2d_PCH_SOURCE_FILE}
    
    ${canvas_SOURCE}
    ${canvas_BOOL_SOURCE}
    ${canvas_HEADERS}
 )

GENERATE_POT( ${wxart2d_SOURCE_DIR}/share/locale canvas )

IF( WXART2D_MAKE_SHARED )
    IF( WIN32 )
        IF( WXART2D_USE_AGGDRAWER )
            TARGET_LINK_LIBRARIES(canvas artbase kbool aggdrawer)
        ELSE( WXART2D_USE_AGGDRAWER )
            TARGET_LINK_LIBRARIES(canvas artbase kbool)
        ENDIF( WXART2D_USE_AGGDRAWER )
    ELSE( WIN32 )
        IF( WXART2D_USE_AGGDRAWER )
            TARGET_LINK_LIBRARIES(canvas artbase kbool aggdrawer svgio gdsio )
        ELSE( WXART2D_USE_AGGDRAWER )
            TARGET_LINK_LIBRARIES(canvas artbase kbool svgio gdsio )
        ENDIF( WXART2D_USE_AGGDRAWER )
    ENDIF( WIN32 )
ENDIF( WXART2D_MAKE_SHARED )

INSTALL( 
    FILES ${canvas_HEADERS} 
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/canvas
)
INSTALL( TARGETS canvas DESTINATION lib )

INSTALL( 
    FILES ${wxart2d_SOURCE_DIR}/include/wx/GL/glext.h
    DESTINATION include/wxart2d-${WXART2D_MAJOR_VERSION}.${WXART2D_MINOR_VERSION}/wx/GL
)