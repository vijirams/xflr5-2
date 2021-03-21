
SOURCES += \
    design/afoil.cpp \
    design/afoiltabledlg.cpp \
    design/foiltabledelegate.cpp \
    design/gridsettingsdlg.cpp \
    design/lecircledlg.cpp \
    design/splinectrlsdlg.cpp \
    glcontextinfo/opengldlg.cpp \
    globals/main.cpp \
    globals/mainframe.cpp \
    globals/xflr5app.cpp \
    graph/curve.cpp \
    graph/graph.cpp \
    graph/graph_globals.cpp \
    graph/graphdlg.cpp \
    gui_objects/spline5.cpp \
    gui_objects/splinefoil.cpp \
    miarex/analysis/aerodatadlg.cpp \
    miarex/analysis/ctrltabledelegate.cpp \
    miarex/analysis/editpolardefdlg.cpp \
    miarex/analysis/lltanalysisdlg.cpp \
    miarex/analysis/panelanalysisdlg.cpp \
    miarex/analysis/stabpolardlg.cpp \
    miarex/analysis/wadvanceddlg.cpp \
    miarex/analysis/wpolardlg.cpp \
    miarex/design/bodygriddlg.cpp \
    miarex/design/bodyscaledlg.cpp \
    miarex/design/bodytabledelegate.cpp \
    miarex/design/bodytransdlg.cpp \
    miarex/design/editbodydlg.cpp \
    miarex/design/editobjectdelegate.cpp \
    miarex/design/editplanedlg.cpp \
    miarex/design/gl3dbodydlg.cpp \
    miarex/design/gl3dwingdlg.cpp \
    miarex/design/inertiadlg.cpp \
    miarex/design/planedlg.cpp \
    miarex/design/wingdelegate.cpp \
    miarex/design/wingscaledlg.cpp \
    miarex/design/wingseldlg.cpp \
    miarex/mgt/importobjectdlg.cpp \
    miarex/mgt/manageplanesdlg.cpp \
    miarex/mgt/planetabledelegate.cpp \
    miarex/mgt/xmlplanereader.cpp \
    miarex/mgt/xmlplanewriter.cpp \
    miarex/mgt/xmlwpolarreader.cpp \
    miarex/mgt/xmlwpolarwriter.cpp \
    miarex/miarex.cpp \
    miarex/objects3d.cpp \
    miarex/view/gl3dscales.cpp \
    miarex/view/gllightdlg.cpp \
    miarex/view/stabviewdlg.cpp \
    miarex/view/targetcurvedlg.cpp \
    miarex/view/w3dprefsdlg.cpp \
    misc/aboutq5.cpp \
    misc/color/colorbutton.cpp \
    misc/editplrdlg.cpp \
    misc/exponentialslider.cpp \
    misc/lengthunitdlg.cpp \
    misc/line/linebtn.cpp \
    misc/line/linecbbox.cpp \
    misc/line/linedelegate.cpp \
    misc/line/linepickerdlg.cpp \
    misc/moddlg.cpp \
    misc/newnamedlg.cpp \
    misc/objectpropsdlg.cpp \
    misc/options/languagewt.cpp \
    misc/options/preferencesdlg.cpp \
    misc/options/saveoptions.cpp \
    misc/options/settings.cpp \
    misc/options/units.cpp \
    misc/polarfilterdlg.cpp \
    misc/popup.cpp \
    misc/renamedlg.cpp \
    misc/stlexportdialog.cpp \
    misc/text/doubleedit.cpp \
    misc/text/floateditdelegate.cpp \
    misc/text/intedit.cpp \
    misc/text/mintextedit.cpp \
    misc/text/numedit.cpp \
    misc/text/textclrbtn.cpp \
    misc/updater.cpp \
    misc/voidwidget.cpp \
    script/logwt.cpp \
    script/xflscriptexec.cpp \
    script/xflscriptreader.cpp \
    viewwidgets/glwidgets/arcball.cpp \
    viewwidgets/glwidgets/gl3dbodyview.cpp \
    viewwidgets/glwidgets/gl3dmiarexview.cpp \
    viewwidgets/glwidgets/gl3dplaneview.cpp \
    viewwidgets/glwidgets/gl3dtestview.cpp \
    viewwidgets/glwidgets/gl3dview.cpp \
    viewwidgets/glwidgets/gl3dwingview.cpp \
    viewwidgets/graphwidgets/graphtilewt.cpp \
    viewwidgets/graphwidgets/graphwt.cpp \
    viewwidgets/graphwidgets/legendwt.cpp \
    viewwidgets/graphwidgets/miarextilewt.cpp \
    viewwidgets/graphwidgets/xdirecttilewt.cpp \
    viewwidgets/inverseviewwt.cpp \
    viewwidgets/oppointwt.cpp \
    viewwidgets/twodwidgets/bodyframewt.cpp \
    viewwidgets/twodwidgets/bodylinewt.cpp \
    viewwidgets/twodwidgets/foildesignwt.cpp \
    viewwidgets/twodwidgets/foilwt.cpp \
    viewwidgets/twodwidgets/section2dwt.cpp \
    viewwidgets/wingwt.cpp \
    xdirect/analysis/batchdlg.cpp \
    xdirect/analysis/batchthreaddlg.cpp \
    xdirect/analysis/foilpolardlg.cpp \
    xdirect/analysis/relistdlg.cpp \
    xdirect/analysis/xfoiladvanceddlg.cpp \
    xdirect/analysis/xfoilanalysisdlg.cpp \
    xdirect/analysis/xfoiltask.cpp \
    xdirect/geometry/cadddlg.cpp \
    xdirect/geometry/flapdlg.cpp \
    xdirect/geometry/foilcoorddlg.cpp \
    xdirect/geometry/foilgeomdlg.cpp \
    xdirect/geometry/interpolatefoilsdlg.cpp \
    xdirect/geometry/ledlg.cpp \
    xdirect/geometry/nacafoildlg.cpp \
    xdirect/geometry/tegapdlg.cpp \
    xdirect/geometry/twodpaneldlg.cpp \
    xdirect/mgt/managefoilsdlg.cpp \
    xdirect/objects2d.cpp \
    xdirect/optim2d/gatask.cpp \
    xdirect/optim2d/mopsotask.cpp \
    xdirect/optim2d/mopsotask2d.cpp \
    xdirect/optim2d/optim2d.cpp \
    xdirect/optim2d/optimtask.cpp \
    xdirect/optim2d/particle.cpp \
    xdirect/xdirect.cpp \
    xdirect/xdirectstyledlg.cpp \
    xdirect/xml/xmlpolarreader.cpp \
    xdirect/xml/xmlpolarwriter.cpp \
    xflanalysis/analysis3d_globals.cpp \
    xflanalysis/matrix.cpp \
    xflanalysis/plane_analysis/lltanalysis.cpp \
    xflanalysis/plane_analysis/panelanalysis.cpp \
    xflanalysis/plane_analysis/planetask.cpp \
    xflcore/trace.cpp \
    xflcore/xflcore.cpp \
    xflobjects/objects2d/blxfoil.cpp \
    xflobjects/objects2d/foil.cpp \
    xflobjects/objects2d/opppoint.cpp \
    xflobjects/objects2d/polar.cpp \
    xflobjects/objects2d/spline.cpp \
    xflobjects/objects2d/vector2d.cpp \
    xflobjects/objects3d/body.cpp \
    xflobjects/objects3d/frame.cpp  \
    xflobjects/objects3d/nurbssurface.cpp  \
    xflobjects/objects3d/panel.cpp \
    xflobjects/objects3d/plane.cpp \
    xflobjects/objects3d/planeopp.cpp \
    xflobjects/objects3d/quaternion.cpp \
    xflobjects/objects3d/surface.cpp \
    xflobjects/objects3d/vector3d.cpp \
    xflobjects/objects3d/wing.cpp \
    xflobjects/objects3d/wingopp.cpp \
    xflobjects/objects3d/wpolar.cpp \
    xflobjects/objects_global.cpp \
    xinverse/foilselectiondlg.cpp \
    xinverse/inverseoptionsdlg.cpp \
    xinverse/pertdlg.cpp \
    xinverse/xinverse.cpp \


HEADERS += \
    $$PWD/xflcore/linestyle.h \
    $$PWD/xflcore/ls2.h \
    $$PWD/xflobjects/xflobject.h \
    design/afoil.h \
    design/afoiltabledlg.h \
    design/foiltabledelegate.h \
    design/gridsettingsdlg.h \
    design/lecircledlg.h \
    design/splinectrlsdlg.h \
    glcontextinfo/opengldlg.h \
    globals/mainframe.h \
    globals/xflr5app.h \
    graph/curve.h \
    graph/graph.h \
    graph/graph_globals.h \
    graph/graphdlg.h \
    gui_objects/spline5.h \
    gui_objects/splinefoil.h \
    miarex/analysis/aerodatadlg.h \
    miarex/analysis/ctrltabledelegate.h \
    miarex/analysis/editpolardefdlg.h \
    miarex/analysis/lltanalysisdlg.h \
    miarex/analysis/panelanalysisdlg.h \
    miarex/analysis/stabpolardlg.h \
    miarex/analysis/wadvanceddlg.h \
    miarex/analysis/wpolardlg.h \
    miarex/design/bodygriddlg.h \
    miarex/design/bodyscaledlg.h \
    miarex/design/bodytabledelegate.h \
    miarex/design/bodytransdlg.h \
    miarex/design/editbodydlg.h \
    miarex/design/editobjectdelegate.h \
    miarex/design/editplanedlg.h \
    miarex/design/gl3dbodydlg.h \
    miarex/design/gl3dwingdlg.h \
    miarex/design/inertiadlg.h \
    miarex/design/planedlg.h \
    miarex/design/wingdelegate.h \
    miarex/design/wingscaledlg.h \
    miarex/design/wingseldlg.h \
    miarex/mgt/importobjectdlg.h \
    miarex/mgt/manageplanesdlg.h \
    miarex/mgt/planetabledelegate.h \
    miarex/mgt/xmlplanereader.h \
    miarex/mgt/xmlplanewriter.h \
    miarex/mgt/xmlwpolarreader.h \
    miarex/mgt/xmlwpolarwriter.h \
    miarex/miarex.h \
    miarex/objects3d.h \
    miarex/view/gl3dscales.h \
    miarex/view/gllightdlg.h \
    miarex/view/stabviewdlg.h \
    miarex/view/targetcurvedlg.h \
    miarex/view/w3dprefsdlg.h \
    misc/aboutq5.h \
    misc/color/colorbutton.h \
    misc/editplrdlg.h \
    misc/exponentialslider.h \
    misc/lengthunitdlg.h \
    misc/line/linebtn.h \
    misc/line/linecbbox.h \
    misc/line/linedelegate.h \
    misc/line/linepickerdlg.h \
    misc/moddlg.h \
    misc/newnamedlg.h \
    misc/objectpropsdlg.h \
    misc/options/languagewt.h \
    misc/options/preferencesdlg.h \
    misc/options/saveoptions.h \
    misc/options/settings.h \
    misc/options/units.h \
    misc/polarfilterdlg.h \
    misc/popup.h \
    misc/renamedlg.h \
    misc/stlexportdialog.h \
    misc/text/doubleedit.h \
    misc/text/floateditdelegate.h \
    misc/text/intedit.h \
    misc/text/mintextedit.h \
    misc/text/numedit.h \
    misc/text/textclrbtn.h \
    misc/updater.h \
    misc/voidwidget.h \
    script/logwt.h \
    script/xflscriptexec.h \
    script/xflscriptreader.h \
    viewwidgets/glwidgets/arcball.h \
    viewwidgets/glwidgets/gl3dbodyview.h \
    viewwidgets/glwidgets/gl3dmiarexview.h \
    viewwidgets/glwidgets/gl3dplaneview.h \
    viewwidgets/glwidgets/gl3dtestview.h \
    viewwidgets/glwidgets/gl3dview.h \
    viewwidgets/glwidgets/gl3dwingview.h \
    viewwidgets/graphwidgets/graphtilewt.h \
    viewwidgets/graphwidgets/graphwt.h \
    viewwidgets/graphwidgets/legendwt.h \
    viewwidgets/graphwidgets/miarextilewt.h \
    viewwidgets/graphwidgets/xdirecttilewt.h \
    viewwidgets/inverseviewwt.h \
    viewwidgets/oppointwt.h \
    viewwidgets/twodwidgets/bodyframewt.h \
    viewwidgets/twodwidgets/bodylinewt.h \
    viewwidgets/twodwidgets/foildesignwt.h \
    viewwidgets/twodwidgets/foilwt.h \
    viewwidgets/twodwidgets/section2dwt.h \
    viewwidgets/wingwt.h \
    xdirect/analysis/batchdlg.h \
    xdirect/analysis/batchthreaddlg.h \
    xdirect/analysis/foilpolardlg.h \
    xdirect/analysis/relistdlg.h \
    xdirect/analysis/xfoiladvanceddlg.h \
    xdirect/analysis/xfoilanalysisdlg.h \
    xdirect/analysis/xfoiltask.h \
    xdirect/geometry/cadddlg.h \
    xdirect/geometry/flapdlg.h \
    xdirect/geometry/foilcoorddlg.h \
    xdirect/geometry/foilgeomdlg.h \
    xdirect/geometry/interpolatefoilsdlg.h \
    xdirect/geometry/ledlg.h \
    xdirect/geometry/nacafoildlg.h \
    xdirect/geometry/tegapdlg.h \
    xdirect/geometry/twodpaneldlg.h \
    xdirect/mgt/managefoilsdlg.h \
    xdirect/objects2d.h \
    xdirect/optim2d/gatask.h \
    xdirect/optim2d/mopsotask.h \
    xdirect/optim2d/mopsotask2d.h \
    xdirect/optim2d/optim2d.h \
    xdirect/optim2d/optimevent.h \
    xdirect/optim2d/optimtask.h \
    xdirect/optim2d/optstructures.h \
    xdirect/optim2d/particle.h \
    xdirect/xdirect.h \
    xdirect/xdirectstyledlg.h \
    xdirect/xml/xmlpolarreader.h \
    xdirect/xml/xmlpolarwriter.h \
    xflanalysis/analysis3d_globals.h \
    xflanalysis/analysis3d_params.h \
    xflanalysis/matrix.h \
    xflanalysis/plane_analysis/lltanalysis.h \
    xflanalysis/plane_analysis/panelanalysis.h \
    xflanalysis/plane_analysis/planetask.h \
    xflanalysis/plane_analysis/planetaskevent.h \
    xflcore/constants.h \
    xflcore/core_enums.h \
    xflcore/gui_enums.h \
    xflcore/gui_params.h \
    xflcore/trace.h \
    xflcore/xflcore.h \
    xflcore/xflevents.h \
    xflobjects/objects2d/blxfoil.h \
    xflobjects/objects2d/foil.h \
    xflobjects/objects2d/oppoint.h \
    xflobjects/objects2d/polar.h \
    xflobjects/objects2d/spline.h \
    xflobjects/objects2d/vector2d.h \
    xflobjects/objects3d/body.h \
    xflobjects/objects3d/frame.h  \
    xflobjects/objects3d/nurbssurface.h  \
    xflobjects/objects3d/panel.h \
    xflobjects/objects3d/plane.h \
    xflobjects/objects3d/planeopp.h \
    xflobjects/objects3d/pointmass.h \
    xflobjects/objects3d/quaternion.h \
    xflobjects/objects3d/surface.h \
    xflobjects/objects3d/vector3d.h \
    xflobjects/objects3d/wing.h \
    xflobjects/objects3d/wingopp.h \
    xflobjects/objects3d/wingsection.h \
    xflobjects/objects3d/wpolar.h \
    xflobjects/objects_global.h \
    xinverse/foilselectiondlg.h \
    xinverse/inverseoptionsdlg.h \
    xinverse/pertdlg.h \
    xinverse/xinverse.h \


RESOURCES += \
    scripts.qrc \
    images.qrc \
    shaders.qrc \
    textures.qrc


