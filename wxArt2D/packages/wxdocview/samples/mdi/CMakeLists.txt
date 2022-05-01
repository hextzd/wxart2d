PROJECT(mdi)

COMPILE_RC( "${wxWidgets_RC}" wx.o )

ADD_EXECUTABLE( sample_docview_mdi ${WIN32GUI}  ${wxWidgets_RC}  ../common/view.h ../common/doc.h ../common/docdialog.h
../common/view.cpp ../common/doc.cpp ../common/docdialog.cpp
mdi.cpp mdi.h)

TARGET_LINK_LIBRARIES( sample_docview_mdi  ${wxDocview_LIBRARY_LIST_WXLIKE} ${wxDocview_LIBRARIES} ${wxWidgets_LIBRARIES} )
