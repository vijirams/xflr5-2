# -------------------------------------------------
# Project created by QtCreator 2009-02-14T15:30:46
# -------------------------------------------------

# message(qmake version: $$[QMAKE_VERSION])
# message(Qt version: $$[QT_VERSION])

#Qt5.4 required for QOpenGLWidget instead of QGLWidget
lessThan(QT_MAJOR_VERSION, 5) {
  error("Qt5.4 or greater is required for xflr5 v6")
}
else
{
	lessThan(QT_MINOR_VERSION, 4) {
	  error("Qt5.4 or greater is required for xflr5 v6")
	}
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


CONFIG += qt
QT += opengl

TEMPLATE = app


SOURCES += \
	globals/XFLR5Application.cpp \
	globals/globals.cpp \
	globals/main.cpp \
	globals/mainframe.cpp \
	viewwidgets/section2dwidget.cpp \
	viewwidgets/graphwidget.cpp \
	viewwidgets/graphtilewidget.cpp \
	viewwidgets/legendwidget.cpp \
	viewwidgets/BodyFrameWidget.cpp \
	viewwidgets/BodyLineWidget.cpp \
	viewwidgets/Direct2dDesign.cpp \
	viewwidgets/miarextilewidget.cpp \
	viewwidgets/xdirecttilewidget.cpp \
	viewwidgets/wingwidget.cpp \
	viewwidgets/oppointwidget.cpp \
	viewwidgets/inverseviewwidget.cpp \
	misc/ProgressDlg.cpp \
	misc/ModDlg.cpp \
	misc/PolarFilterDlg.cpp \
	misc/RenameDlg.cpp \
	misc/AboutQ5.cpp \
	misc/NewNameDlg.cpp \
	misc/ObjectPropsDlg.cpp \
	misc/EditPlrDlg.cpp \
	misc/LengthUnitDlg.cpp \
	misc/exponentialslider.cpp \
	misc/stlexportdialog.cpp \
	miarex/Miarex.cpp \
	miarex/objects3d.cpp \
	miarex/analysis/StabPolarDlg.cpp \
	miarex/analysis/CtrlTableDelegate.cpp \
	miarex/analysis/WAdvancedDlg.cpp \
	miarex/analysis/WPolarDlg.cpp  \
	miarex/analysis/LLTAnalysisDlg.cpp \
	miarex/analysis/PanelAnalysisDlg.cpp \
	miarex/analysis/AeroDataDlg.cpp \
	miarex/analysis/EditPolarDefDlg.cpp \
	miarex/design/GL3dWingDlg.cpp \
	miarex/design/GL3dBodyDlg.cpp \
	miarex/design/PlaneDlg.cpp \
	miarex/design/WingDelegate.cpp \
	miarex/design/WingScaleDlg.cpp \
	miarex/design/InertiaDlg.cpp \
	miarex/design/BodyGridDlg.cpp \
	miarex/design/BodyScaleDlg.cpp \
	miarex/design/BodyTableDelegate.cpp \
	miarex/design/BodyTransDlg.cpp \
	miarex/design/EditObjectDelegate.cpp \
	miarex/design/EditPlaneDlg.cpp \
	miarex/design/EditBodyDlg.cpp \
	miarex/design/wingseldlg.cpp \
	miarex/view/TargetCurveDlg.cpp \
	miarex/view/GL3DScales.cpp \
	miarex/view/StabViewDlg.cpp \
	miarex/view/W3dPrefsDlg.cpp \
	miarex/mgt/ImportObjectDlg.cpp\
	miarex/view/GLLightDlg.cpp \
	miarex/mgt/ManagePlanesDlg.cpp \
	miarex/mgt/PlaneTableDelegate.cpp \
	miarex/mgt/XmlPlaneReader.cpp \
	miarex/mgt/XmlPlaneWriter.cpp \
	miarex/mgt/xmlwpolarreader.cpp \
	miarex/mgt/xmlwpolarwriter.cpp \
	xdirect/XDirect.cpp \
	xdirect/ManageFoilsDlg.cpp \
	xdirect/XDirectStyleDlg.cpp \
	xdirect/analysis/BatchDlg.cpp \
	xdirect/analysis/BatchThreadDlg.cpp \
	xdirect/analysis/FoilPolarDlg.cpp \
	xdirect/analysis/ReListDlg.cpp \
	xdirect/analysis/XFoilAdvancedDlg.cpp \
	xdirect/analysis/XFoilAnalysisDlg.cpp \
	xdirect/analysis/XFoilTask.cpp \
	xdirect/geometry/CAddDlg.cpp \
	xdirect/geometry/FlapDlg.cpp \
	xdirect/geometry/FoilCoordDlg.cpp \
	xdirect/geometry/FoilGeomDlg.cpp \
	xdirect/geometry/InterpolateFoilsDlg.cpp \
	xdirect/geometry/LEDlg.cpp \
	xdirect/geometry/NacaFoilDlg.cpp \
	xdirect/geometry/TEGapDlg.cpp \
	xdirect/geometry/TwoDPanelDlg.cpp \
	xdirect/xmlpolarreader.cpp \
	xdirect/xmlpolarwriter.cpp \
	xdirect/objects2d.cpp \
	xinverse/FoilSelectionDlg.cpp \
	xinverse/PertDlg.cpp \
	xinverse/XInverse.cpp \
	xinverse/InverseOptionsDlg.cpp \
	design/FoilTableDelegate.cpp \
	design/LECircleDlg.cpp \
	design/AFoil.cpp \
	design/SplineCtrlsDlg.cpp \
	design/AFoilTableDlg.cpp \
	design/GridSettingsDlg.cpp \
	glcontextinfo/glrenderwindow.cpp \
	glcontextinfo/openglinfodlg.cpp \
	script/xflscriptexec.cpp \
	script/xflscriptreader.cpp \
	gui_objects/Spline5.cpp \
	gui_objects/SplineFoil.cpp \
	viewwidgets/glWidgets/gl3dview.cpp \
	viewwidgets/glWidgets/gl3dbodyview.cpp \
	viewwidgets/glWidgets/gl3dmiarexview.cpp \
	viewwidgets/glWidgets/gl3dwingview.cpp \
	viewwidgets/glWidgets/gl3dplaneview.cpp \
	viewwidgets/glWidgets/ArcBall.cpp \
	misc/options/displayoptions.cpp \
	misc/options/language.cpp \
	misc/options/preferencesdlg.cpp \
	misc/options/saveoptions.cpp \
	misc/options/Units.cpp \
	misc/color/ColorButton.cpp \
	misc/text/DoubleEdit.cpp \
	misc/text/IntEdit.cpp \
	misc/text/TextClrBtn.cpp \
	misc/line/LineBtn.cpp \
	misc/line/LineCbBox.cpp \
	misc/line/LineDelegate.cpp \
	misc/line/LinePickerDlg.cpp \
	misc/text/MinTextEdit.cpp \
	misc/text/FloatEditDelegate.cpp \
	misc/voidwidget.cpp \
	graph/graphdlg.cpp \
	graph/graph.cpp \
	graph/graph_globals.cpp \
	graph/curve.cpp


HEADERS += \
	globals/gui_enums.h \
	globals/gui_params.h \
	globals/globals.h \
	globals/mainframe.h \
	globals/XFLR5Application.h \
	viewwidgets/section2dwidget.h \
	viewwidgets/graphwidget.h \
	viewwidgets/graphtilewidget.h \
	viewwidgets/legendwidget.h \
	viewwidgets/BodyFrameWidget.h \
	viewwidgets/BodyLineWidget.h \
	viewwidgets/Direct2dDesign.h \
	viewwidgets/miarextilewidget.h \
	viewwidgets/xdirecttilewidget.h \
	viewwidgets/wingwidget.h \
	viewwidgets/oppointwidget.h \
	viewwidgets/inverseviewwidget.h \
	misc/ModDlg.h \
	misc/PolarFilterDlg.h \
	misc/RenameDlg.h \
	misc/color/ColorButton.h \
	misc/AboutQ5.h \
	misc/text/DoubleEdit.h \
	misc/ProgressDlg.h \
	misc/NewNameDlg.h \
	misc/ObjectPropsDlg.h \
	misc/EditPlrDlg.h \
	misc/LengthUnitDlg.h \
	misc/exponentialslider.h \
	misc/stlexportdialog.h \
	misc/voidwidget.h \
	miarex/Miarex.h \
	miarex/objects3d.h \
	miarex/analysis/WAdvancedDlg.h \
	miarex/analysis/WPolarDlg.h \
	miarex/analysis/StabPolarDlg.h \
	miarex/analysis/CtrlTableDelegate.h \
	miarex/analysis/LLTAnalysisDlg.h \
	miarex/analysis/PanelAnalysisDlg.h \
	miarex/analysis/AeroDataDlg.h \
	miarex/analysis/EditPolarDefDlg.h \
	miarex/design/InertiaDlg.h \
	miarex/design/GL3dBodyDlg.h \
	miarex/design/WingScaleDlg.h \
	miarex/design/WingDelegate.h \
	miarex/design/PlaneDlg.h \
	miarex/design/BodyGridDlg.h \
	miarex/design/BodyTableDelegate.h \
	miarex/design/BodyScaleDlg.h \
	miarex/design/GL3dWingDlg.h \
	miarex/design/BodyTransDlg.h \
	miarex/design/EditObjectDelegate.h \
	miarex/design/EditPlaneDlg.h \
	miarex/design/EditBodyDlg.h \
	miarex/design/wingseldlg.h \
	miarex/view/TargetCurveDlg.h \
	miarex/view/GL3DScales.h \
	miarex/view/StabViewDlg.h \
	miarex/view/GLLightDlg.h \
	miarex/view/W3dPrefsDlg.h \
	miarex/mgt/ImportObjectDlg.h \
	miarex/mgt/ManagePlanesDlg.h \
	miarex/mgt/PlaneTableDelegate.h \
	miarex/mgt/XmlPlaneReader.h \
	miarex/mgt/XmlPlaneWriter.h \
	miarex/mgt/xmlwpolarreader.h \
	miarex/mgt/xmlwpolarwriter.h \
	xdirect/XDirect.h \
	xdirect/ManageFoilsDlg.h \
	xdirect/XDirectStyleDlg.h \
	xdirect/analysis/BatchDlg.h \
	xdirect/analysis/BatchThreadDlg.h \
	xdirect/analysis/FoilPolarDlg.h \
	xdirect/analysis/ReListDlg.h \
	xdirect/analysis/XFoilAdvancedDlg.h \
	xdirect/analysis/XFoilAnalysisDlg.h \
	xdirect/analysis/XFoilTask.h \
	xdirect/analysis/xfoiltaskevent.h \
	xdirect/geometry/CAddDlg.h \
	xdirect/geometry/FlapDlg.h \
	xdirect/geometry/FoilCoordDlg.h \
	xdirect/geometry/FoilGeomDlg.h \
	xdirect/geometry/InterpolateFoilsDlg.h \
	xdirect/geometry/LEDlg.h \
	xdirect/geometry/NacaFoilDlg.h \
	xdirect/geometry/TEGapDlg.h \
	xdirect/geometry/TwoDPanelDlg.h \
	xdirect/xmlpolarreader.h \
	xdirect/xmlpolarwriter.h \
	xdirect/objects2d.h \
	xinverse/XInverse.h \
	xinverse/InverseOptionsDlg.h \
	xinverse/FoilSelectionDlg.h \
	xinverse/PertDlg.h \
	design/AFoil.h \
	design/LECircleDlg.h \
	design/SplineCtrlsDlg.h \
	design/FoilTableDelegate.h \
	design/AFoilTableDlg.h \
	design/GridSettingsDlg.h \
	glcontextinfo/glrenderwindow.h \
	glcontextinfo/openglinfodlg.h \
	script/xflscriptexec.h \
	script/xflscriptreader.h \
	gui_objects/Spline5.h \
	gui_objects/SplineFoil.h \
	viewwidgets/glWidgets/gl3dview.h \
	viewwidgets/glWidgets/gl3dbodyview.h \
	viewwidgets/glWidgets/gl3dmiarexview.h \
	viewwidgets/glWidgets/gl3dwingview.h \
	viewwidgets/glWidgets/gl3dplaneview.h \
	viewwidgets/glWidgets/ArcBall.h \
	misc/options/displayoptions.h \
	misc/options/language.h \
	misc/options/preferencesdlg.h \
	misc/options/saveoptions.h \
	misc/options/Units.h \
	misc/color/ColorButton.h \
	misc/text/DoubleEdit.h \
	misc/text/IntEdit.h \
	misc/text/TextClrBtn.h \
	misc/line/LineBtn.h \
	misc/line/LineCbBox.h \
	misc/line/LineDelegate.h \
	misc/line/LinePickerDlg.h \
	misc/text/MinTextEdit.h \
	misc/text/FloatEditDelegate.h \
	graph/graphdlg.h \
	graph/linestyle.h \
	graph/graph.h \
	graph/graph_globals.h \
	graph/curve.h


RESOURCES += \
	scripts.qrc \
	images.qrc \
	shaders.qrc \
	textures.qrc



INCLUDEPATH += $$PWD/viewwidgets
INCLUDEPATH += $$PWD/viewwidgets/glWidgets
INCLUDEPATH += $$PWD/graph
INCLUDEPATH += $$PWD/misc
INCLUDEPATH += $$PWD/xdirect/xfoil_task
INCLUDEPATH += $$PWD/glcontextinfo

# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../XFoil-lib/
INCLUDEPATH += $$PWD/../xflr5-engine/
# Forces re-build if a library header file has been modified
DEPENDPATH += $$PWD/../XFoil-lib/
DEPENDPATH += $$PWD/../xflr5-engine/


win32 {
	TARGET = xflr5

#   Specify here the directories where the shared library files XFoil.dll and Xfl5-engine.dll are located
#   The precise paths depend on QtCreator's settings
#   Alternate option is to compile the libraries separately and declare them in the system's PATH
#   Alternate option is to specify the absolute path instead of the relative path
#   Uncomment the following line to print the name of the variable OUT_PWD in the console
#message($$OUT_PWD)
	CONFIG(release, debug|release){
		LIBS += -L$$OUT_PWD/../../build-xflr5/xflr5-engine/release/ -lxflr5-engine
		LIBS += -L$$OUT_PWD/../../build-xflr5/XFoil-lib/release/ -lXFoil
	}
	else:CONFIG(debug, debug|release)
	{
		LIBS += -L$$OUT_PWD/../../build-xflr5/xflr5-engine/debug/ -lxflr5-engine
		LIBS += -L$$OUT_PWD/../../build-xflr5/XFoil-lib/debug/ -lXFoil
	}
	LIBS += -lopenGL32

	RC_FILE = ../win/xflr5.rc
}


unix{
	TARGET = xflr5
#   Specify here the directories where the shared library files XFoil.so and Xfl5-engine.so are located
#   The precise paths depend on QtCreator's settings
#   Alternate option is to compile the libraries separately and declare them in the system's PATH
#   Alternate option is to specify the absolute path instead of the relative path
#   Uncomment the following line to print the name of the variable OUT_PWD in the console
#message($$OUT_PWD)

	LIBS += -L$$OUT_PWD/../XFoil-lib/ -lXFoil
	LIBS += -L$$OUT_PWD/../xflr5-engine/ -lxflr5-engine

	# VARIABLES
	isEmpty(PREFIX):PREFIX = /usr
	BINDIR = $$PREFIX/bin
	DATADIR = $$PREFIX/share

	# MAKE INSTALL
	INSTALLS += target
	target.path = $$BINDIR
}


macx {
	CONFIG(release, debug|release) {
		OBJECTS_DIR = ./build/release
		MOC_DIR = ./build/release
		RCC_DIR = ./build/release
		UI_HEADERS_DIR = ./build/release
	}
	CONFIG(debug, debug|release) {
		OBJECTS_DIR = ./build/debug
		MOC_DIR = ./build/debug
		RCC_DIR = ./build/debug
		UI_HEADERS_DIR = ./build/debug
	}
	DESTDIR = ../
	TARGET = XFLR5
	TEMPLATE = app
	CONFIG += i386
	QMAKE_MAC_SDK = macosx
	#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
	#QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
	OTHER_FILES += ./mac/Info.plist

	LIBS += -framework \
		CoreFoundation

#   Specify here the directories where the shared library files XFoil.dylib and Xfl5-engine.dylib are located
#   The precise paths depend on QtCreator's settings
#   Alternate option is to compile the libraries separately and declare them in the system's PATH
#   Alternate option is to specify the absolute path instead of the relative path
#   Uncomment the following line to print the name of the variable OUT_PWD in the console
#message($$OUT_PWD)
	CONFIG(release, debug|release){
		LIBS += -L$$OUT_PWD/../xflr5-engine/build/ -lxflr5-engine
		LIBS += -L$$OUT_PWD/../XFoil-lib/build/ -lXFoil
	}
	else:CONFIG(debug, debug|release)
	{
		LIBS += -L$$OUT_PWD/../xflr5-engine/build/ -lxflr5-engine
		LIBS += -L$$OUT_PWD/../XFoil-lib/build/ -lXFoil
	}
	QMAKE_INFO_PLIST = ./mac/Info.plist
	ICON = ./mac/xflr5.icns
}

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4





