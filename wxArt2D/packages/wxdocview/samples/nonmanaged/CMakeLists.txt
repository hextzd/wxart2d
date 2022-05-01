PROJECT(nonmanaged)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_docview_nonmanaged  ${WIN32GUI}  ${wxWidgets_RC}  ../common/view.h ../common/doc.h ../common/docdialog.h
../common/view.cpp ../common/doc.cpp ../common/docdialog.cpp
nonmanaged.cpp nonmanaged.h)

TARGET_LINK_LIBRARIES( sample_docview_nonmanaged  ${wxDocview_LIBRARY_LIST_WXLIKE} ${wxDocview_LIBRARIES} ${wxWidgets_LIBRARIES} )

