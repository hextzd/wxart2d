PROJECT(tabframe)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_docview_tabframe ${WIN32GUI}  ${wxWidgets_RC} ../common/view.h ../common/doc.h ../common/docdialog.h
../common/view.cpp ../common/doc.cpp ../common/docdialog.cpp
tabframe.cpp tabframe.h)

TARGET_LINK_LIBRARIES( sample_docview_tabframe ${wxDocview_LIBRARY_LIST_WXLIKE} ${wxDocview_LIBRARIES} ${wxWidgets_LIBRARIES} )

