/****************************************************************************

	Miarex
			Copyright (C) 2008-2017 Andre Deperrois adeperrois@xflr5.com

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#include <QtDebug>
#include <QApplication>
#include <QDockWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QAction>
#include <QFileDialog>
#include <QColorDialog>
#include <QDesktopServices>
#include <math.h>

#include "Miarex.h"
#include <mainframe.h>
#include <globals.h>
#include "./Objects3D.h"
#include "./mgt/ManagePlanesDlg.h"
#include "./mgt/XmlPlaneReader.h"
#include "./mgt/XmlPlaneWriter.h"
#include "./view/StabViewDlg.h"
#include "./view/W3dPrefsDlg.h"
#include "./view/TargetCurveDlg.h"
#include "./design/EditPlaneDlg.h"
#include "./design/EditBodyDlg.h"
#include "./design/PlaneDlg.h"
#include "./design/GL3dWingDlg.h"
#include "./design/GL3dBodyDlg.h"
#include "./design/WingScaleDlg.h"
#include "./design/InertiaDlg.h"
#include "./analysis/AeroDataDlg.h"
#include "./analysis/PanelAnalysisDlg.h"
#include "./analysis/StabPolarDlg.h"
#include "./analysis/WAdvancedDlg.h"
#include "./analysis/WPolarDlg.h"
#include "./analysis/StabPolarDlg.h"
#include "./analysis/EditPolarDefDlg.h"
#include <miarex/mgt/xmlwpolarreader.h>
#include <miarex/mgt/xmlwpolarwriter.h>
#include <objects3d/PointMass.h>
#include <objects3d/Surface.h>
#include <misc/Settings.h>
#include <misc/ProgressDlg.h>
#include <misc/ModDlg.h>
#include <misc/RenameDlg.h>
#include <misc/PolarFilterDlg.h>
#include <misc/ObjectPropsDlg.h>
#include <misc/Units.h>
#include <misc/EditPlrDlg.h>
#include <misc/stlexportdialog.h>
#include <viewwidgets/miarextilewidget.h>
#include <objects_global.h>
#include <xdirect/objects2d.h>
#include <matrix.h>


#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif


MainFrame *QMiarex::s_pMainFrame = NULL;

bool QMiarex::s_bAutoCpScale = true;
double QMiarex::s_LegendMin = -1.0;
double QMiarex::s_LegendMax =  1.0;

double QMiarex::s_LiftScale     = 1.0;
double QMiarex::s_VelocityScale = 1.0;
double QMiarex::s_DragScale     = 1.0;


bool QMiarex::m_bResetglGeom = true;
bool QMiarex::m_bResetglMesh = true;
bool QMiarex::m_bResetglWake = true;
bool QMiarex::m_bResetglOpp = true;
bool QMiarex::m_bResetglLift = true;
bool QMiarex::m_bResetglDrag = true;
bool QMiarex::m_bResetglDownwash = true;
bool QMiarex::m_bResetglPanelForce = true;
bool QMiarex::m_bResetglPanelCp = true;
bool QMiarex::m_bResetglStream = true;
bool QMiarex::m_bResetglLegend = true;
bool QMiarex::m_bResetglBody = true;
bool QMiarex::m_bResetglBodyMesh = true;
bool QMiarex::m_bResetglSurfVelocities = true;

bool QMiarex::s_bResetCurves = true;
bool QMiarex::m_bLogFile = true;

QList<void *> *QMiarex::m_poaPlane = NULL;
QList<void *> *QMiarex::m_poaWPolar = NULL;
QList<void *> *QMiarex::m_poaPOpp = NULL;


/**
 * The public constructor.
 *
 * @param parent: a pointer to the parent window
 */
QMiarex::QMiarex(QWidget *parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_theLLTAnalysis.m_poaPolar = &Objects2D::s_oaPolar;

	//construct and initialize everything
/*	int memsize=0;
	if(!Objects3D::AllocatePanelArrays(memsize))
	{
		QString strange = tr("This computer does not have enough RAM to run xflr5");
		Trace(strange);
		QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
	}
	else
	{
		QString strange = QString("QMiarex::Initial memory allocation for PanelAnalysis is %1 MB").arg((double)memsize/1024./1024., 7, 'f', 2);
		Trace(strange);
	}*/

	m_theTask.m_ptheLLTAnalysis = &m_theLLTAnalysis;
	m_theTask.m_pthePanelAnalysis = &m_thePanelAnalysis;

	m_pLLTDlg = new LLTAnalysisDlg(this);
	m_pPanelAnalysisDlg = new PanelAnalysisDlg(s_pMainFrame);

	connect(m_theTask.m_pthePanelAnalysis, SIGNAL(outputMsg(QString)), m_pPanelAnalysisDlg, SLOT(onMessage(QString)), Qt::QueuedConnection);
	connect(m_theTask.m_ptheLLTAnalysis, SIGNAL(outputMsg(QString)), m_pLLTDlg, SLOT(onMessage(QString)), Qt::QueuedConnection);

	m_PixText = QPixmap(107, 97);
	m_PixText.fill(Qt::transparent);

	m_pCurPlane   = NULL;
	m_pCurPOpp    = NULL;
	m_pCurWPolar  = NULL;

	Wing::s_poaFoil  = &Objects2D::s_oaFoil;
	Wing::s_poaPolar = &Objects2D::s_oaPolar;

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		m_pWOpp[iw]     = NULL;
	}


	m_bXPressed = m_bYPressed = false;
	m_bXCmRef            = true;
	m_bXCP               = false;
	m_bXTop              = false;
	m_bXBot              = false;
	m_b3DCp              = false;
	m_bDownwash          = false;
	m_bMoments           = false;
	m_bICd               = false;
	m_bVCd               = false;
	m_bTrans             = false;
	m_bInitLLTCalc       = true;
	m_bTransGraph        = true;
	m_bPanelForce        = false;
	m_bLongitudinal      = true;
	m_bCurPOppOnly       = true;
	m_bCurFrameOnly      = true;
	m_bType1 = m_bType2 = m_bType4 = m_bType7 = true;
	m_bShowEllipticCurve = false;
	m_bShowBellCurve     = false;
	m_bShowWingCurve[0] = m_bShowWingCurve[1] = m_bShowWingCurve[2] = m_bShowWingCurve[3] = true;
	m_bAnimateWOpp       = false;
	m_bAnimateWOppPlus   = true;
	m_bAnimateMode       = false;
	m_bStream            = false;
	m_bSurfVelocities    = false;
	m_bCrossPoint        = false;
	m_bPickCenter        = false;
	m_bShowCpScale       = true;
	m_bIs2DScaleSet      = false;
	m_bResetTextLegend   = true;
	m_bShowFlapMoments   = true;


	m_LLTMaxIterations          = 100;
	LLTAnalysis::s_CvPrec       =   0.01;
	LLTAnalysis::s_RelaxMax     =  20.0;
	LLTAnalysis::s_NLLTStations = 20;

	Panel::s_VortexPos = 0.25;
	Panel::s_CtrlPos   = 0.75;

	m_LineStyle.m_Style = 0;
	m_LineStyle.m_Width = 1;
	m_LineStyle.m_PointStyle  = 0;
	m_LineStyle.m_Color = QColor(127, 255, 70);
	m_bCurveVisible = true;

	m_WakeInterNodes  = 6;
	m_bResetWake      = true;
	m_bSequence       = false;

	m_bDirichlet = true;

	m_CurSpanPos    = 0.0;

	m_AlphaMin     =  0.0;
	m_AlphaMax     =  1.0;
	m_AlphaDelta   =  0.5;
	m_BetaMin      =  0.0;
	m_BetaMax      =  1.0;
	m_BetaDelta    =  0.5;
	m_QInfMin      = 10.0;
	m_QInfMax      = 50.0;
	m_QInfDelta    = 10.0;
	m_ControlMin   =  0.0;
	m_ControlMax   =  1.0;
	m_ControlDelta =  0.1;

	m_ModeNorm = 1.0;
	m_ModeTime = 0.0;
	m_Modedt   = 0.01;

	m_LastAlpha = 0.0;
	m_LastBeta = 0.0;


	m_InducedDragPoint = 0;

	m_pTimerWOpp= new QTimer(this);
	m_posAnimateWOpp         = 0;

	m_pTimerMode= new QTimer(this);
	m_posAnimateMode         = 0;

	memset(m_ModeState, 0, 6*sizeof(double));
	m_TimeInput[0] = m_TimeInput[1] = m_TimeInput[2] = m_TimeInput[3] = 0.0;
	m_TotalTime = 10;//s
	m_Deltat    = 0.1;//s

	m_RampTime = .1;//s
	m_RampAmplitude = 1.;//CtrlUnit;

	m_WingGraph.clear();
	for(int ig=0; ig<MAXWINGGRAPHS; ig++)
	{
		m_WingGraph.append(new QGraph);
		m_WingGraph[ig]->setGraphName(QString("Wing_Graph_%1").arg(ig));
		m_WingGraph[ig]->graphType() = QGRAPH::POPPGRAPH;
		m_WingGraph[ig]->setAutoX(true);
		m_WingGraph[ig]->setXUnit(2.0);
		m_WingGraph[ig]->setXMin(-1.0);
		m_WingGraph[ig]->setXMax( 1.0);
		m_WingGraph[ig]->setYMin(0.000);
		m_WingGraph[ig]->setYMax(0.001);
		m_WingGraph[ig]->setXMajGrid(true, QColor(120,120,120),2,1);
		m_WingGraph[ig]->setXMinGrid(false, true, QColor(80,80,80),2, 1, 100.0);
		m_WingGraph[ig]->setYMajGrid(true, QColor(120,120,120),2,1);
		m_WingGraph[ig]->setYMinGrid(false, true, QColor(80,80,80),2, 1, 0.1);
		m_WingGraph[ig]->setType(1);
		m_WingGraph[ig]->setMargin(50);
		m_WingGraph[ig]->setYVariable(ig);
	}

	m_WPlrGraph.clear();
	for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
	{
		m_WPlrGraph.append(new QGraph);
		m_WPlrGraph[ig]->graphType() = QGRAPH::WPOLARGRAPH;
		m_WPlrGraph[ig]->setGraphName(QString("Wing_Polar_Graph_%1").arg(ig));
		m_WPlrGraph[ig]->setXMajGrid(true, QColor(120,120,120),2,1);
		m_WPlrGraph[ig]->setYMajGrid(true, QColor(120,120,120),2,1);
		m_WPlrGraph[ig]->setXMin(-0.0);
		m_WPlrGraph[ig]->setXMax( 0.1);
		m_WPlrGraph[ig]->setYMin(-0.01);
		m_WPlrGraph[ig]->setYMax( 0.01);
		m_WPlrGraph[ig]->setType(0);
		m_WPlrGraph[ig]->setMargin(50);
		m_WPlrGraph.at(ig)->setOppHighlighting(true);
	}

	m_WPlrGraph[0]->setVariables(3,2);
	m_WPlrGraph[1]->setVariables(0,2);
	m_WPlrGraph[2]->setVariables(0,7);
	m_WPlrGraph[3]->setVariables(0,14);
	m_WPlrGraph[4]->setVariables(0,15);

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++) setWGraphTitles(m_WPlrGraph[ig]);


	m_CpGraph.graphType() = QGRAPH::CPGRAPH;
	m_CpGraph.setXMajGrid(true, QColor(120,120,120),2,1);
	m_CpGraph.setYMajGrid(true, QColor(120,120,120),2,1);
	m_CpGraph.setXTitle(tr("x"));
	m_CpGraph.setYTitle(tr("Cp"));
	m_CpGraph.setXMin( 0.0);
	m_CpGraph.setXMax( 0.1);
	m_CpGraph.setYMin(-0.01);
	m_CpGraph.setYMax( 0.01);
	m_CpGraph.setType(0);
	m_CpGraph.setMargin(50);
	m_CpGraph.setInverted(true);

	for(int i=0; i<MAXWINGS;i++) m_CpGraph.addCurve(); // four curves, one for each of the plane's wings

	//set the default settings for the time response graphs
	m_TimeGraph.clear();
	for(int ig=0; ig<MAXTIMEGRAPHS; ig++)
	{
		m_TimeGraph.append(new QGraph);
		m_TimeGraph[ig]->graphType() = QGRAPH::STABTIMEGRAPH;
		m_TimeGraph[ig]->setXMajGrid(true, QColor(120,120,120),2,1);
		m_TimeGraph[ig]->setYMajGrid(true, QColor(120,120,120),2,1);
		m_TimeGraph[ig]->setXTitle("s");
		m_TimeGraph[ig]->setXMin( 0.0);
		m_TimeGraph[ig]->setXMax( 0.1);
		m_TimeGraph[ig]->setYMin(-0.01);
		m_TimeGraph[ig]->setYMax( 0.01);
		m_TimeGraph[ig]->setType(0);
		m_TimeGraph[ig]->setMargin(50);
		m_TimeGraph[ig]->setInverted(false);
		m_TimeGraph[ig]->setGraphName("Time Response");
	}

	//set the axis labels for the time graphs
	if(m_bLongitudinal)
	{
		m_TimeGraph[0]->setYTitle("u (m/s)");
		m_TimeGraph[1]->setYTitle("w (m/s)");
		m_TimeGraph[2]->setYTitle("q ("+QString::fromUtf8("°") +"/s)");
		m_TimeGraph[3]->setYTitle("theta ("+QString::fromUtf8("°") +"/s)");
	}
	else
	{
		m_TimeGraph[0]->setYTitle("v (m/s)");
		m_TimeGraph[1]->setYTitle("p ("+QString::fromUtf8("°") +"/s)");
		m_TimeGraph[2]->setYTitle("r ("+QString::fromUtf8("°") +"/s)");
		m_TimeGraph[3]->setYTitle("phi ("+QString::fromUtf8("°") +"/s)");
	}

	//set the default settings for the root locus graphs
	m_StabPlrGraph.append(new QGraph);   // the longitudinal graph
	m_StabPlrGraph.append(new QGraph);   // the lateral graph
	m_StabPlrGraph.at(0)->graphType() = QGRAPH::WPOLARGRAPH;
	m_StabPlrGraph.at(0)->setXMajGrid(true, QColor(120,120,120),2,1);
	m_StabPlrGraph.at(0)->setYMajGrid(true, QColor(120,120,120),2,1);
	m_StabPlrGraph.at(0)->setXTitle(tr("Real"));
	m_StabPlrGraph.at(0)->setYTitle(tr("Imag/2.pi"));
	m_StabPlrGraph.at(0)->setXMin( 0.0);
	m_StabPlrGraph.at(0)->setXMax( 0.1);
	m_StabPlrGraph.at(0)->setYMin(-0.01);
	m_StabPlrGraph.at(0)->setYMax( 0.01);
	m_StabPlrGraph.at(0)->setType(0);
	m_StabPlrGraph.at(0)->setMargin(50);
	m_StabPlrGraph.at(0)->setInverted(false);
	m_StabPlrGraph.at(0)->setGraphName("Longitudinal Modes");
	m_StabPlrGraph.at(0)->setOppHighlighting(true);

	m_StabPlrGraph.at(1)->graphType() = QGRAPH::WPOLARGRAPH;
	m_StabPlrGraph.at(1)->setXMajGrid(true, QColor(120,120,120),2,1);
	m_StabPlrGraph.at(1)->setYMajGrid(true, QColor(120,120,120),2,1);
	m_StabPlrGraph.at(1)->setXTitle(tr("Real"));
	m_StabPlrGraph.at(1)->setYTitle(tr("Imag/2.pi"));
	m_StabPlrGraph.at(1)->setXMin( 0.0);
	m_StabPlrGraph.at(1)->setXMax( 0.1);
	m_StabPlrGraph.at(1)->setYMin(-0.01);
	m_StabPlrGraph.at(1)->setYMax( 0.01);
	m_StabPlrGraph.at(1)->setType(0);
	m_StabPlrGraph.at(1)->setMargin(50);
	m_StabPlrGraph.at(1)->setInverted(false);
	m_StabPlrGraph.at(1)->setGraphName("Lateral Modes");
	m_StabPlrGraph.at(1)->setOppHighlighting(true);


	m_CpLineStyle.m_Color = QColor(255,100,150);
	m_CpLineStyle.m_Style = 0;
	m_CpLineStyle.m_Width = 1;
	m_CpLineStyle.m_PointStyle = 0;
	m_bShowCp       = true;

	m_iView          = XFLR5::WOPPVIEW;
	m_iWingView      = XFLR5::ONEGRAPH;
	m_iWPlrView      = XFLR5::FOURGRAPHS;
	m_iRootLocusView = XFLR5::ONEGRAPH;
	m_iStabTimeView  = XFLR5::FOURGRAPHS;

	m_CpGraph.setGraphName(tr("Cp Graph"));

	s_LiftScale = s_DragScale = s_VelocityScale = 0.7;

	m_StabilityResponseType = 0;

	m_BellCurveExp = 1;
	m_bMaxCL = true;

	setupLayout();
}



/**
 * The public destructor.
 */
QMiarex::~QMiarex()
{
	if(m_pLLTDlg) delete m_pLLTDlg;
	if(m_pPanelAnalysisDlg) delete m_pPanelAnalysisDlg;

	Objects3D::deleteObjects();

	for(int ig=m_WingGraph.count()-1; ig>=0; ig--)
	{
		delete m_WingGraph.at(ig);
		m_WingGraph.removeAt(ig);
	}

	for(int ig=m_WPlrGraph.count()-1; ig>=0; ig--)
	{
		delete m_WPlrGraph.at(ig);
		m_WPlrGraph.removeAt(ig);
	}

	for(int ig=m_StabPlrGraph.count()-1; ig>=0; ig--)
	{
		delete m_StabPlrGraph.at(ig);
		m_StabPlrGraph.removeAt(ig);
	}

	for(int ig=m_TimeGraph.count()-1; ig>=0; ig--)
	{
		delete m_TimeGraph.at(ig);
		m_TimeGraph.removeAt(ig);
	}
}



/**
 * Connect signals and slots
 */
void QMiarex::connectSignals()
{
	connect(this, SIGNAL(projectModified()), s_pMainFrame, SLOT(onProjectModified()));

	connect(m_pctrlSequence, SIGNAL(clicked()), this, SLOT(onSequence()));
	connect(m_pctrlStoreWOpp, SIGNAL(clicked()), this, SLOT(onStoreWOpp()));
	connect(m_pctrlInitLLTCalc, SIGNAL(clicked()), this, SLOT(onInitLLTCalc()));
	connect(m_pctrlAnalyze, SIGNAL(clicked()), this, SLOT(onAnalyze()));
	connect(m_pctrlCurveStyle, SIGNAL(activated(int)), this, SLOT(onCurveStyle(int)));
	connect(m_pctrlCurveWidth, SIGNAL(activated(int)), this, SLOT(onCurveWidth(int)));
	connect(m_pctrlCurvePoints, SIGNAL(activated(int)), this, SLOT(onCurvePoints(int)));
	connect(m_pctrlCurveColor, SIGNAL(clickedLB()), this, SLOT(onCurveColor()));
	connect(m_pctrlShowCurve, SIGNAL(clicked()), this, SLOT(onShowCurve()));

	connect(m_pctrlPanelForce, SIGNAL(clicked()), this, SLOT(onPanelForce()));
	connect(m_pctrlLift, SIGNAL(clicked()), this, SLOT(onShowLift()));
	connect(m_pctrlIDrag, SIGNAL(clicked()), this, SLOT(onShowIDrag()));
	connect(m_pctrlVDrag, SIGNAL(clicked()), this, SLOT(onShowVDrag()));
	connect(m_pctrlTrans, SIGNAL(clicked()), this, SLOT(onShowTransitions()));
	connect(m_pctrlCp, SIGNAL(clicked()),this, SLOT(on3DCp()));
	connect(m_pctrlMoment, SIGNAL(clicked()), this, SLOT(onMoment()));
	connect(m_pctrlDownwash, SIGNAL(clicked()), this, SLOT(onDownwash()));
	connect(m_pctrlStream, SIGNAL(clicked()), this, SLOT(onStreamlines()));
	connect(m_pctrlSurfVel, SIGNAL(clicked()), this, SLOT(onSurfaceSpeeds()));

	connect(m_pctrlWOppAnimate, SIGNAL(clicked()), this, SLOT(onAnimateWOpp()));
	connect(m_pctrlAnimateWOppSpeed, SIGNAL(sliderMoved(int)), this, SLOT(onAnimateWOppSpeed(int)));
	connect(m_pTimerWOpp, SIGNAL(timeout()), this, SLOT(onAnimateWOppSingle()));
	connect(m_pTimerMode, SIGNAL(timeout()), this, SLOT(onAnimateModeSingle()));

	connect(m_pctrlSurfaces,  SIGNAL(clicked(bool)), m_pGL3dView, SLOT(onSurfaces(bool)));
	connect(m_pctrlOutline,   SIGNAL(clicked(bool)), m_pGL3dView, SLOT(onOutline(bool)));
	connect(m_pctrlPanels,    SIGNAL(clicked(bool)), m_pGL3dView, SLOT(onPanels(bool)));
	connect(m_pctrlFoilNames, SIGNAL(clicked(bool)), m_pGL3dView, SLOT(onFoilNames(bool)));
	connect(m_pctrlMasses,    SIGNAL(clicked(bool)), m_pGL3dView, SLOT(onShowMasses(bool)));



	connect(m_pctrlKeepCpSection,   SIGNAL(clicked()),         this, SLOT(onKeepCpSection()));
	connect(m_pctrlResetCpSection,  SIGNAL(clicked()),         this, SLOT(onResetCpSection()));
	connect(m_pctrlCpSectionSlider, SIGNAL(sliderMoved(int)),  this, SLOT(onCpSectionSlider(int)));
	connect(m_pctrlSpanPos,         SIGNAL(editingFinished()), this, SLOT(onCpPosition()));

	connect(m_pctrlAxes,  SIGNAL(clicked(bool)), m_pGL3dView, SLOT(onAxes(bool)));
	connect(m_pctrlX,     SIGNAL(clicked()),     m_pGL3dView, SLOT(on3DFront()));
	connect(m_pctrlY,     SIGNAL(clicked()),     m_pGL3dView, SLOT(on3DLeft()));
	connect(m_pctrlZ,     SIGNAL(clicked()),     m_pGL3dView, SLOT(on3DTop()));
	connect(m_pctrlIso,   SIGNAL(clicked()),     m_pGL3dView, SLOT(on3DIso()));
	connect(m_pctrlFlip,  SIGNAL(clicked()),     m_pGL3dView, SLOT(on3DFlip()));
	connect(m_pctrlClipPlanePos, SIGNAL(sliderMoved(int)), m_pGL3dView, SLOT(onClipPlane(int)));

	connect(m_pctrl3DResetScale, SIGNAL(clicked()), this, SLOT(on3DResetScale()));

	connect(m_pctrlAlphaMin,   SIGNAL(editingFinished()), this, SLOT(onReadAnalysisData()));
	connect(m_pctrlAlphaMax,   SIGNAL(editingFinished()), this, SLOT(onReadAnalysisData()));
	connect(m_pctrlAlphaDelta, SIGNAL(editingFinished()), this, SLOT(onReadAnalysisData()));

	connect(m_pGL3dView, SIGNAL(viewModified()), this, SLOT(onCheckViewIcons()));
}


/**
 * Unselects all the 3D-view icons.
 */
void QMiarex::onCheckViewIcons()
{
	m_pctrlIso->setChecked(false);
	m_pctrlX->setChecked(false);
	m_pctrlY->setChecked(false);
	m_pctrlZ->setChecked(false);
}



/**
* Checks and enables all buttons and actions
* depending on the currently active objects
*/
void QMiarex::setControls()
{
	blockSignals(true);

	if(m_iView==XFLR5::W3DVIEW) m_pctrlBottomControls->setCurrentIndex(1);
	else                        m_pctrlBottomControls->setCurrentIndex(0);

	if(m_iView==XFLR5::WPOLARVIEW)     m_pctrlMiddleControls->setCurrentIndex(1);
	else if(m_iView==XFLR5::WCPVIEW)   m_pctrlMiddleControls->setCurrentIndex(2);
	else if(m_iView==XFLR5::STABTIMEVIEW || m_iView==XFLR5::STABPOLARVIEW) m_pctrlMiddleControls->setCurrentIndex(1);
	else                                                                   m_pctrlMiddleControls->setCurrentIndex(0);

	if (m_iView==XFLR5::W3DVIEW && (m_pCurWPolar && m_pCurWPolar->isStabilityPolar()))
		 s_pMainFrame->m_pctrlStabViewWidget->show();
	else if (m_iView==XFLR5::STABTIMEVIEW || m_iView==XFLR5::STABPOLARVIEW)
		 s_pMainFrame->m_pctrlStabViewWidget->show();
	else s_pMainFrame->m_pctrlStabViewWidget->hide();

	StabViewDlg *pStabView = (StabViewDlg*)s_pMainFrame->m_pStabView;
	pStabView->setControls();


	if(m_pCurWPolar)
	{
		QString PolarProps;
		getPolarProperties(m_pCurWPolar, PolarProps);
		m_pctrlPolarProps->setText(PolarProps);
	}
	else
	{
		m_pctrlPolarProps->clear();
	}

	m_pctrlInitLLTCalc->setEnabled(m_pCurWPolar && m_pCurWPolar->analysisMethod()==XFLR5::LLTMETHOD);

	s_pMainFrame->m_pWOppAct->setChecked(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pWPolarAct->setChecked(m_iView==XFLR5::WPOLARVIEW);
	s_pMainFrame->m_pW3DAct->setChecked(m_iView==XFLR5::W3DVIEW);
	s_pMainFrame->m_pCpViewAct->setChecked(m_iView==XFLR5::WCPVIEW);

	s_pMainFrame->m_pStabTimeAct->setChecked(m_iView==XFLR5::STABTIMEVIEW);
	s_pMainFrame->m_pRootLocusAct->setChecked(m_iView==XFLR5::STABPOLARVIEW);

	s_pMainFrame->m_pShowWing2Curve->setChecked(m_bShowWingCurve[1]);
	s_pMainFrame->m_pShowStabCurve->setChecked(m_bShowWingCurve[2]);
	s_pMainFrame->m_pShowFinCurve->setChecked(m_bShowWingCurve[3]);

	s_pMainFrame->m_pShowCurWOppOnly->setChecked(m_bCurPOppOnly);

	s_pMainFrame->m_pShowFlapMoments->setChecked(m_bShowFlapMoments);

	m_pctrlAnalyze->setEnabled(m_pCurWPolar);
	m_pctrlAlphaMin->setEnabled(m_pCurWPolar);
	m_pctrlAlphaMax->setEnabled(m_pCurWPolar && m_bSequence);
	m_pctrlAlphaDelta->setEnabled(m_pCurWPolar && m_bSequence);
	m_pctrlSequence->setEnabled(m_pCurWPolar);

	m_pctrlStoreWOpp->setEnabled(m_pCurWPolar);

	s_pMainFrame->m_pShowCurWOppOnly->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pShowAllWOpps->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pHideAllWOpps->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pShowTargetCurve->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pShowXCmRefLocation->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pShowWing2Curve->setEnabled(pWing(1) && (m_iView==XFLR5::WOPPVIEW || m_iView==XFLR5::WCPVIEW));
	s_pMainFrame->m_pShowStabCurve->setEnabled( pWing(2) && (m_iView==XFLR5::WOPPVIEW || m_iView==XFLR5::WCPVIEW));
	s_pMainFrame->m_pShowFinCurve->setEnabled(  pWing(3) && (m_iView==XFLR5::WOPPVIEW || m_iView==XFLR5::WCPVIEW));
	s_pMainFrame->m_pShowAllWPlrOpps->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pHideAllWPlrOpps->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pShowPlaneWPlrsOnly->setEnabled(m_iView==XFLR5::WPOLARVIEW);
	s_pMainFrame->m_pShowPlaneWPlrs->setEnabled(m_iView==XFLR5::WPOLARVIEW);
	s_pMainFrame->m_pHidePlaneWPlrs->setEnabled(m_iView==XFLR5::WPOLARVIEW);
	s_pMainFrame->m_pShowPlaneWOpps->setEnabled(m_iView==XFLR5::WOPPVIEW);
	s_pMainFrame->m_pHidePlaneWOpps->setEnabled(m_iView==XFLR5::WOPPVIEW);

	m_pctrlLift->setEnabled( (m_iView==XFLR5::WOPPVIEW||m_iView==XFLR5::W3DVIEW) && m_pCurPOpp);
	m_pctrlTrans->setEnabled((m_iView==XFLR5::WOPPVIEW||m_iView==XFLR5::W3DVIEW) && m_pCurPOpp);
	m_pctrlWOppAnimate->setEnabled((m_iView==XFLR5::WOPPVIEW||m_iView==XFLR5::W3DVIEW) && m_pCurPOpp && m_pCurPOpp->polarType()!=XFLR5::STABILITYPOLAR);
	m_pctrlAnimateWOppSpeed->setEnabled((m_iView==XFLR5::WOPPVIEW||m_iView==XFLR5::W3DVIEW) && m_pCurPOpp && m_pctrlWOppAnimate->isChecked());
	m_pctrlIDrag->setEnabled(     m_iView==XFLR5::W3DVIEW && m_pCurPOpp);
	m_pctrlVDrag->setEnabled(     m_iView==XFLR5::W3DVIEW && m_pCurPOpp);
	m_pctrlDownwash->setEnabled(  m_iView==XFLR5::W3DVIEW && m_pCurPOpp);
	m_pctrlMoment->setEnabled(    m_iView==XFLR5::W3DVIEW && m_pCurPOpp);
	m_pctrlPanelForce->setEnabled(m_iView==XFLR5::W3DVIEW && m_pCurPOpp && m_pCurWPolar && m_pCurWPolar->analysisMethod()!=XFLR5::LLTMETHOD);
	m_pctrlCp->setEnabled(        m_iView==XFLR5::W3DVIEW && m_pCurPOpp && m_pCurWPolar && m_pCurWPolar->analysisMethod()!=XFLR5::LLTMETHOD);
	m_pctrlStream->setEnabled(    m_iView==XFLR5::W3DVIEW && m_pCurPOpp && m_pCurWPolar && m_pCurWPolar->analysisMethod()!=XFLR5::LLTMETHOD);
	m_pctrlSurfVel->setEnabled(   m_iView==XFLR5::W3DVIEW && m_pCurPOpp && m_pCurWPolar && m_pCurWPolar->analysisMethod()!=XFLR5::LLTMETHOD);

	m_pctrlFoilNames->setChecked(m_pGL3dView->m_bFoilNames);
	m_pctrlMasses->setChecked(m_pGL3dView->m_bShowMasses);

	s_pMainFrame->m_pHighlightOppAct->setChecked(QGraph::isHighLighting());

	s_pMainFrame->m_pDefineWPolar->setEnabled(m_pCurPlane);
	s_pMainFrame->m_pDefineStabPolar->setEnabled(m_pCurPlane);

	s_pMainFrame->m_pCurrentPlaneMenu->setEnabled(m_pCurPlane);
    s_pMainFrame->m_pCurrentPlaneMenu_WOppCtxMenu->setEnabled(m_pCurPlane);
    s_pMainFrame->m_pCurrentPlaneMenu_WCpCtxMenu->setEnabled(m_pCurPlane);
    s_pMainFrame->m_pCurrentPlaneMenu_WTimeCtxMenu->setEnabled(m_pCurPlane);
    s_pMainFrame->m_pCurrentPlaneMenu_WPlrCtxMenu->setEnabled(m_pCurPlane);
    s_pMainFrame->m_pCurrentPlaneMenu_W3DCtxMenu->setEnabled(m_pCurPlane);
    s_pMainFrame->m_pCurrentPlaneMenu_W3DStabCtxMenu->setEnabled(m_pCurPlane);

	s_pMainFrame->m_pCurWPlrMenu->setEnabled(m_pCurWPolar);
    s_pMainFrame->m_pCurWPlrMenu_WOppCtxMenu->setEnabled(m_pCurWPolar);
    s_pMainFrame->m_pCurWPlrMenu_WCpCtxMenu->setEnabled(m_pCurWPolar);
    s_pMainFrame->m_pCurWPlrMenu_WTimeCtxMenu->setEnabled(m_pCurWPolar);
    s_pMainFrame->m_pCurWPlrMenu_WPlrCtxMenu->setEnabled(m_pCurWPolar);
    s_pMainFrame->m_pCurWPlrMenu_W3DCtxMenu->setEnabled(m_pCurWPolar);
    s_pMainFrame->m_pCurWPlrMenu_W3DStabCtxMenu->setEnabled(m_pCurWPolar);

	s_pMainFrame->m_pCurWOppMenu->setEnabled(m_pCurPOpp);
    s_pMainFrame->m_pCurWOppMenu_WOppCtxMenu->setEnabled(m_pCurPOpp);
    s_pMainFrame->m_pCurWOppMenu_WCpCtxMenu->setEnabled(m_pCurPOpp);
    s_pMainFrame->m_pCurWOppMenu_WTimeCtxMenu->setEnabled(m_pCurPOpp);
    s_pMainFrame->m_pCurWOppMenu_W3DCtxMenu->setEnabled(m_pCurPOpp);
    s_pMainFrame->m_pCurWOppMenu_W3DStabCtxMenu->setEnabled(m_pCurPOpp);

	//	s_pMainFrame->CurBodyMenu->setVisible(m_pCurPlane!=NULL);
	s_pMainFrame->m_pEditWingAct->setEnabled(m_pCurPlane);
	s_pMainFrame->m_pEditBodyAct->setEnabled(m_pCurPlane && m_pCurPlane->body());
	s_pMainFrame->m_pEditBodyObjectAct->setEnabled(m_pCurPlane && m_pCurPlane->body());

	s_pMainFrame->checkGraphActions();

	m_pctrlSpanPos->setValue(m_CurSpanPos);
	m_pctrlCpSectionSlider->setValue((int)(m_CurSpanPos*100.0));

	s_pMainFrame->m_pW3DScalesAct->setChecked(s_pMainFrame->m_pctrl3DScalesWidget->isVisible());

	m_pctrlAxes->setChecked(m_pGL3dView->m_bAxes);
	m_pctrlOutline->setChecked(m_pGL3dView->m_bOutline);
	m_pctrlPanels->setChecked(m_pGL3dView->m_bVLMPanels);
	m_pctrlAxes->setChecked(m_pGL3dView->m_bAxes);
	m_pctrlSurfaces->setChecked(m_pGL3dView->m_bSurfaces);
	m_pctrlOutline->setChecked(m_pGL3dView->m_bOutline);

	m_pctrlCp->setChecked(m_b3DCp);
	m_pctrlPanelForce->setChecked(m_bPanelForce);
	m_pctrlDownwash->setChecked(m_bDownwash);
	m_pctrlMoment->setChecked(m_bMoments);
	m_pctrlTrans->setChecked(m_bXTop);
	m_pctrlLift->setChecked(m_bXCP);
	m_pctrlIDrag->setChecked(m_bICd);
	m_pctrlVDrag->setChecked(m_bVCd);
	m_pctrlStream->setChecked(m_bStream);
	m_pctrlClipPlanePos->setValue((int)(m_pGL3dView->m_ClipPlanePos*100.0));

	m_pctrlOutline->setEnabled(m_pCurPlane);
	m_pctrlSurfaces->setEnabled(m_pCurPlane);
	m_pctrlMasses->setEnabled(m_pCurPlane);
	m_pctrlFoilNames->setEnabled(m_pCurPlane);
	m_pctrlPanels->setEnabled(m_pCurPlane);

	setCurveParams();

	blockSignals(false);
}


/**
 * Sets the checkboxes of the x, y and z view to their default false value
 */
void QMiarex::setViewControls()
{
	m_pctrlX->setChecked(false);
	m_pctrlY->setChecked(false);
	m_pctrlZ->setChecked(false);
	m_pctrlIso->setChecked(false);
}


/**
 * Creates the curves of the Cp graph at the selected span positions
 */
void QMiarex::clearCpCurves()
{
	m_CpGraph.deleteCurves();
}




/**
 * Creates the curves of the Cp graph at the selected span positions
 */
void QMiarex::createCpCurves()
{
	int pp,i;
	bool bFound;
	double SpanPos, SpanInc;

	Curve *pCurve = NULL;
	QString str2, str3;

	if(!m_pCurPOpp || !m_pCurWPolar) return;
	if(m_pCurWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		s_pMainFrame->statusBar()->showMessage(tr("Cp Curves are only available for VLM and panel methods"));
		return;
	}

	for (i=0; i<MAXWINGS; i++)
	{
		// the first four curves are necessarily the current opPoint's main wing, second wing, elevator and fin
		// the next are those the user has chosen to keep for display --> don't reset them
		pCurve = m_CpGraph.curve(i);
		if(pCurve) pCurve->clear();
	}


	if(!m_pCurPlane || !m_pCurPOpp || !m_bShowCp) return;

	int coef = m_pCurWPolar->bThinSurfaces() ? 1 : 2;

	m_CurSpanPos = qMax(-1.0, m_CurSpanPos);
	m_CurSpanPos = qMin( 1.0, m_CurSpanPos);
	SpanPos = m_CurSpanPos*m_pCurPlane->span()/2.000001;

//	str1 = m_pCurPlane->m_Wing[0].WingName();
	str2 = QString(" a=%1").arg(m_pCurPOpp->alpha(), 5, 'f', 2);
	str3 = QString(" y/b=%1").arg(m_CurSpanPos, 5, 'f', 2);

//	if(m_bCurWOppOnly)
	{
//		p=0;
		bFound = false;
//		if(m_pCurWPolar->bThinSurfaces()) p+=m_pCurPlane->m_Wing[0].m_Surface[0]->m_NXPanels;

		SpanInc = -m_pCurPlane->planformSpan()/2.0;
		for (int p=0; p<m_pCurPlane->m_Wing[0].m_MatSize; p++)
		{
			if(m_pCurPlane->m_Wing[0].m_pWingPanel[p].m_bIsTrailing && m_pCurPlane->m_Wing[0].m_pWingPanel[p].m_Pos<=MIDSURFACE)
			{
				SpanInc += m_pCurPlane->m_Wing[0].m_pWingPanel[p].width();
				if(SpanPos<=SpanInc || qAbs(SpanPos-SpanInc)/m_pCurPlane->planformSpan()<0.001)
				{
					bFound = true;
					break;
				}
			}
		}
		for (int iw=0; iw<MAXWINGS; iw++)
		{
			if(pWing(iw) && m_bShowWingCurve[iw])
			{
				int p=0;
				bFound = false;
//				if(m_pCurWPolar->bThinSurfaces()) p+=pWingList(iw)->m_Surface.at(0)->m_NXPanels;

				SpanInc = -pWing(iw)->m_PlanformSpan/2.0;
				for (p=0; p<pWing(iw)->m_MatSize; p++)
				{
					if(pWing(iw)->m_pWingPanel[p].m_bIsTrailing && pWing(iw)->m_pWingPanel[p].m_Pos<=MIDSURFACE)
					{
						SpanInc += pWing(iw)->m_pWingPanel[p].width();
						if(SpanPos<=SpanInc || qAbs(SpanPos-SpanInc)/pWing(iw)->m_PlanformSpan<0.001)
						{
							bFound = true;
							break;
						}
					}
				}

				if(bFound)
				{
					pCurve = m_CpGraph.curve(iw);
					pCurve->setColor(m_CpLineStyle.m_Color);
					pCurve->setStyle(m_CpLineStyle.m_Style);
					pCurve->setWidth(m_CpLineStyle.m_Width);
					pCurve->setPoints(m_CpLineStyle.m_PointStyle);

					pCurve->setCurveName(pWing(iw)->m_WingName+str2+str3);

					for (pp=p; pp<p+coef*pWing(iw)->m_Surface.at(0)->m_NXPanels; pp++)
					{
						pCurve->appendPoint(m_theTask.m_Panel[pp].CollPt.x, m_pWOpp[iw]->m_dCp[pp]);
					}
				}
			}
		}
	}
	s_bResetCurves = false;
}


/**
 * Creates the curves for the graphs in the operating point view.
*/
void QMiarex::createWOppCurves()
{
	int i,k;
	for(int ig=0; ig<MAXWINGGRAPHS; ig++) m_WingGraph[ig]->deleteCurves();

	// Browse through the array of plane operating points
	// add a curve for those selected, and fill them with data
	for (k=0; k<m_poaPOpp->size(); k++)
	{
		PlaneOpp *pPOpp = (PlaneOpp*)m_poaPOpp->at(k);
		if (pPOpp->isVisible() && (!m_bCurPOppOnly || (m_pCurPOpp==pPOpp)))
		{
			for(int iw=0; iw<MAXWINGS; iw++)
			{
				if(m_bShowWingCurve[iw] && pPOpp->m_pPlaneWOpp[iw])
				{
					for(int ic=0; ic<m_WingGraph.count(); ic++)
					{
						Curve *pWingCurve = m_WingGraph[ic]->addCurve();
						pWingCurve->setPoints(pPOpp->points());
						pWingCurve->setStyle(pPOpp->style());
						pWingCurve->setColor(pPOpp->color());
						pWingCurve->setWidth(pPOpp->width());
						pWingCurve->setCurveName(POppTitle(pPOpp));
						fillWOppCurve(pPOpp->m_pPlaneWOpp[iw], m_WingGraph[ic], pWingCurve);
					}
				}
			}
		}
	}

	//if the elliptic curve is requested, and if the graph variable is local lift, then add the curve
	if(m_bShowEllipticCurve && m_pCurPOpp)
	{
		double x, y;
		double lift, maxlift = 0.0;

		int nStart;
		if(m_pCurPOpp->analysisMethod()==XFLR5::LLTMETHOD) nStart = 1;
		else                                               nStart = 0;
		if(m_bMaxCL) maxlift = m_pCurPOpp->m_pPlaneWOpp[0]->maxLift();
		else
		{
			lift=0.0;
			for (i=nStart; i<m_pCurPOpp->m_NStation; i++)
			{
				x = m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[i]/m_pCurPlane->span()*2.0;
				y = sqrt(1.0 - x*x);
				lift += y*m_pCurPOpp->m_pPlaneWOpp[0]->m_StripArea[i] ;
			}
			maxlift = m_pCurPOpp->m_CL / lift * m_pCurPlane->planformArea();
		}

		for(int ig=0; ig<MAXWINGGRAPHS; ig++)
		{
			if(m_WingGraph[ig]->yVariable()==3)
			{
				Curve *pCurve = m_WingGraph[ig]->addCurve();
				pCurve->setStyle(1);
				pCurve->setWidth(2);
				pCurve->setColor(QColor(100, 100, 100));
				for (double id=-50.0; id<=50.5; id+=1.0)
				{
					x = m_pCurPlane->span()/2.0 * cos(id*PI/50.0) * ( 1.0-PRECISION);
					y = maxlift*sqrt(1.0 - x*x/m_pCurPlane->span()/m_pCurPlane->span()*4.0);
					pCurve->appendPoint(x*Units::mtoUnit(),y);
				}
			}
		}
	}
	//if the target bell curve is requested, and if the graph variable is local lift, then add the curve
	if(m_bShowBellCurve && m_pCurPOpp)
	{
		double b2 = m_pCurPlane->span()/2.0;
		int nStart;
		if(m_pCurPOpp->analysisMethod()==XFLR5::LLTMETHOD) nStart = 1;
		else                                               nStart = 0;

		double lift, maxlift, x, y;
		if(m_bMaxCL) maxlift = m_pCurPOpp->m_pPlaneWOpp[0]->maxLift();
		else
		{
			lift=0.0;
			for (i=nStart; i<m_pCurPOpp->m_NStation; i++)
			{
				x = m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[i];
				y = pow(1.0-x*x/b2/b2, m_BellCurveExp);
				lift += y*m_pCurPOpp->m_pPlaneWOpp[0]->m_StripArea[i];
			}
			maxlift = m_pCurPOpp->m_CL / lift * m_pCurPlane->planformArea();
		}

		for(int ig=0; ig<MAXWINGGRAPHS; ig++)
		{
			if(m_WingGraph[ig]->yVariable()==3)
			{
				Curve *pCurve = m_WingGraph[ig]->addCurve();
				pCurve->setStyle(1);
				pCurve->setWidth(2);
				pCurve->setColor(QColor(100, 100, 100));
				for (double id=-50.0; id<=50.5; id+=1.0)
				{
					double phi = id*PI/2/ 50.0;
					x = sin(phi) * b2;
					y = maxlift * pow(1.0-x*x/b2/b2, m_BellCurveExp);
					pCurve->appendPoint(x*Units::mtoUnit(),y);
				}
			}
		}
	}
	s_bResetCurves = false;
}


/**
* Resets and fills the polar graphs curves with the data from the WPolar objects
*/
void QMiarex::createWPolarCurves()
{
	WPolar *pWPolar;
	Curve *pCurve[MAXPOLARGRAPHS];

	for(int ig=0; ig<m_WPlrGraph.count(); ig++) m_WPlrGraph[ig]->deleteCurves();

	for (int k=0; k<m_poaWPolar->size(); k++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(k);
		if (pWPolar->isVisible() && pWPolar->dataSize()>0 &&
			((m_bType1 && pWPolar->polarType()==XFLR5::FIXEDSPEEDPOLAR) ||
			(m_bType2 && pWPolar->polarType()==XFLR5::FIXEDLIFTPOLAR) ||
			(m_bType4 && pWPolar->polarType()==XFLR5::FIXEDAOAPOLAR) ||
			(            pWPolar->polarType()==XFLR5::BETAPOLAR) ||
			(m_bType7 && pWPolar->polarType()==XFLR5::STABILITYPOLAR)))
		{

			for(int ig=0; ig<m_WPlrGraph.count(); ig++)
			{
				pCurve[ig] = m_WPlrGraph[ig]->addCurve();
				fillWPlrCurve(pCurve[ig], pWPolar, m_WPlrGraph[ig]->xVariable(), m_WPlrGraph[ig]->yVariable());
				pCurve[ig]->setPoints(pWPolar->points());
				pCurve[ig]->setStyle(pWPolar->curveStyle());
				pCurve[ig]->setColor(pWPolar->curveColor());
				pCurve[ig]->setWidth(pWPolar->curveWidth());
				pCurve[ig]->setCurveName(pWPolar->polarName());
			}
		}
	}
	s_bResetCurves = false;
}


/**
* Resets and fills the stability graphs curves with the data from the CWPolar objects
*/
void QMiarex::createStabilityCurves()
{
	if(m_iView==XFLR5::STABTIMEVIEW)
	{
		if(m_StabilityResponseType==1)  createStabRungeKuttaCurves();
		else                            createStabTimeCurves();
	}
	else
	{
		createStabRLCurves();
	}
}


/**
* Builds the initial condition response due to perturbations from steady state
* The time response is calculated analytically based on the knowledge of the eigenvalues and eigenvectors
*/
void QMiarex::createStabTimeCurves()
{
	complex<double> M[16];// the modal matrix
	complex<double> InvM[16];// the inverse of the modal matrix
	complex<double> q[4],q0[4],y[4];//the part of each mode in the solution
	int i,j,k;
	double t, dt, TotalPoints; // the input load
	complex<double> in[4];
	Curve *pCurve0, *pCurve1, *pCurve2, *pCurve3;
	QString strong, CurveTitle;

	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
	CurveTitle = pStabView->m_pctrlCurveList->currentText();

	pCurve0 = m_TimeGraph[0]->curve(CurveTitle);
	if(pCurve0) pCurve0->clear();
	else return;
	pCurve1 = m_TimeGraph[1]->curve(CurveTitle);
	if(pCurve1) pCurve1->clear();
	else return;
	pCurve2 = m_TimeGraph[2]->curve(CurveTitle);
	if(pCurve2) pCurve2->clear();
	else return;
	pCurve3 = m_TimeGraph[3]->curve(CurveTitle);
	if(pCurve3) pCurve3->clear();
	else return;

	if(!m_pCurPOpp || !m_pCurPOpp->isVisible()) return;

	strong = pStabView->m_pctrlCurveList->currentText();

	m_Deltat = pStabView->m_pctrlDeltat->value();
	m_TotalTime = pStabView->m_pctrlTotalTime->value();
	dt = m_TotalTime/1000.;
	if(dt<m_Deltat) dt = m_Deltat;

	TotalPoints = qMin(1000, (int)(m_TotalTime/dt));
	//read the initial state condition
	m_TimeInput[0] = pStabView->m_pctrlStabVar1->value();
	m_TimeInput[1] = pStabView->m_pctrlStabVar2->value();
	m_TimeInput[2] = pStabView->m_pctrlStabVar3->value();
	m_TimeInput[3] = 0.0;//we start with an initial 0.0 value for pitch or bank angles

	if(m_StabilityResponseType==0)
	{
		//start with the user input initial conditions
		in[0] = complex<double>(m_TimeInput[0], 0.0);
		in[1] = complex<double>(m_TimeInput[1], 0.0);
		in[2] = complex<double>(m_TimeInput[2], 0.0);
		in[3] = complex<double>(m_TimeInput[3]*PI/180.0, 0.0);
	}
	else if(m_StabilityResponseType==2)
	{
		//start with the initial conditions which will excite only the requested mode
		in[0] = m_pCurPOpp->m_EigenVector[pStabView->m_iCurrentMode][0];
		in[1] = m_pCurPOpp->m_EigenVector[pStabView->m_iCurrentMode][1];
		in[2] = m_pCurPOpp->m_EigenVector[pStabView->m_iCurrentMode][2];
		in[3] = m_pCurPOpp->m_EigenVector[pStabView->m_iCurrentMode][3];
	}

	//fill the modal matrix
	if(m_bLongitudinal) k=0; else k=1;
	for (i=0; i<4; i++)
	{
		for(j=0;j<4;j++)
		{
			*(M+4*j+i) = m_pCurPOpp->m_EigenVector[k*4+i][j];
		}
	}

	//Invert the matrix
	if(!Invert44(M, InvM))
	{
	}
	else
	{
		//calculate the modal coefficients at t=0
		q0[0] = InvM[0] * in[0] + InvM[1] * in[1] + InvM[2] * in[2] + InvM[3] * in[3];
		q0[1] = InvM[4] * in[0] + InvM[5] * in[1] + InvM[6] * in[2] + InvM[7] * in[3];
		q0[2] = InvM[8] * in[0] + InvM[9] * in[1] + InvM[10]* in[2] + InvM[11]* in[3];
		q0[3] = InvM[12]* in[0] + InvM[13]* in[1] + InvM[14]* in[2] + InvM[15]* in[3];

		for(i=0; i<TotalPoints; i++)
		{
			t = (double)i * dt;
			q[0] = q0[0] * exp(m_pCurPOpp->m_EigenValue[0+k*4]*t);
			q[1] = q0[1] * exp(m_pCurPOpp->m_EigenValue[1+k*4]*t);
			q[2] = q0[2] * exp(m_pCurPOpp->m_EigenValue[2+k*4]*t);
			q[3] = q0[3] * exp(m_pCurPOpp->m_EigenValue[3+k*4]*t);
			y[0] = *(M+4*0+0) * q[0] +*(M+4*0+1) * q[1] +*(M+4*0+2) * q[2] +*(M+4*0+3) * q[3];
			y[1] = *(M+4*1+0) * q[0] +*(M+4*1+1) * q[1] +*(M+4*1+2) * q[2] +*(M+4*1+3) * q[3];
			y[2] = *(M+4*2+0) * q[0] +*(M+4*2+1) * q[1] +*(M+4*2+2) * q[2] +*(M+4*2+3) * q[3];
			y[3] = *(M+4*3+0) * q[0] +*(M+4*3+1) * q[1] +*(M+4*3+2) * q[2] +*(M+4*3+3) * q[3];
			if(fabs(q[0])>1.e10 || fabs(q[1])>1.e10 || fabs(q[2])>1.e10  || fabs(q[3])>1.e10 ) break;

			pCurve0->appendPoint(t, y[0].real());
			if(m_bLongitudinal) pCurve1->appendPoint(t, y[1].real());
			else                pCurve1->appendPoint(t, y[1].real()*180.0/PI);
			pCurve2->appendPoint(t, y[2].real()*180.0/PI);
			pCurve3->appendPoint(t, y[3].real()*180.0/PI);
		}
	}
	s_bResetCurves = false;
}



/**
* Builds the forced response from the state matrix and the forced input matrix
* using a Runge-Kutta integration scheme.
* The forced input is interpolated in the control history defined in the input table.
*/
void QMiarex::createStabRungeKuttaCurves()
{
	int i, TotalPoints, PlotInterval;

	double t, dt, ctrl_t;
	Curve *pCurve0, *pCurve1, *pCurve2, *pCurve3;
	double A[4][4], B[4];
	double m[5][4];
	double y[4], yp[4];

	QString CurveTitle;

	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
	CurveTitle = pStabView->m_pctrlCurveList->currentText();
	pCurve0 = m_TimeGraph[0]->curve(CurveTitle);
	if(pCurve0) pCurve0->clear();
	else return;
	pCurve1 = m_TimeGraph[1]->curve(CurveTitle);
	if(pCurve1) pCurve1->clear();
	else return;
	pCurve2 = m_TimeGraph[2]->curve(CurveTitle);
	if(pCurve2) pCurve2->clear();
	else return;
	pCurve3 = m_TimeGraph[3]->curve(CurveTitle);
	if(pCurve3) pCurve3->clear();
	else return;

	//We need a WOpp
	if(!m_pCurPOpp) return;//nothing to plot
	//Check that the current polar is of the stability type
	if(!m_pCurWPolar || m_pCurWPolar->polarType()!=XFLR5::STABILITYPOLAR) return;

	if(m_bLongitudinal)
	{
		memcpy(A, m_pCurPOpp->m_ALong, 4*4*sizeof(double));
		memcpy(B, m_pCurPOpp->m_BLong, 4*sizeof(double));
	}
	else
	{
		memcpy(A, m_pCurPOpp->m_ALat, 4*4*sizeof(double));
		memcpy(B, m_pCurPOpp->m_BLat, 4*sizeof(double));
	}

	// Rebuild the Forced Response matrix
	//read the initial step condition
//	pStabView->ReadForcedInput(time,input);
//	RampAmp     = m_RampAmplitude*PI/180.0;
//	RampTime    = m_RampTime;           //s

	m_Deltat    = pStabView->m_pctrlDeltat->value();
	m_TotalTime = pStabView->m_pctrlTotalTime->value();
	dt = m_TotalTime/1000.;
	if(dt<m_Deltat) dt = m_Deltat;

	TotalPoints  = qMin(1000, (int)(m_TotalTime/dt));
	PlotInterval = qMax(1,(int)(TotalPoints/200));

	//we are considering forced response from initial steady state, so set
	// initial conditions to 0
	t = 0.0;
	y[0] = y[1] = y[2] = y[3] = 0.0;
	pCurve0->appendPoint(0.0, y[0]);
	pCurve1->appendPoint(0.0, y[1]);
	pCurve2->appendPoint(0.0, y[2]);
	pCurve3->appendPoint(0.0, y[3]);

	//Runge-Kutta method
	for(i=0; i<TotalPoints; i++)
	{
		//initial slope m1
		m[0][0] = A[0][0]*y[0] + A[0][1]*y[1] + A[0][2]*y[2] + A[0][3]*y[3];
		m[0][1] = A[1][0]*y[0] + A[1][1]*y[1] + A[1][2]*y[2] + A[1][3]*y[3];
		m[0][2] = A[2][0]*y[0] + A[2][1]*y[1] + A[2][2]*y[2] + A[2][3]*y[3];
		m[0][3] = A[3][0]*y[0] + A[3][1]*y[1] + A[3][2]*y[2] + A[3][3]*y[3];

		ctrl_t = pStabView->getControlInput(t);
		m[0][0] += B[0] * ctrl_t;
		m[0][1] += B[1] * ctrl_t;
		m[0][2] += B[2] * ctrl_t;
		m[0][3] += B[3] * ctrl_t;

		//middle point m2
		yp[0] = y[0] + dt/2.0 * m[0][0];
		yp[1] = y[1] + dt/2.0 * m[0][1];
		yp[2] = y[2] + dt/2.0 * m[0][2];
		yp[3] = y[3] + dt/2.0 * m[0][3];

		m[1][0] = A[0][0]*yp[0] + A[0][1]*yp[1] + A[0][2]*yp[2] + A[0][3]*yp[3];
		m[1][1] = A[1][0]*yp[0] + A[1][1]*yp[1] + A[1][2]*yp[2] + A[1][3]*yp[3];
		m[1][2] = A[2][0]*yp[0] + A[2][1]*yp[1] + A[2][2]*yp[2] + A[2][3]*yp[3];
		m[1][3] = A[3][0]*yp[0] + A[3][1]*yp[1] + A[3][2]*yp[2] + A[3][3]*yp[3];

		ctrl_t = pStabView->getControlInput(t+dt/2.0);
		m[1][0] += B[0] * ctrl_t;
		m[1][1] += B[1] * ctrl_t;
		m[1][2] += B[2] * ctrl_t;
		m[1][3] += B[3] * ctrl_t;

		//second point m3
		yp[0] = y[0] + dt/2.0 * m[1][0];
		yp[1] = y[1] + dt/2.0 * m[1][1];
		yp[2] = y[2] + dt/2.0 * m[1][2];
		yp[3] = y[3] + dt/2.0 * m[1][3];

		m[2][0] = A[0][0]*yp[0] + A[0][1]*yp[1] + A[0][2]*yp[2] + A[0][3]*yp[3];
		m[2][1] = A[1][0]*yp[0] + A[1][1]*yp[1] + A[1][2]*yp[2] + A[1][3]*yp[3];
		m[2][2] = A[2][0]*yp[0] + A[2][1]*yp[1] + A[2][2]*yp[2] + A[2][3]*yp[3];
		m[2][3] = A[3][0]*yp[0] + A[3][1]*yp[1] + A[3][2]*yp[2] + A[3][3]*yp[3];

		ctrl_t = pStabView->getControlInput(t+dt/2.0);

		m[2][0] += B[0] * ctrl_t;
		m[2][1] += B[1] * ctrl_t;
		m[2][2] += B[2] * ctrl_t;
		m[2][3] += B[3] * ctrl_t;

		//third point m4
		yp[0] = y[0] + dt * m[2][0];
		yp[1] = y[1] + dt * m[2][1];
		yp[2] = y[2] + dt * m[2][2];
		yp[3] = y[3] + dt * m[2][3];

		m[3][0] = A[0][0]*yp[0] + A[0][1]*yp[1] + A[0][2]*yp[2] + A[0][3]*yp[3];
		m[3][1] = A[1][0]*yp[0] + A[1][1]*yp[1] + A[1][2]*yp[2] + A[1][3]*yp[3];
		m[3][2] = A[2][0]*yp[0] + A[2][1]*yp[1] + A[2][2]*yp[2] + A[2][3]*yp[3];
		m[3][3] = A[3][0]*yp[0] + A[3][1]*yp[1] + A[3][2]*yp[2] + A[3][3]*yp[3];

		ctrl_t = pStabView->getControlInput(t+dt);

		m[3][0] += B[0] * ctrl_t;
		m[3][1] += B[1] * ctrl_t;
		m[3][2] += B[2] * ctrl_t;
		m[3][3] += B[3] * ctrl_t;

		//final slope m5
		m[4][0] = 1./6. * (m[0][0] + 2.0*m[1][0] + 2.0*m[2][0] + m[3][0]);
		m[4][1] = 1./6. * (m[0][1] + 2.0*m[1][1] + 2.0*m[2][1] + m[3][1]);
		m[4][2] = 1./6. * (m[0][2] + 2.0*m[1][2] + 2.0*m[2][2] + m[3][2]);
		m[4][3] = 1./6. * (m[0][3] + 2.0*m[1][3] + 2.0*m[2][3] + m[3][3]);

		y[0] += m[4][0] * dt;
		y[1] += m[4][1] * dt;
		y[2] += m[4][2] * dt;
		y[3] += m[4][3] * dt;
		t +=dt;
		if(qAbs(y[0])>1.e10 || qAbs(y[1])>1.e10 || qAbs(y[2])>1.e10  || qAbs(y[3])>1.e10 ) break;

		if(i%PlotInterval==0)
		{
			if(m_bLongitudinal)
			{
				pCurve0->appendPoint(t, y[0]*Units::mstoUnit());
				pCurve1->appendPoint(t, y[1]*Units::mstoUnit());
				pCurve2->appendPoint(t, y[2]*180.0/PI);//deg/s
				pCurve3->appendPoint(t, y[3]*180.0/PI);//deg
			}
			else
			{
				pCurve0->appendPoint(t, y[0]*Units::mstoUnit());
				pCurve1->appendPoint(t, y[1]*180.0/PI);//deg/s
				pCurve2->appendPoint(t, y[2]*180.0/PI);//deg/s
				pCurve3->appendPoint(t, y[3]*180.0/PI);//deg
			}
		}
	}
	pCurve0->setVisible(true);
	pCurve1->setVisible(true);
	pCurve2->setVisible(true);
	pCurve3->setVisible(true);

	s_bResetCurves = false;
}


/**
* Resets and fills the curves of the root locus graph with the data from the CWPolar objects
*/
void QMiarex::createStabRLCurves()
{
	WPolar *pWPolar;

	// we have eight modes, 4 longitudinal and 4 lateral
	// declare a curve for each
	Curve *pLongCurve[4];
	Curve *pLatCurve [4];

	m_StabPlrGraph.at(0)->deleteCurves();
	m_StabPlrGraph.at(1)->deleteCurves();

	for (int k=0; k<m_poaWPolar->size(); k++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(k);
		if ((pWPolar->isVisible())
			&& pWPolar->dataSize()>0 && (m_bType7 && pWPolar->isStabilityPolar()))
		{
			for(int iCurve=0; iCurve<4; iCurve++)
			{
				pLongCurve[iCurve] = m_StabPlrGraph.at(0)->addCurve();
				pLongCurve[iCurve]->setVisible(pWPolar->isVisible());
				pLongCurve[iCurve]->setPoints(pWPolar->points());
				pLongCurve[iCurve]->setStyle(pWPolar->curveStyle());
				pLongCurve[iCurve]->setColor(pWPolar->curveColor());
				pLongCurve[iCurve]->setWidth(pWPolar->curveWidth());
				pLongCurve[iCurve]->setCurveName(pWPolar->polarName()+QString("_Mode_%1").arg(iCurve));
				fillStabCurve(pLongCurve[iCurve], pWPolar, iCurve);
			}

			//Lateral modes
			for(int iCurve=0; iCurve<4; iCurve++)
			{
				pLatCurve[iCurve] = m_StabPlrGraph.at(1)->addCurve();
				pLatCurve[iCurve]->setVisible(pWPolar->isVisible());
				pLatCurve[iCurve]->setPoints(pWPolar->points());
				pLatCurve[iCurve]->setStyle(pWPolar->curveStyle());
				pLatCurve[iCurve]->setColor(pWPolar->curveColor());
				pLatCurve[iCurve]->setWidth(pWPolar->curveWidth());
				pLatCurve[iCurve]->setCurveName(pWPolar->polarName()+QString("_Mode_%1").arg(iCurve));
				fillStabCurve(pLatCurve[iCurve], pWPolar, iCurve+4);
			}
		}
	}
	s_bResetCurves = false;
}




/**
 * Initializes the style combo box for the graph curves
 * Selects the styles of the active curve
 */
void QMiarex::fillComboBoxes(bool bEnable)
{
	m_pctrlCurveColor->setEnabled(bEnable);
	m_pctrlCurveStyle->setEnabled(bEnable);
	m_pctrlCurveWidth->setEnabled(bEnable);
	m_pctrlCurvePoints->setEnabled(bEnable);
	m_pctrlShowCurve->setEnabled(bEnable);

	int LineStyle[5];
	int LineWidth[5];
	int LinePoints[5];

	for (int i=0; i<5;i++)
	{
		LineWidth[i] = m_LineStyle.m_Width;
		LineStyle[i] = m_LineStyle.m_Style;
		LinePoints[i] = m_LineStyle.m_PointStyle;
	}

	m_pStyleDelegate->setLineWidth(LineWidth); // the same selected width for all styles
	m_pStyleDelegate->setLineColor(m_LineStyle.m_Color);
//	else        m_pStyleDelegate->setLineColor(QColor(100,100,100));
	m_pStyleDelegate->setPointStyle(LinePoints);

	m_pWidthDelegate->setLineStyle(LineStyle); //the same selected style for all widths
	m_pWidthDelegate->setLineColor(m_LineStyle.m_Color);
//	else        m_pPointDelegate->setLineColor(QColor(100,100,100));
	m_pWidthDelegate->setPointStyle(LinePoints);

	m_pPointDelegate->setLineStyle(LineStyle);
	m_pPointDelegate->setLineWidth(LineWidth);
	for (int i=0; i<5;i++) LinePoints[i]=i;
	m_pPointDelegate->setPointStyle(LinePoints);
	m_pPointDelegate->setLineColor(m_LineStyle.m_Color);
//	else        m_pPointDelegate->setLineColor(QColor(100,100,100));


	if(bEnable)
	{
		m_pctrlCurveStyle->setLine( m_LineStyle.m_Style, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
		m_pctrlCurveWidth->setLine( m_LineStyle.m_Style, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
		m_pctrlCurvePoints->setLine(m_LineStyle.m_Style, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
		m_pctrlCurveColor->setColor(m_LineStyle.m_Color);
		m_pctrlCurveColor->setStyle(m_LineStyle.m_Style);
		m_pctrlCurveColor->setWidth(m_LineStyle.m_Width);
	}
	else
	{
		m_pctrlCurveStyle->setLine( 0, 1, QColor(100,100,100), 0);
		m_pctrlCurveWidth->setLine( 0, 1, QColor(100,100,100), 0);
		m_pctrlCurvePoints->setLine(0, 1, QColor(100,100,100), 0);
		m_pctrlCurveColor->setColor(QColor(100,100,100));
		m_pctrlCurveColor->setStyle(0);
		m_pctrlCurveColor->setWidth(1);
	}


	m_pctrlCurveStyle->update();
	m_pctrlCurveWidth->update();
	m_pctrlCurvePoints->update();
	m_pctrlCurveColor->update();

	m_pctrlCurveStyle->setCurrentIndex(m_LineStyle.m_Style);
	m_pctrlCurveWidth->setCurrentIndex(m_LineStyle.m_Width-1);
	m_pctrlCurvePoints->setCurrentIndex(m_LineStyle.m_PointStyle);
}



/**
* Fills the existing active curve with the WOpp data
*@param pWOpp  a pointer to the instance of the CWOpp object from which the data is to be extracted
*@param pGraph a pointer to the instance of the Graph object to which the curve belongs
*@param pCurve a pointer to the instance of the CCurve object to be filled with the data from the CWOpp object
*/
void QMiarex::fillWOppCurve(WingOpp *pWOpp, Graph *pGraph, Curve *pCurve)
{
	if(!pWOpp || !pGraph || !pCurve) return;
	int Var = pGraph->yVariable();
	int nStart, i;

	if(pWOpp->m_AnalysisMethod==XFLR5::LLTMETHOD) nStart = 1;
	else nStart = 0;

	switch(Var)
	{
		case 0:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_Ai[i]);
			}
			pGraph->setYTitle(tr("Induced Angle"));
			break;
		}
		case 1:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(),
					pWOpp->m_Alpha + pWOpp->m_Ai[i] + pWOpp->m_Twist[i]);
			}
			pGraph->setYTitle(tr("Total Angle"));
			break;
		}
		case 2:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_Cl[i]);
			}
			pGraph->setYTitle(tr("Cl"));
			break;
		}
		case 3:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_Cl[i] * pWOpp->m_Chord[i]/pWOpp->m_MAChord);
			}
			pGraph->setYTitle(tr("Local lift"));
			break;
		}
		case 4:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_PCd[i]);
			}
			pGraph->setYTitle(tr("Airfoil drag"));
			break;
		}
		case 5:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_ICd[i]);
			}
			pGraph->setYTitle(tr("Induced drag"));
			break;
		}
		case 6:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_PCd[i]+ pWOpp->m_ICd[i]);
			}
			pGraph->setYTitle(tr("Total drag"));
			break;
		}
		case 7:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), (pWOpp->m_PCd[i]+ pWOpp->m_ICd[i])* pWOpp->m_Chord[i]/pWOpp->m_MAChord);
			}
			pGraph->setYTitle(tr("Local drag"));
			break;
		}
		case 8:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_CmAirf[i]);
			}
			pGraph->setYTitle(tr("Cm Airfoil"));
			break;
		}
		case 9:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_Cm[i]);
			}
			pGraph->setYTitle(tr("Cm total"));
			break;
		}
		case 10:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_Re[i]);
			}
			pGraph->setYTitle(tr("Re"));
			break;
		}
		case 11:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_XTrTop[i]);
			}
			pGraph->setYTitle(tr("Top Trans x-Pos %"));
			break;
		}
		case 12:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_XTrBot[i]);
			}
			pGraph->setYTitle(tr("Bot Trans x-Pos %"));
			break;
		}
		case 13:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_XCPSpanRel[i]*100.0);
			}
			pGraph->setYTitle(tr("CP x-Pos %"));
			break;
		}
		case 14:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(),
								pWOpp->m_BendingMoment[i] * Units::NmtoUnit());
			}
			QString str;
			Units::getMomentUnitLabel(str);
			pGraph->setYTitle(tr("BM (") + str + ")");
			break;
		}
		default:
		{
			for (i=nStart; i<pWOpp->m_NStation; i++)
			{
				pCurve->appendPoint(pWOpp->m_SpanPos[i]*Units::mtoUnit(), pWOpp->m_Ai[i]);
			}
			pGraph->setYTitle(tr("Induced Angle"));
		}
	}
}


/**
* Fills the curve of the stability graph with the data from the pWPolar oject for the seleted mode
*@param pCurve  a pointer to the instance of the CCurve object to be filled with the data from the CWPolar object
*@param pWPolar a pointer to the instance of the CWPolar object from which the data is to be extracted
*@param iMode the index of the mode for which the curve is to be created
*/
void QMiarex::fillStabCurve(Curve *pCurve, WPolar *pWPolar, int iMode)
{
	int i;
	double x,y;

	pCurve->setSelected(-1);

	for (i=0; i<pWPolar->dataSize(); i++)
	{
		x = pWPolar->m_EigenValue[iMode][i].real();
		y = pWPolar->m_EigenValue[iMode][i].imag()/2./PI;

		pCurve->appendPoint(x, y);
		if(m_pCurPlane && m_pCurPOpp && QGraph::isHighLighting())
		{
			if(qAbs(pWPolar->m_Ctrl[i]-m_pCurPOpp->m_Ctrl)<0.0001)
			{
				if((pWPolar->planeName()==m_pCurPlane->planeName()) && (m_pCurPOpp->polarName()==pWPolar->polarName()))
				{
					pCurve->setSelected(i);
				}
			}
		}
	}
}


/**
* Fills the polar curve object which has been created with the variable data specified by XVar and YVar.
*@param pCurve a pointer to the curve to fill with the data
*@param pWPolar a pointer to the instance of the CWPolar object from which the data will be extracted
*@param XVar the index of the variable to appear on the x-axis
*@param YVar the index of the variable to appear on the y-axis
*/
void QMiarex::fillWPlrCurve(Curve *pCurve, WPolar *pWPolar, int XVar, int YVar)
{
	bool bAdd;
	int i;
	double x,y;
	QString PlaneName;
	if(m_pCurPlane)     PlaneName=m_pCurPlane->planeName();
	QList <double> *pX;
	QList <double> *pY;
	pX = (QList <double> *) pWPolar->getWPlrVariable(XVar);
	pY = (QList <double> *) pWPolar->getWPlrVariable(YVar);

	pCurve->setSelected(-1);
	for (i=0; i<pWPolar->dataSize(); i++)
	{
		bAdd = true;

		x = (*pX)[i];
		y = (*pY)[i];

//		if((XVar==16 || XVar==17 || XVar==20) && x<0) bAdd = false;
//		if((YVar==16 || YVar==17 || YVar==20) && y<0) bAdd = false;

		//Set user units
		if(XVar==17 || XVar==18 || XVar==19)  x *= Units::NtoUnit(); //force
		if(YVar==17 || YVar==18 || YVar==19)  y *= Units::NtoUnit(); //force

		if(XVar==20 || XVar==21 || XVar==22)  x *= Units::mstoUnit();//speed
		if(YVar==20 || YVar==21 || YVar==22)  y *= Units::mstoUnit();//speed

		if(XVar==24 || XVar==25 || XVar==26)  x *= Units::NmtoUnit();//moment
		if(YVar==24 || YVar==25 || YVar==26)  y *= Units::NmtoUnit();//moment

		if(XVar==27 || XVar==28 || XVar==29 ) x *= Units::mtoUnit();//length
		if(YVar==27 || YVar==28 || YVar==29 ) y *= Units::mtoUnit();//length

		if(XVar==30)                          x *= Units::NmtoUnit();//moment
		if(YVar==30)                          y *= Units::NmtoUnit();//moment

		if(XVar==36)                          x *= Units::mtoUnit();// XNP, length
		if(YVar==36)                          y *= Units::mtoUnit();// XNP, length

		if(XVar==47)                          x *= Units::kgtoUnit(); //mass
		if(YVar==47)                          y *= Units::kgtoUnit(); //mass
		if(XVar==48 || XVar==49)              x *= Units::mtoUnit();//length
		if(YVar==48 || YVar==49)              y *= Units::mtoUnit();//length

		if(bAdd)
		{
			pCurve->appendPoint(x,y);
			if(m_pCurPOpp && QGraph::isHighLighting())
			{
				if(qAbs(pWPolar->m_Alpha[i]-m_pCurPOpp->m_pPlaneWOpp[0]->m_Alpha)<0.0001)
				{
					if(m_pCurPOpp && m_pCurPlane
					   && pWPolar->planeName()==m_pCurPlane->planeName()
					   && m_pCurPOpp->polarName() ==pWPolar->polarName())
					{
						pCurve->setSelected(i);
					}
				}
			}
		}
	}
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void QMiarex::glMake3DObjects()
{
	Body *pCurBody = NULL;
	if(!m_pCurPlane) return;

	pCurBody = m_pCurPlane->body();

	if(m_pCurWPolar) m_pGL3dView->setSpanStations(m_pCurPlane, m_pCurWPolar, m_pCurPOpp);

	if(m_bResetglBody && pCurBody)
	{
		Body translatedBody;
		translatedBody.duplicate(pCurBody);
		translatedBody.translate(m_pCurPlane->m_BodyPos);
		if(pCurBody->isSplineType())         m_pGL3dView->glMakeBodySplines(&translatedBody);
		else if(pCurBody->isFlatPanelType()) m_pGL3dView->glMakeBody3DFlatPanels(&translatedBody);
		m_bResetglBody = false;
	}

	if(m_bResetglGeom  && m_pCurPlane && m_iView==XFLR5::W3DVIEW)
	{
		Body translatedBody;
		if(m_pCurPlane->body())
		{
			translatedBody.duplicate(m_pCurPlane->body());
			translatedBody.translate(m_pCurPlane->bodyPos());
		}

		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pCurPlane->wing(iw))
			{
				if(m_pCurPlane->body())
				{
					m_pGL3dView->glMakeWingGeometry(iw, m_pCurPlane->wing(iw), &translatedBody);
				}
				else
				{
					m_pGL3dView->glMakeWingGeometry(iw, m_pCurPlane->wing(iw), NULL);
				}
			}
		}

		m_bResetglGeom = false;
	}


	if(m_bResetglMesh)
	{
		m_pGL3dView->glMakePanels(m_pGL3dView->m_vboMesh, m_theTask.m_MatSize, m_theTask.m_nNodes, m_theTask.m_Node, m_theTask.m_Panel, NULL);
		m_bResetglMesh = false;
	}

	if(m_bResetglPanelCp || m_bResetglOpp)
	{
		if(m_pCurWPolar && m_pCurWPolar->analysisMethod()!=XFLR5::LLTMETHOD)
			m_pGL3dView->glMakePanels(m_pGL3dView->m_vboPanelCp, m_theTask.m_MatSize, m_theTask.m_nNodes, m_theTask.m_Node, m_theTask.m_Panel, m_pCurPOpp);
		m_bResetglPanelCp = false;
	}


	if((m_bResetglPanelForce || m_bResetglOpp)
		&& m_iView==XFLR5::W3DVIEW
		&& m_pCurWPolar && m_pCurWPolar->analysisMethod()!=XFLR5::LLTMETHOD)
	{
		if (m_pCurPlane && m_pCurPOpp)
		{
			m_pGL3dView->glMakePanelForces(m_theTask.m_MatSize, m_theTask.m_Panel, m_pCurWPolar, m_pCurPOpp);
		}
		m_bResetglPanelForce = false;
	}


	if((m_bResetglLift || m_bResetglOpp) && m_iView==XFLR5::W3DVIEW)
	{
		if (m_pCurPOpp)
		{
			for(int iw=0; iw<MAXWINGS; iw++)
			{
				if(pWing(iw))
				{
					m_pGL3dView->glMakeLiftStrip( iw, pWing(iw), m_pCurWPolar, m_pWOpp[iw]);
					m_pGL3dView->glMakeTransitions(iw, pWing(iw), m_pCurWPolar, m_pWOpp[iw]);
				}
			}
			m_pGL3dView->glMakeLiftForce(m_pCurWPolar, m_pCurPOpp);
			m_pGL3dView->glMakeMoments(pWing(0), m_pCurWPolar, m_pCurPOpp);
		}
		m_bResetglLift = false;
	}

	if((m_bResetglDrag || m_bResetglOpp) && m_iView==XFLR5::W3DVIEW)
	{
		if (m_pCurPOpp)
		{
			for(int iw=0; iw<MAXWINGS; iw++)
			{
				if(pWing(iw))
				{
					m_pGL3dView->glMakeDragStrip( iw, pWing(iw), m_pCurWPolar, m_pWOpp[iw], m_pCurPOpp->beta());
				}
			}
		}
		m_bResetglLift = false;
	}

	if((m_bResetglDownwash || m_bResetglOpp) && m_iView==XFLR5::W3DVIEW)
	{
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(pWing(iw) && m_pWOpp[iw])
			{
				m_pGL3dView->glMakeDownwash(iw, pWing(iw), m_pCurWPolar, m_pWOpp[iw]);
			}
		}

		m_bResetglDownwash = false;
	}

	if((m_bResetglStream) && m_iView==XFLR5::W3DVIEW)
	{
		if(m_bStream)
		{
			m_bStream = false; //Disable temporarily during calculation
			//no need to recalculate if not showing
			if(m_pCurPlane && m_pCurPOpp && m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
			{
				Wing *pWingList[MAXWINGS];
				for(int iw=0; iw<MAXWINGS;iw++) pWingList[iw]=m_pCurPlane->wing(iw);
				if(!m_pGL3dView->glMakeStreamLines(pWingList, m_theTask.m_Node, m_pCurWPolar, m_pCurPOpp, m_theTask.m_MatSize))
				{
					m_bStream  = false;
					m_bResetglStream = true;
					m_pctrlStream->blockSignals(true);
					m_pctrlStream->setChecked(false);
					m_pctrlStream->blockSignals(false);
				}
				else
				{
					m_bStream  = true;
					m_bResetglStream = false;
				}
			}
		}
	}
	if((m_bResetglLegend || m_bResetglOpp || m_bResetglGeom) && m_iView==XFLR5::W3DVIEW)
	{
		if(m_pCurPOpp)
		{
			m_pGL3dView->glMakeCpLegendClr();
		}
		m_bResetglLegend = false;
	}

	if((m_bResetglSurfVelocities || m_bResetglOpp) && m_iView==XFLR5::W3DVIEW && m_bSurfVelocities)
	{
		if(m_pCurPlane && m_pCurPOpp && m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
		{
			m_pGL3dView->glMakeSurfVelocities(m_theTask.m_Panel, m_pCurWPolar, m_pCurPOpp, m_theTask.m_MatSize);
			m_bResetglSurfVelocities = false;
		}
	}

	m_bResetglOpp = false;
}





/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void QMiarex::keyPressEvent(QKeyEvent *event)
{
	bool bCtrl  = (event->modifiers() & Qt::ControlModifier) ? true : false;
	bool bShift = (event->modifiers() & Qt::ShiftModifier)   ? true : false;

	m_pGL3dView->m_bArcball=false;

	if(event->key()==Qt::Key_0 || event->text()=="0")
	{
		return;
	}
	else if(event->key()==Qt::Key_1 || event->text()=="1")
	{
		if(bCtrl)
		{
			s_pMainFrame->onAFoil();
			event->accept();
			return;
		}

	}
	else if(event->key()==Qt::Key_2 || event->text()=="2")
	{
		if(bCtrl)
		{
			s_pMainFrame->onAFoil();
			event->accept();
			return;
		}
	}
	else if(event->key()==Qt::Key_3 || event->text()=="3")
	{
		if(bCtrl)
		{
			s_pMainFrame->onXInverse();
			event->accept();
			return;
		}
	}
	else if(event->key()==Qt::Key_4 || event->text()=="4")
	{
		if(bCtrl)
		{
			s_pMainFrame->onXInverseMixed();
			event->accept();
			return;
		}

	}
	else if(event->key()==Qt::Key_5 || event->text()=="5")
	{
		if(bCtrl)
		{
			s_pMainFrame->onXDirect();
			event->accept();
			return;
		}

	}
	else if(event->key()==Qt::Key_6 || event->text()=="6")
	{
		if(bCtrl)
		{
			s_pMainFrame->onXDirect();
			event->accept();
			return;
		}
	}
	else if(event->key()==Qt::Key_7 || event->text()=="7")
	{
/*		if(bCtrl)
		{
			s_pMainFrame->loadXFLR5File(s_pMainFrame->m_RecentFiles.at(0));

			s_pMainFrame->updatePlaneListBox();
			setPlane();
			updateView();
		}*/
	}

	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if (event->modifiers().testFlag(Qt::AltModifier))
			{
				onWPolarProperties();
				break;
			}
			if(!m_pctrlAnalyze->hasFocus())
			{
				activateWindow();
				m_pctrlAnalyze->setFocus();
			}
			else
			{
				onAnalyze();
			}
			event->accept();
			break;
		}
		case Qt::Key_Escape:
		{

			stopAnimate();

			if(s_pMainFrame->m_pctrl3DScalesWidget->isVisible()) s_pMainFrame->m_pctrl3DScalesWidget->hide();
			updateView();
			break;
		}
		case Qt::Key_A:
		{
			onGL3DScale();
			break;
		}

		case Qt::Key_D:
		{
			if(MainFrame::s_bTrace)
			{
				QString FileName = QDir::tempPath() + "/Trace.log";
				QDesktopServices::openUrl(QUrl::fromLocalFile(FileName));
			}
			break;
		}
		case Qt::Key_L:
		{
			s_pMainFrame->onLogFile();
			break;
		}
		case Qt::Key_X:
			m_bXPressed = true;
			break;
		case Qt::Key_Y:
			m_bYPressed = true;
			break;
		case Qt::Key_H:
		{
			if((m_iView==XFLR5::WPOLARVIEW || m_iView==XFLR5::STABPOLARVIEW) && event->modifiers().testFlag(Qt::ControlModifier))
			{
				s_pMainFrame->onHighlightOperatingPoint();
			}
			break;
		}
		case Qt::Key_F12:
		{
			onPlaneInertia();
			break;
		}
		case Qt::Key_F2:
		{
			if(bShift) onRenameCurWPolar();
			else       onRenameCurPlane();
			break;
		}
		case Qt::Key_F3:
		{
			if (event->modifiers().testFlag(Qt::ShiftModifier))        onEditCurPlane();
			else if (event->modifiers().testFlag(Qt::ControlModifier)) onEditCurObject();
			else                                                       onNewPlane();
			break;
		}
		case Qt::Key_F4:
		{
			if(bCtrl)
			{
				s_pMainFrame->m_pCloseProjectAct->trigger();
			}
			if(MainFrame::hasOpenGL()) on3DView();
			break;
		}
		case Qt::Key_F5:
		{
			onWOppView();
			break;
		}
		case Qt::Key_F6:
		{
			if (event->modifiers().testFlag(Qt::ShiftModifier))         onDefineStabPolar();
			else if (event->modifiers().testFlag(Qt::ControlModifier))  onDefineWPolarObject();
			else                                                        onDefineWPolar();
			break;
		}
		case Qt::Key_F8:
		{
			if (event->modifiers().testFlag(Qt::ShiftModifier))        onRootLocusView();
			else if (event->modifiers().testFlag(Qt::ControlModifier)) onStabTimeView();
			else                                                       onWPolarView();
			break;
		}
		case Qt::Key_F9:
		{
			onCpView();
			break;
		}
		case Qt::Key_F10:
		{
			onEditCurWing();
			break;
		}
		case Qt::Key_F11:
		{
			onEditCurBody();
			break;
		}

		case Qt::Key_F1:
		{
/*			if(m_pCurPlane && m_pCurPlane->body())
			{
				EditBodyDlg ebDlg;
				ebDlg.initDialog(m_pCurPlane->body());
				ebDlg.exec();
			}*/
			break;
		}
		case Qt::Key_8:
		{
			if(bCtrl) s_pMainFrame->onOpenGLInfo();
			break;
		}
		case Qt::Key_9:
		{
			if(bCtrl) onExporttoSTL();
			break;
		}
		case Qt::Key_M:
		{
//			if(bCtrl) onImportSTLFile();
			break;
		}
		default:
			//			QWidget::keyPressEvent(event);
			event->ignore();
	}
}

/**
 * Dispatches the key release event
 * @param event the QKeyEvent sent by Qt
 */
void QMiarex::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_X:
			if(!event->isAutoRepeat()) m_bXPressed = false;
			break;
		case Qt::Key_Y:
			if(!event->isAutoRepeat()) m_bYPressed = false;
			break;
		default:
			event->ignore();
	}
}


/**
 * Launches the LLT analysis and updates the display after the analysis
 * @param V0 : the start angle
 * @param VMax : the maximal angle
 * @param VDelta : the increment angle
 * @param bSequence : if true, the analysis will be run for the whole range between V0 and VMax; if not, the analysis will be run for V0 only
 * @param bInitCalc : if true, the starting point for convergence iterations will be reset to the default; if not, the iterations will start at the last calculated point
 *
*/
void QMiarex::LLTAnalyze(double V0, double VMax, double VDelta, bool bSequence, bool bInitCalc)
{
	if(!m_pCurPlane || !m_pCurWPolar) return;

	LLTAnalysis::s_bInitCalc = bInitCalc;
	LLTAnalysis::s_IterLim = m_LLTMaxIterations;

	m_pLLTDlg->iterGraph()->copySettings(&Settings::s_RefGraph);

//	m_pLLTDlg->deleteTask();

//	PlaneAnalysisTask *pTask = new PlaneAnalysisTask();
//	memcpy(pTask, &m_theTask, sizeof(PlaneAnalysisTask));

	m_theTask.initializeTask(m_pCurPlane, m_pCurWPolar, V0, VMax, VDelta, bSequence);
	m_pLLTDlg->setTask(&m_theTask);
	m_pLLTDlg->initDialog();
	m_pLLTDlg->show();

	m_pLLTDlg->analyze();

	if(m_bLogFile && (m_theTask.m_ptheLLTAnalysis->m_bError || m_theTask.m_ptheLLTAnalysis->m_bWarning))
	{
	}
	else
	{
		m_pLLTDlg->hide();
	}

	setPlaneOpp(false, V0);
	s_pMainFrame->updatePOppListBox();
	emit projectModified();
}



/**
 * Loads the user's saved settings from the configuration file and maps the data.
 *@param a pointer to the QSettings object loaded in the MainFrame class
 *@return true if the settings have been loaded successfully
 */
bool QMiarex::loadSettings(QSettings *pSettings)
{
	QString strong;
	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;

	pSettings->beginGroup("Miarex");
	{
		m_bXCmRef       = pSettings->value("bXCmRef", true).toBool();
		m_bXTop         = pSettings->value("bXTop", false).toBool();
		m_bXBot         = pSettings->value("bXBot", false).toBool();
		m_bXCP          = pSettings->value("bXCP", false).toBool();
		m_bPanelForce   = pSettings->value("bPanelForce", false).toBool();
		m_bICd          = pSettings->value("bICd", true).toBool();
		m_bVCd          = pSettings->value("bVCd", true).toBool();
		m_pGL3dView->m_bSurfaces     = pSettings->value("bSurfaces").toBool();
		m_pGL3dView->m_bOutline      = pSettings->value("bOutline").toBool();
		m_pGL3dView->m_bVLMPanels    = pSettings->value("bVLMPanels").toBool();
		m_pGL3dView->m_bAxes         = pSettings->value("bAxes").toBool();
		m_b3DCp         = pSettings->value("b3DCp").toBool();
		m_bDownwash     = pSettings->value("bDownwash").toBool();
		m_bMoments      = pSettings->value("bMoments").toBool();
		s_bAutoCpScale  = pSettings->value("bAutoCpScale").toBool();
		m_bShowCpScale  = pSettings->value("bShowCpScale").toBool();
		m_bCurPOppOnly  = pSettings->value("CurWOppOnly").toBool();
		m_bShowEllipticCurve = pSettings->value("bShowElliptic").toBool();
		m_bShowBellCurve     = pSettings->value("bShowTargetCurve").toBool();
		m_BellCurveExp  = pSettings->value("BellCurveExp", 1).toDouble();
		m_bMaxCL        = pSettings->value("CurveMaxCL", true ).toBool();
		m_bLogFile      = pSettings->value("LogFile").toBool();
		m_bDirichlet    = pSettings->value("Dirichlet").toBool();
		m_bResetWake    = pSettings->value("ResetWake").toBool();
		m_bShowWingCurve[0]    = pSettings->value("ShowWing").toBool();
		m_bShowWingCurve[1]    = pSettings->value("ShowWing2").toBool();
		m_bShowWingCurve[2]    = pSettings->value("ShowStab").toBool();
		m_bShowWingCurve[3]    = pSettings->value("ShowFin").toBool();
		m_bShowWingCurve[0] = true;
		m_bShowFlapMoments = pSettings->value("showFlapMoments", true).toBool();

		PlaneOpp::s_bStoreOpps    = pSettings->value("StoreWOpp").toBool();
		m_bSequence     = pSettings->value("Sequence").toBool();

		m_AlphaMin      = pSettings->value("AlphaMin").toDouble();
		m_AlphaMax      = pSettings->value("AlphaMax").toDouble();
		m_AlphaDelta    = pSettings->value("AlphaDelta").toDouble();
		m_BetaMin       = pSettings->value("BetaMin", 0.0).toDouble();
		m_BetaMax       = pSettings->value("BetaMax", 1.0).toDouble();
		m_BetaDelta     = pSettings->value("BetaDelta", 0.5).toDouble();
		m_QInfMin       = pSettings->value("QInfMin").toDouble();
		m_QInfMax       = pSettings->value("QInfMax").toDouble();
		m_QInfDelta     = pSettings->value("QInfDelta").toDouble();
		m_ControlMin    = pSettings->value("ControlMin").toDouble();
		m_ControlMax    = pSettings->value("ControlMax").toDouble();
		m_ControlDelta  = pSettings->value("ControlDelta").toDouble();

		m_CpLineStyle.m_Style = pSettings->value("CpStyle").toInt();
		m_CpLineStyle.m_Width = pSettings->value("CpWidth").toInt();
		m_CpLineStyle.m_Color = pSettings->value("CpColor").value<QColor>();
		m_CpLineStyle.m_PointStyle = pSettings->value("CpPointStyle").toInt();

		int k = pSettings->value("iView").toInt();
		if(k==0)      m_iView = XFLR5::WOPPVIEW;
		else if(k==1) m_iView = XFLR5::WPOLARVIEW;
		else if(k==2) m_iView = XFLR5::W3DVIEW;
		else if(k==3) m_iView = XFLR5::WCPVIEW;
		else if(k==4) m_iView = XFLR5::STABTIMEVIEW;
		else if(k==5) m_iView = XFLR5::STABPOLARVIEW;

		k = pSettings->value("iWingView").toInt();
		if(k==0)      m_iWingView  = XFLR5::ALLGRAPHS;
		else if(k==1) m_iWingView  = XFLR5::ONEGRAPH;
		else if(k==2) m_iWingView  = XFLR5::TWOGRAPHS;
		else if(k==4) m_iWingView  = XFLR5::FOURGRAPHS;

		k = pSettings->value("iWPlrView").toInt();
		if(k==0)      m_iWPlrView  = XFLR5::ALLGRAPHS;
		else if(k==1) m_iWPlrView  = XFLR5::ONEGRAPH;
		else if(k==2) m_iWPlrView  = XFLR5::TWOGRAPHS;
		else if(k==4) m_iWPlrView  = XFLR5::FOURGRAPHS;

		k = pSettings->value("iRootLocusView").toInt();
		if(k==0)      m_iRootLocusView  = XFLR5::ALLGRAPHS;
		else if(k==1) m_iRootLocusView  = XFLR5::ONEGRAPH;
		else if(k==2) m_iRootLocusView  = XFLR5::TWOGRAPHS;
		else if(k==4) m_iRootLocusView  = XFLR5::FOURGRAPHS;

		k = pSettings->value("iStabTimeView").toInt();
		if(k==0)      m_iStabTimeView  = XFLR5::ALLGRAPHS;
		else if(k==1) m_iStabTimeView  = XFLR5::ONEGRAPH;
		else if(k==2) m_iStabTimeView  = XFLR5::TWOGRAPHS;
		else if(k==4) m_iStabTimeView  = XFLR5::FOURGRAPHS;

		m_LLTMaxIterations         = pSettings->value("Iter").toInt();
//		GL3dBodyDlg::s_NHoopPoints  = pSettings->value("NHoopPoints").toInt();
//		GL3dBodyDlg::s_NXPoints     = pSettings->value("NXPoints").toInt();
		m_InducedDragPoint  = pSettings->value("InducedDragPoint").toInt();

		s_LiftScale     = pSettings->value("LiftScale").toDouble();
		s_DragScale     = pSettings->value("DragScale").toDouble();
		s_VelocityScale = pSettings->value("VelocityScale").toDouble();

		m_WakeInterNodes    = pSettings->value("WakeInterNodes").toInt();

		m_RampTime      = pSettings->value("RampTime", 0.1).toDouble();
		m_RampAmplitude = pSettings->value("RampAmplitude", 1.0).toDouble();

		m_TotalTime         = pSettings->value("TotalTime",10.0).toDouble();
		m_Deltat            = pSettings->value("Delta_t",0.01).toDouble();

		m_TimeInput[0]      = pSettings->value("TimeIn0",0.0).toDouble();
		m_TimeInput[1]      = pSettings->value("TimeIn1",0.0).toDouble();
		m_TimeInput[2]      = pSettings->value("TimeIn2",0.0).toDouble();
		m_TimeInput[3]      = pSettings->value("TimeIn3",0.0).toDouble();
		m_bLongitudinal     = pSettings->value("DynamicsMode").toBool();
		m_StabilityResponseType = pSettings->value("StabCurveType",0).toInt();


		for(int i=0; i<20; i++)
		{
			strong = QString("ForcedTime%1").arg(i);
			pStabView->m_Time[i] = pSettings->value(strong, (double)i).toDouble();
		}
		for(int i=0; i<20; i++)
		{
			strong = QString("ForcedAmplitude%1").arg(i);
			pStabView->m_Amplitude[i] = pSettings->value(strong, 0.0).toDouble();
		}
		pStabView->updateControlModelData();

		StabPolarDlg::s_StabWPolar.m_bAutoInertia = pSettings->value("StabPolarAutoInertia", true).toBool();
		StabPolarDlg::s_StabWPolar.mass()   = pSettings->value("StabPolarMass", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.CoG().x  = pSettings->value("StabPolarCoGx", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.CoG().y  = pSettings->value("StabPolarCoGy", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.CoG().z  = pSettings->value("StabPolarCoGz", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.m_CoGIxx = pSettings->value("StabPolarCoGIxx", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.m_CoGIyy = pSettings->value("StabPolarCoGIyy", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.m_CoGIzz = pSettings->value("StabPolarCoGIzz", 0.0).toDouble();
		StabPolarDlg::s_StabWPolar.m_CoGIxz = pSettings->value("StabPolarCoGIxz", 0.0).toDouble();


		WPolarDlg::s_WPolar.m_bAutoInertia =	pSettings->value("bAutoInertia", false).toBool();
		WPolarDlg::s_WPolar.bVLM1() = pSettings->value("bVLM1").toBool();

		PlaneOpp::s_bKeepOutOpps  = pSettings->value("KeepOutOpps").toBool();

		W3dPrefsDlg::s_MassColor = pSettings->value("MassColor", QColor(100, 100, 200)).value<QColor>();

		LLTAnalysis::s_CvPrec       = pSettings->value("CvPrec").toDouble();
		LLTAnalysis::s_RelaxMax     = pSettings->value("RelaxMax").toDouble();
		LLTAnalysis::s_NLLTStations = pSettings->value("NLLTStations").toInt();

		PanelAnalysis::s_bTrefftz   = pSettings->value("Trefftz").toBool();

		Panel::s_CtrlPos       = pSettings->value("CtrlPos").toDouble();
		Panel::s_VortexPos     = pSettings->value("VortexPos").toDouble();
		Panel::s_CoreSize      = pSettings->value("CoreSize", 0.000001).toDouble();
		Wing::s_MinPanelSize   = pSettings->value("MinPanelSize").toDouble();

		AeroDataDlg::s_Temperature = pSettings->value("Temperature", 288.15).toDouble();
		AeroDataDlg::s_Altitude    = pSettings->value("Altitude", 0.0).toDouble();
	}

	pSettings->endGroup();

	GL3dBodyDlg::loadSettings(pSettings);

	GLLightDlg::loadSettings(pSettings);

	EditPlaneDlg::loadSettings(pSettings);
	EditBodyDlg::loadSettings(pSettings);

	m_CpGraph.loadSettings(pSettings);

	for(int ig=0; ig<MAXWINGGRAPHS; ig++)
	{
		m_WingGraph[ig]->loadSettings(pSettings);
	}
	for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
	{
		m_WPlrGraph[ig]->loadSettings(pSettings);
		setWGraphTitles(m_WPlrGraph[ig]);
	}
	for(int ig=0; ig<MAXTIMEGRAPHS; ig++)
	{
		m_TimeGraph[ig]->loadSettings(pSettings);
	}
	m_StabPlrGraph.at(0)->loadSettings(pSettings);
	m_StabPlrGraph.at(1)->loadSettings(pSettings);

	setStabGraphTitles();

	return true;
}




/**
 * Updates the display after the user has requested a switch to the OpenGL 3D view
 */
void QMiarex::on3DView()
{
	if(!MainFrame::hasOpenGL())
	{
		m_iView = XFLR5::WPOLARVIEW;
		updateView();
		return;
	}

	m_bResetTextLegend = true;

	if(m_iView==XFLR5::W3DVIEW)
	{
		setControls();
		updateView();
		if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
		{
			s_pMainFrame->m_pctrlStabViewWidget->show();
		}
		return;
	}

	m_iView = XFLR5::W3DVIEW;
	setControls();

	s_pMainFrame->setMainFrameCentralWidget();

	updateView();
	return;
}



/**
 * Updates the display after the user has toggled the switch for the display of Cp coefficients
 */
void QMiarex::on3DCp()
{
	m_b3DCp = m_pctrlCp->isChecked();
	m_bResetTextLegend = true;

	if(m_b3DCp)
	{
		m_pGL3dView->m_bSurfaces = false;
		m_pctrlSurfaces->setChecked(false);
		m_bPanelForce = false;
		m_pctrlPanelForce->setChecked(false);
	}
	updateView();
}


/**
 * Updates the display after the user has requested a reset of the scales in the 3D view
*/
void QMiarex::on3DResetScale()
{
	m_bPickCenter   = false;
	m_pGL3dView->on3DReset();
}


/**
 * The user has requested a modification of the styles for the 3D view
 * Launhes the dialog box, reads the data, and updates the view
*/
void QMiarex::on3DPrefs()
{
	W3dPrefsDlg w3dDlg(s_pMainFrame);
	w3dDlg.initDialog();

	w3dDlg.exec();
	m_bResetglWake = true;
	m_bResetglBody = true;
	m_bResetglGeom = true;
	m_bResetglMesh = true;
	m_bResetglOpp  = true;
	m_bResetglStream = true;

	updateView();
}




/**
 * The user has requested a launch of the analysis
 * Reads a last time the input parameters from the control box
 * Checks the foils
 * Launches the analysis
 * Updates the active view
*/
void QMiarex::onAnalyze()
{
	int l;
	double V0, VMax, VDelta;
	bool bHigh = QGraph::isHighLighting();
	QGraph::setOppHighlighting(false);

	if(!m_pCurPlane)
	{
		QMessageBox::warning(s_pMainFrame, tr("Warning"), tr("Please define a plane object before running a calculation"));
		return;
	}
	if(!m_pCurWPolar)
	{
		QMessageBox::warning(s_pMainFrame, tr("Warning"), tr("Please define an analysis/polar before running a calculation"));
		return;
	}

	//prevent an automatic and lengthy redraw of the streamlines after the calculation
	m_bStream = m_bSurfVelocities = false;
	m_pctrlStream->setChecked(false);
	m_pctrlSurfVel->setChecked(false);

	// make sure that the latest parameters are loaded
	onReadAnalysisData();

	if(m_pCurWPolar->polarType()==XFLR5::FIXEDAOAPOLAR)
	{
		V0     = m_QInfMin;
		VMax   = m_QInfMax;
		VDelta = m_QInfDelta;
	}
	else if(m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
	{
		V0     = m_ControlMin;
		VMax   = m_ControlMax;
		VDelta = m_ControlDelta;
	}
	else if(m_pCurWPolar->polarType()==XFLR5::BETAPOLAR)
	{
		V0     = m_BetaMin;
		VMax   = m_BetaMax;
		VDelta = m_BetaDelta;
	}
	else if(m_pCurWPolar->polarType() <XFLR5::FIXEDAOAPOLAR)
	{
		V0     = m_AlphaMin;
		VMax   = m_AlphaMax;
		VDelta = m_AlphaDelta;
	}
	else
	{
		V0 = VMax = VDelta = 0.0;
	}

	// check if all the foils are in the database...
	// ...could have been deleted or renamed or not imported with AVL wing or whatever
	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pWing(iw))
		{
			for (l=0; l<pWing(iw)->NWingSection(); l++)
			{
				if (!Objects2D::foil(pWing(iw)->rightFoil(l)))
				{
					QString strong;
					strong = pWing(iw)->m_WingName + ": "+tr("Could not find the wing's foil ")+ pWing(iw)->rightFoil(l) +tr("...\nAborting Calculation");
					QMessageBox::warning(s_pMainFrame, tr("Warning"), strong);
					return;
				}
				if (!Objects2D::foil(pWing(iw)->leftFoil(l)))
				{
					QString strong;
					strong = pWing(iw)->m_WingName + ": "+tr("Could not find the wing's foil ")+ pWing(iw)->leftFoil(l) +tr("...\nAborting Calculation");
					QMessageBox::warning(s_pMainFrame, tr("Warning"), strong);
					return;
				}
			}
		}
	}

	m_pctrlAnalyze->setEnabled(false);
	s_pMainFrame->m_pctrlPlane->setEnabled(false);
	s_pMainFrame->m_pctrlPlanePolar->setEnabled(false);
	s_pMainFrame->m_pctrlPlaneOpp->setEnabled(false);

	if(m_pCurWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		LLTAnalyze(V0, VMax, VDelta, m_bSequence, m_bInitLLTCalc);
	}
	else if(m_theTask.m_MatSize>0 && m_theTask.m_Panel && m_theTask.m_Node)
	{
		panelAnalyze(V0, VMax, VDelta, m_bSequence);
	}

	m_pctrlAnalyze->setEnabled(true);
	s_pMainFrame->m_pctrlPlane->setEnabled(true);
	s_pMainFrame->m_pctrlPlanePolar->setEnabled(true);
	s_pMainFrame->m_pctrlPlaneOpp->setEnabled(true);

	//restore things as they were
	QGraph::setOppHighlighting(bHigh);

	//refresh the view
	s_bResetCurves = true;
	updateView();
	setControls();
	s_pMainFrame->setFocus();
}




/**
 * Launches the animation of the WOpp display
 * Will display all the available WOpps for this WPolar in sequence
*/
void QMiarex::onAnimateWOpp()
{
	m_pctrlAnimateWOppSpeed->setEnabled(m_pctrlWOppAnimate->isChecked());
	if(!m_pCurPlane || !m_pCurWPolar || m_iView==XFLR5::WPOLARVIEW)
	{
		m_bAnimateWOpp = false;
		return;
	}
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	PlaneOpp*pPOpp;
	int l;

	if(m_pctrlWOppAnimate->isChecked())
	{
		if(m_pCurPlane)
		{
			for (l=0; l< m_poaPOpp->size(); l++)
			{
				pPOpp = (PlaneOpp*)m_poaPOpp->at(l);

				if (pPOpp &&
					pPOpp->polarName() == m_pCurWPolar->polarName() &&
					pPOpp->planeName() == m_pCurPlane->planeName())
				{
						if(m_pCurPOpp->alpha() - pPOpp->alpha()<0.0001)
							m_posAnimateWOpp = l;
				}
			}
		}

		m_bAnimateWOpp  = true;
		int speed = m_pctrlAnimateWOppSpeed->value();
		m_pTimerWOpp->setInterval(800-speed);
		m_pTimerWOpp->start();
	}
	else
	{
		stopAnimate();
	}
	QApplication::restoreOverrideCursor();
}


/**
 * A signal has been received from the timer to update the 3D mode display
 * So calculates the state corresponding to the time m_ModeTime and displays it
 *@param if true, the time position of the modal response will be incremented after the display
*/
void QMiarex::onAnimateModeSingle(bool bStep)
{
	double t, sigma, s2, omega, o2, theta_sum, psi_sum, norm;
	double *vabs, *phi;
	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;

	if(m_iView!=XFLR5::W3DVIEW)
	{
		m_pTimerMode->stop();
		return; //nothing to animate
	}
	if(!m_pCurPlane || !m_pCurWPolar || m_pCurWPolar->polarType()!=XFLR5::STABILITYPOLAR || !m_pCurPOpp)
	{
		m_pTimerMode->stop();
		return; //nothing to animate
	}
	//read the data, since the user may have been playing with it
	norm = m_ModeNorm * pStabView->m_ModeAmplitude;
	vabs = pStabView->m_vabs;
	phi  = pStabView->m_phi;

	// calculate the new state
	sigma = m_pCurPOpp->m_EigenValue[pStabView->m_iCurrentMode].real();
	omega = m_pCurPOpp->m_EigenValue[pStabView->m_iCurrentMode].imag();
	s2 = sigma*sigma;
	o2 = omega*omega;
	t=m_ModeTime;

	if(t>=100) stopAnimate();

	if(s2+o2>PRECISION)
	{
		if(m_bLongitudinal)
		{
			//x, z, theta are evaluated by direct integration of u, w, q
			m_ModeState[1] = 0.0;
			m_ModeState[3] = 0.0;
			m_ModeState[5] = 0.0;
			m_ModeState[0] = norm*vabs[0]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[0])+omega*sin(omega*t+phi[0]));
			m_ModeState[2] = norm*vabs[1]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[1])+omega*sin(omega*t+phi[1]));
			m_ModeState[4] = norm*vabs[2]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[2])+omega*sin(omega*t+phi[2]));
	//		m_ModeState[4] = norm*vabs[3]*exp(sigma*t)*cos(omega*t+phi[3]);

			//add u0 x theta_sum to z component
			theta_sum      = norm*vabs[3]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[3])+omega*sin(omega*t+phi[3]));
			m_ModeState[2] -= theta_sum *m_pCurPOpp->m_pPlaneWOpp[0]->m_QInf;
		}
		else
		{
			//y, phi, psi evaluation
			m_ModeState[0] = 0.0;
			m_ModeState[2] = 0.0;
			m_ModeState[4] = 0.0;

			// integrate (v+u0.psi.cos(theta0)) to get y
			m_ModeState[1] = norm*vabs[0]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[0])+omega*sin(omega*t+phi[0]));

			//integrate psi = integrate twice r (thanks Matlab !)
			psi_sum =   sigma * ( sigma * cos(omega*t+phi[2]) + omega * sin(omega*t+phi[2]))
					  + omega * (-omega * cos(omega*t+phi[2]) + sigma * sin(omega*t+phi[2]));
			psi_sum *= vabs[2] * exp(sigma*t)/(s2+o2)/(s2+o2);

			m_ModeState[1] += norm * m_pCurPOpp->m_pPlaneWOpp[0]->m_QInf * psi_sum;

			// get directly phi from fourth eigenvector component (alternatively integrate p+r.tan(theta0));
			m_ModeState[3] = norm*vabs[3]*exp(sigma*t)*cos(omega*t+phi[3]);
	//		m_ModeState[3] = norm*vabs[1]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[1])+omega*sin(omega*t+phi[1]));

			// integrate once 'p+r.sin(theta0)' to get heading angle
			m_ModeState[5] = norm*vabs[2]*exp(sigma*t)/(s2+o2) * (sigma*cos(omega*t+phi[2])+omega*sin(omega*t+phi[2]));
		}
	}
	else
	{
		//something went wrong somewhere
		m_ModeState[0] = 0.0;
		m_ModeState[1] = 0.0;
		m_ModeState[2] = 0.0;
		m_ModeState[3] = 0.0;
		m_ModeState[4] = 0.0;
		m_ModeState[5] = 0.0;
	}

	//increase the time for the next update
	if(bStep) m_ModeTime += m_Modedt;

	updateView();
}


/**
* A signal has been received from the timer to update the WOPP display
* So displays the next WOpp in the sequence.
*/
void QMiarex::onAnimateWOppSingle()
{
	bool bIsValid, bSkipOne;
	int size=0;
	PlaneOpp *pPOpp;

	//KickIdle
	if(m_iView!=XFLR5::W3DVIEW && m_iView !=XFLR5::WOPPVIEW) return; //nothing to animate
	if(!m_pCurPlane || !m_pCurWPolar) return;

	if(m_pCurPlane)	size = m_poaPOpp->size();
	if(size<=1) return;

	bIsValid = false;
	bSkipOne = false;

	while(!bIsValid)
	{
		pPOpp = NULL;
		//Find the current position to display

		if(m_pCurPlane)
		{
			pPOpp = (PlaneOpp*)m_poaPOpp->at(m_posAnimateWOpp);
			if(!pPOpp) return;
		}
		if(m_pCurPlane)
			bIsValid =(pPOpp->polarName()==m_pCurWPolar->polarName()  &&  pPOpp->planeName()==m_pCurPlane->planeName());

		if (bIsValid && !bSkipOne)
		{
			if(m_pCurPlane)
			{
				m_pCurPOpp = pPOpp;
				for(int iw=0; iw<MAXWINGS;iw++)
				{
					if(m_pCurPOpp->m_pPlaneWOpp[iw]) m_pWOpp[iw] = m_pCurPOpp->m_pPlaneWOpp[iw];
					else                             m_pWOpp[iw] = NULL;
				}

			}
			m_pCurPOpp = pPOpp;
			m_bCurPOppOnly = true;

			if (m_iView==XFLR5::WOPPVIEW)
			{
				m_bResetTextLegend = true;
				s_bResetCurves = true;
				updateView();
			}
			else if (m_iView==XFLR5::W3DVIEW)
			{
				m_bResetTextLegend = true;
				m_bResetglOpp      = true;
				m_bResetglDownwash = true;
				m_bResetglLift     = true;
				m_bResetglDrag     = true;
				m_bResetglWake     = true;
				m_bResetglLegend   = true;
				m_bResetglStream   = true;

				updateView();
			}

			//select the PlanePOpp in the top listbox
//			s_pMainFrame->SelectWOpp(m_pCurPOpp->m_pPlaneWOpp[0]);
		}
		else if(bIsValid) bSkipOne = false;

		if(m_bAnimateWOppPlus)
		{
			m_posAnimateWOpp++;
			if (m_posAnimateWOpp >= size)
			{
				m_posAnimateWOpp = size-1;
				m_bAnimateWOppPlus = false;
				bSkipOne = true;
			}
		}
		else
		{
			m_posAnimateWOpp--;
			if (m_posAnimateWOpp <0)
			{
				m_posAnimateWOpp = 0;
				m_bAnimateWOppPlus = true;
				bSkipOne = true;
			}
		}

		if(m_posAnimateWOpp<0 || m_posAnimateWOpp>=size) return;
	}
}


/**
* Modfies the animation after the user has changed the animation speed for the WOpp display
*/
void QMiarex::onAnimateWOppSpeed(int val)
{
	if(m_pTimerWOpp->isActive())
	{
		m_pTimerWOpp->setInterval(800-val);
	}
}


/**
* In the Opperating point view, adjusts the graph's scale to the wing's span
*/
void QMiarex::onAdjustToWing()
{
	if(!m_pCurPlane) return;

	double halfspan = m_pCurPlane->planformSpan()/2.0;
	double xmin = -halfspan*Units::mtoUnit();
	for(int ig=0; ig<MAXWINGGRAPHS; ig++)
	{
		m_WingGraph[ig]->setAutoX(false);
		m_WingGraph[ig]->setXMax( halfspan*Units::mtoUnit());
		m_WingGraph[ig]->setXMin(xmin);
	}
}


/**
 * The user has requested an edition of the advanced settings
 * Launches the dialog box and maps the returned data
*/
void QMiarex::onAdvancedSettings()
{
	WAdvancedDlg waDlg(s_pMainFrame);

	waDlg.m_MinPanelSize    = Wing::s_MinPanelSize;
	waDlg.m_AlphaPrec       = LLTAnalysis::s_CvPrec;
	waDlg.m_Relax           = LLTAnalysis::s_RelaxMax;
	waDlg.m_NLLTStation     = LLTAnalysis::s_NLLTStations;

	waDlg.m_bTrefftz        = PanelAnalysis::s_bTrefftz;

	waDlg.m_CoreSize        = Panel::s_CoreSize;
	waDlg.m_ControlPos      = Panel::s_CtrlPos;
	waDlg.m_VortexPos       = Panel::s_VortexPos;


	waDlg.m_Iter            = m_LLTMaxIterations;
	waDlg.m_bDirichlet      = m_bDirichlet;
	waDlg.m_bKeepOutOpps    = PlaneOpp::s_bKeepOutOpps;
	waDlg.m_bLogFile        = m_bLogFile;
	waDlg.m_WakeInterNodes  = m_WakeInterNodes;

	waDlg.initDialog();
	if(waDlg.exec() == QDialog::Accepted)
	{
		Wing::s_MinPanelSize         = waDlg.m_MinPanelSize;
		LLTAnalysis::s_CvPrec        = waDlg.m_AlphaPrec;
		LLTAnalysis::s_RelaxMax      = waDlg.m_Relax;
		LLTAnalysis::s_NLLTStations  = waDlg.m_NLLTStation;

		PanelAnalysis::s_bTrefftz  = waDlg.m_bTrefftz;

		Panel::s_CoreSize          = waDlg.m_CoreSize;
		Panel::s_CtrlPos           = waDlg.m_ControlPos;
		Panel::s_VortexPos         = waDlg.m_VortexPos;

		PlaneOpp::s_bKeepOutOpps         = waDlg.m_bKeepOutOpps;

		m_LLTMaxIterations     = waDlg.m_Iter;
		m_bDirichlet           = waDlg.m_bDirichlet;
		m_WakeInterNodes       = waDlg.m_WakeInterNodes;
		m_InducedDragPoint     = waDlg.m_InducedDragPoint;


		if(waDlg.m_bLogFile) m_bLogFile = true; else m_bLogFile = false;
		m_bResetglWake    = true;
		updateView();
	}
}



/**
* The user has modified the position span section to display in the Cp view
*/
void QMiarex::onCpSectionSlider(int pos)
{
	m_CurSpanPos = (double)pos/100.0;
	m_pctrlSpanPos->setValue(m_CurSpanPos);
	createCpCurves();
	updateView();
}


/**
* The user has modified the position span section to display in the Cp view
*/
void QMiarex::onCpPosition()
{
	m_CurSpanPos = m_pctrlSpanPos->value();
	m_pctrlCpSectionSlider->setValue((int)(m_CurSpanPos*100.0));
	createCpCurves();
	updateView();
}


/**
* The user has switched to the Cp view
*/
void QMiarex::onCpView()
{
	if (m_bAnimateWOpp) stopAnimate();

	if(m_iView==XFLR5::WCPVIEW)
	{
		setControls();
		updateView();
		return;
	}
	m_iView=XFLR5::WCPVIEW;

	setGraphTiles();
	s_pMainFrame->setMainFrameCentralWidget();

	createCpCurves();
	setCurveParams();
	setControls();
	updateView();
}



/**
* The user has requested a display only of the current operating point
*/
void QMiarex::onCurWOppOnly()
{
	m_bCurPOppOnly = !m_bCurPOppOnly;
	s_bResetCurves = true;
	updateView();
	setControls();
}


/**
* The user has requested a change to the color of the active curve.
* The curve may be for a polar curve or for an oppoint.
* Changes the style and modifies the content of the comboboxes
*/
void QMiarex::onCurveColor()
{
	QColor Color = QColorDialog::getColor(m_LineStyle.m_Color);
	if(Color.isValid()) m_LineStyle.m_Color = Color;
	fillComboBoxes();

	updateCurve();
}


/**
* The user has requested a change to the style of the active curve.
* The curve may be for a polar curve or for an oppoint.
* Changes the style and modifies the content of the comboboxes
*/
void QMiarex::onCurveStyle(int index)
{
	m_LineStyle.m_Style = index;
	fillComboBoxes();
	updateCurve();
}



/**
* The user has requested a change to the width of the active curve.
* The curve may be for a polar curve or for an oppoint.
* Changes the style and modifies the content of the comboboxes
*/
void QMiarex::onCurveWidth(int index)
{
	m_LineStyle.m_Width = index+1;
	fillComboBoxes();
	updateCurve();
}


/**
* The user has requested a change to the point style of the active curve.
* The curve may be for a polar curve or for an oppoint.
* Changes the style and modifies the content of the comboboxes
*/
void QMiarex::onCurvePoints(int index)
{
	m_LineStyle.m_PointStyle = index;
	fillComboBoxes();
	updateCurve();
}


/**
* The user has requested the creation of a new stability polar
*/
void QMiarex::onDefineStabPolar()
{
	if(!m_pCurPlane) return;
	stopAnimate();

	StabPolarDlg::s_StabWPolar.viscosity()     = WPolarDlg::s_WPolar.viscosity();
	StabPolarDlg::s_StabWPolar.density()       = WPolarDlg::s_WPolar.density();
	StabPolarDlg::s_StabWPolar.referenceDim()  = WPolarDlg::s_WPolar.referenceDim();
	StabPolarDlg::s_StabWPolar.bThinSurfaces() = WPolarDlg::s_WPolar.bThinSurfaces();


	StabPolarDlg spDlg(s_pMainFrame);
	spDlg.initDialog(m_pCurPlane);
	int res = spDlg.exec();

	if(res == QDialog::Accepted)
	{
		emit projectModified();

		WPolar* pNewStabPolar      = new WPolar;
		pNewStabPolar->planeName() = m_pCurPlane->planeName();
		pNewStabPolar->curveColor() = MainFrame::getColor(4);
		pNewStabPolar->curveWidth() = 2;
		pNewStabPolar->points() = 1;
		pNewStabPolar->isVisible()  = true;

		pNewStabPolar->referenceChordLength()  = m_pCurPlane->mac();

		pNewStabPolar->duplicateSpec(&StabPolarDlg::s_StabWPolar);

		if(pNewStabPolar->polarName().length()>60)
		{
			pNewStabPolar->polarName() = pNewStabPolar->polarName().left(60)+"..."+QString("(%1)").arg(m_poaWPolar->size());
		}

		pNewStabPolar->bVLM1()           = false;


		if(m_bDirichlet) pNewStabPolar->boundaryCondition() = XFLR5::DIRICHLET;
		else             pNewStabPolar->boundaryCondition() = XFLR5::NEUMANN;

		pNewStabPolar->bTilted()         = false;
		pNewStabPolar->bWakeRollUp()     = false;
		pNewStabPolar->analysisMethod()  = XFLR5::PANELMETHOD;
		pNewStabPolar->bGround()         = false;
		pNewStabPolar->m_AlphaSpec       = 0.0;
		pNewStabPolar->m_Height          = 0.0;

		m_pCurWPolar = Objects3D::insertNewWPolar(pNewStabPolar, m_pCurPlane);
		m_pCurPOpp = NULL;

		m_bResetglGeom = true;
		m_bResetglOpp  = true;
		m_bResetglMesh = true;
		m_bResetglWake = true;

		setWPolar();
		s_pMainFrame->updateWPolarListBox();
		updateView();
	}

	setControls();
}


/**
 * The user has requested the creation of a new performance polar.
 * A new WPolar object is created and is attached to the owning plane or wing.
 */
void QMiarex::onDefineWPolar()
{
	if(!m_pCurPlane) return;

	stopAnimate();

	WPolar* pNewWPolar  = new WPolar;

	WPolarDlg wpDlg(s_pMainFrame);
	wpDlg.initDialog(m_pCurPlane);

	int res = wpDlg.exec();

	if (res == QDialog::Accepted)
	{
		//Then add WPolar to array
		emit projectModified();
		pNewWPolar->duplicateSpec(&WPolarDlg::s_WPolar);
		pNewWPolar->planeName() = m_pCurPlane->planeName();
		pNewWPolar->polarName() = wpDlg.s_WPolar.polarName();

		if(pNewWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)
		{
			pNewWPolar->referenceSpanLength() = m_pCurPlane->planformSpan();
			pNewWPolar->referenceArea() = m_pCurPlane->planformArea();
			if(m_pCurPlane && m_pCurPlane->BiPlane()) pNewWPolar->referenceArea() += m_pCurPlane->wing2()->m_PlanformArea;
		}
		else if(pNewWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM)
		{
			pNewWPolar->referenceSpanLength() = m_pCurPlane->projectedSpan();
			pNewWPolar->referenceArea() = m_pCurPlane->projectedArea();
			if(m_pCurPlane && m_pCurPlane->BiPlane()) pNewWPolar->referenceArea() += m_pCurPlane->wing2()->m_ProjectedArea;
		}

		if(m_bDirichlet) pNewWPolar->boundaryCondition() = XFLR5::DIRICHLET;
		else             pNewWPolar->boundaryCondition() = XFLR5::NEUMANN;


		pNewWPolar->curveColor() = MainFrame::getColor(4);
		pNewWPolar->isVisible() = true;

		m_pCurWPolar = Objects3D::insertNewWPolar(pNewWPolar, m_pCurPlane);
		m_pCurPOpp = NULL;

		m_bResetglGeom = true;
		m_bResetglMesh = true;
		m_bResetglOpp  = true;
		m_bResetglWake = true;

		setWPolar();


		s_pMainFrame->updateWPolarListBox();
		updateView();
		m_pctrlAnalyze->setFocus();
	}
	else
	{
		delete pNewWPolar;
	}
	setControls();
}




/**
 * The user has requested the creation of a new performance polar.
 * A new WPolar object is created and is attached to the owning plane or wing.
 */
void QMiarex::onDefineWPolarObject()
{
	if(!m_pCurPlane) return;

	stopAnimate();

	WPolar* pNewWPolar  = new WPolar;
	pNewWPolar->duplicateSpec(&WPolarDlg::s_WPolar);
	pNewWPolar->planeName() = m_pCurPlane->planeName();
//	pNewWPolar->polarName() = WPolarDlg::s_WPolar.polarName();
	pNewWPolar->referenceArea()        = m_pCurPlane->planformArea();
	pNewWPolar->referenceSpanLength()  = m_pCurPlane->planformSpan();
	pNewWPolar->referenceChordLength() = m_pCurPlane->mac();
	pNewWPolar->curveColor() = MainFrame::getColor(4);

	EditPolarDefDlg vpDlg(s_pMainFrame);
	vpDlg.initDialog(m_pCurPlane, pNewWPolar);

	if (vpDlg.exec() == QDialog::Accepted)
	{
		//Then add WPolar to array
		emit projectModified();

		if(pNewWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)
		{
			pNewWPolar->referenceSpanLength() = m_pCurPlane->planformSpan();
			pNewWPolar->referenceArea()       = m_pCurPlane->planformArea();
			if(m_pCurPlane && m_pCurPlane->BiPlane()) pNewWPolar->referenceArea() += m_pCurPlane->wing2()->m_PlanformArea;
		}
		else if(pNewWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM)
		{
			pNewWPolar->referenceSpanLength() = m_pCurPlane->projectedSpan();
			pNewWPolar->referenceArea()       = m_pCurPlane->projectedArea();
			if(m_pCurPlane && m_pCurPlane->BiPlane()) pNewWPolar->referenceArea() += m_pCurPlane->wing2()->m_ProjectedArea;
		}

//		if(m_bDirichlet) pNewWPolar->boundaryCondition() = XFLR5::DIRICHLET;
//		else             pNewWPolar->boundaryCondition() = XFLR5::NEUMANN;
		pNewWPolar->isVisible() = true;

		m_pCurWPolar = Objects3D::insertNewWPolar(pNewWPolar, m_pCurPlane);
		m_pCurPOpp = NULL;

		m_bResetglGeom = true;
		m_bResetglMesh = true;
		m_bResetglOpp  = true;
		m_bResetglWake = true;

		setWPolar();

		s_pMainFrame->updateWPolarListBox();
		updateView();
		m_pctrlAnalyze->setFocus();
	}
	else
	{
		delete pNewWPolar;
	}
	setControls();
}


/**
 * The user wants to edit the analysis parameters of the currently selected polar.
 * A new WPolar object is created. The user may choose to overwrite or not the existing WPolar.
 */
void QMiarex::onEditCurWPolar()
{
	stopAnimate();

	if(!m_pCurPlane || !m_pCurWPolar) return;
	QString WPolarName;
	int res;

	WPolar *pNewWPolar = new WPolar;

	if(m_pCurWPolar->polarType()!=XFLR5::STABILITYPOLAR)
	{
		WPolarDlg dlg(s_pMainFrame);
		dlg.initDialog(m_pCurPlane, m_pCurWPolar);
		res = dlg.exec();
		pNewWPolar->duplicateSpec(&dlg.s_WPolar);
		WPolarName=dlg.s_WPolar.polarName();
	}
	else
	{
		StabPolarDlg dlg(s_pMainFrame);
		dlg.initDialog(m_pCurPlane, m_pCurWPolar);
		res = dlg.exec();
		pNewWPolar->duplicateSpec(&dlg.s_StabWPolar);
		WPolarName=dlg.s_StabWPolar.polarName();
	}

	if (res == QDialog::Accepted)
	{
		emit projectModified();

		pNewWPolar->planeName() = m_pCurPlane->planeName();
		pNewWPolar->polarName() = WPolarName;

//		pNewWPolar->bDirichlet() = m_bDirichlet;

		pNewWPolar->curveColor() = MainFrame::getColor(4);
		pNewWPolar->isVisible() = true;

		m_pCurWPolar = Objects3D::insertNewWPolar(pNewWPolar, m_pCurPlane);
		m_pCurPOpp = NULL;

		m_bResetglGeom = true;
		m_bResetglMesh = true;
		m_bResetglOpp  = true;
		m_bResetglWake = true;

		setWPolar();
		s_pMainFrame->updateWPolarListBox();
		updateView();
		m_pctrlAnalyze->setFocus();
	}
	else
	{
		delete pNewWPolar;
	}
	setControls();
}



/**
 * The user has requested an edition of the current WPolar Object
 */
void QMiarex::onEditCurWPolarObject()
{
	if(!m_pCurPlane || !m_pCurWPolar)	return;

	WPolar *pNewWPolar = new WPolar;
	pNewWPolar->duplicateSpec(m_pCurWPolar);

	EditPolarDefDlg vpDlg(s_pMainFrame);
	vpDlg.initDialog(m_pCurPlane, pNewWPolar);

	if (vpDlg.exec() == QDialog::Accepted)
	{
		emit projectModified();

		pNewWPolar->planeName() = m_pCurPlane->planeName();

//		pNewWPolar->bDirichlet() = m_bDirichlet;

		pNewWPolar->curveColor() = MainFrame::getColor(4);
		pNewWPolar->isVisible() = true;


		m_pCurWPolar = Objects3D::insertNewWPolar(pNewWPolar, m_pCurPlane);
		m_pCurPOpp = NULL;

		m_bResetglGeom = true;
		m_bResetglMesh = true;
		m_bResetglOpp  = true;
		m_bResetglWake = true;

		setWPolar();
		s_pMainFrame->updateWPolarListBox();
		updateView();
		m_pctrlAnalyze->setFocus();
	}
	setControls();
}



/**
 * The user wants to remove some result points of the currently selected polar.
 */
void QMiarex::onEditCurWPolarPts()
{
	stopAnimate();

	if(!m_pCurPlane || !m_pCurWPolar) return;

	WPolar *pMemWPolar = new WPolar;
	//Edit the current WPolar data
	if (!m_pCurWPolar) return;

	pMemWPolar->copy(m_pCurWPolar);

	EditPlrDlg epDlg(s_pMainFrame);
	epDlg.move(EditPlrDlg::s_Position);
	epDlg.resize(EditPlrDlg::s_WindowSize);
	if(EditPlrDlg::s_bWindowMaximized) epDlg.setWindowState(Qt::WindowMaximized);

	epDlg.initDialog(NULL, NULL, this, m_pCurWPolar);


	bool bPoints = m_pCurWPolar->points();
	m_pCurWPolar->points() = true;

	s_bResetCurves = true;
	updateView();

	if(epDlg.exec() == QDialog::Accepted)
	{
	   emit projectModified();
	}
	else
	{
		m_pCurWPolar->copy(pMemWPolar);
	}
	m_pCurWPolar->points() = bPoints;

	m_bResetTextLegend = true;
	s_bResetCurves = true;
	updateView();
	setControls();
	delete pMemWPolar;
}



/**
 * The user has requested a deletion of all the WOpps or POpps associated to the active WPolar.
 */
void QMiarex::onDeleteAllWPlrOpps()
{
	if(!m_pCurWPolar) return;

	emit projectModified();
	PlaneOpp* pPOpp;
	int i;
	if(m_pCurPlane)
	{
		for (i = m_poaPOpp->size()-1; i>=0; i--)
		{
			pPOpp = (PlaneOpp*) m_poaPOpp->at(i);
			if(pPOpp->polarName() == m_pCurWPolar->polarName() &&
			   pPOpp->planeName() == m_pCurPlane->planeName())
			{
				m_poaPOpp->removeAt(i);
				delete pPOpp;
			}
		}
	}

	m_pCurPOpp = NULL;
	m_bResetglMesh = true;
	s_pMainFrame->updatePOppListBox();
	setPlaneOpp(true);
	setControls();
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested a deletion of all the WOpps or POpps
 */
void QMiarex::onDeleteAllWOpps()
{
	emit projectModified();
	PlaneOpp* pPOpp;
	int i;
	for (i = m_poaPOpp->size()-1; i>=0; i--)
	{
		pPOpp = (PlaneOpp*) m_poaPOpp->at(i);
		m_poaPOpp->removeAt(i);
		delete pPOpp;
	}

	m_pCurPOpp = NULL;
	s_pMainFrame->updatePOppListBox();

	setPlaneOpp(true);

	setControls();

	s_bResetCurves = true;
	updateView();
}


/**
* The user has requested a deletion of the current wing of plane
*/
void QMiarex::onDeleteCurPlane()
{
	if(!m_pCurPlane) return;
	m_bAnimateWOpp = false;

	QString strong;
	if(m_pCurPlane) strong = tr("Are you sure you want to delete the plane :\n") +  m_pCurPlane->planeName() +"?\n";
	if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

	Objects3D::deletePlaneResults(m_pCurPlane, true);
	Objects3D::deletePlane(m_pCurPlane);
	m_pCurPlane = NULL;
	m_pCurWPolar = NULL;
	m_pCurPOpp = NULL;

	setPlane();
	s_pMainFrame->updatePlaneListBox();
	setControls();
	s_bResetCurves = true;
	emit projectModified();
	updateView();
}


/**
 * The user has requested a deletion of the current operating point
 */
void QMiarex::onDeleteCurWOpp()
{
	int i;

	double alpha=-9999999;

	if(m_pCurPOpp)
	{
		alpha = m_pCurPOpp->alpha();
		PlaneOpp* pPOpp;
		for (i = m_poaPOpp->size()-1; i>=0; i--)
		{
			pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
			if(pPOpp == m_pCurPOpp)
			{
				m_poaPOpp->removeAt(i);
				delete pPOpp;
				break;
			}
		}
		m_pCurPOpp = NULL;
		s_pMainFrame->updatePOppListBox();
		setPlaneOpp(true);
		emit projectModified();

		s_bResetCurves = true;
		updateView();
	}

	if(m_pCurWPolar)
	{
		m_pCurWPolar->remove(alpha);
	}

	setControls();
}




/**
* The user has requested a deletion of all operating point associated to the wing or plane
*/
void QMiarex::onDeletePlanePOpps()
{
	PlaneOpp *pPOpp;
	int i;
	if(m_pCurPlane)
	{
		for (i=m_poaPOpp->size()-1; i>=0; i--)
		{
			pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
			if (pPOpp->planeName() == m_pCurPlane->planeName())
			{
				m_poaPOpp->removeAt(i);
				delete pPOpp;
			}
		}
	}
	m_pCurPOpp = NULL;

	emit projectModified();
	m_bResetTextLegend = true;
	s_pMainFrame->updatePOppListBox();
	setControls();
	s_bResetCurves = true;
	updateView();
}



/**
* The user has requested a deletion of all WPolars associated to the wing or plane
*/
void QMiarex::onDeletePlaneWPolars()
{
	if(!m_pCurPlane) return;

	QString PlaneName, strong;

	PlaneName = m_pCurPlane->planeName();

	strong = tr("Are you sure you want to delete the polars associated to :\n") +  PlaneName +"?\n";
	if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

	Objects3D::deletePlaneResults(m_pCurPlane, true);

	m_pCurWPolar = NULL;
	setWPolar();
	s_pMainFrame->updateWPolarListBox();
	emit projectModified();
	setControls();
	updateView();
}


/**
 * The user has requested a deletion of the current WPolar object
 */
void QMiarex::onDeleteCurWPolar()
{
	if(!m_pCurWPolar) return;
	m_bAnimateWOpp = false;

	QString strong = tr("Are you sure you want to delete the polar :\n") +  m_pCurWPolar->polarName() +"?\n";
	if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong,
												  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

	Objects3D::deleteWPolar(m_pCurWPolar);

	m_pCurPOpp = NULL;
	m_pCurWPolar = NULL;
	emit projectModified();
	setWPolar();

	s_pMainFrame->updateWPolarListBox();
	setControls();
	updateView();
}


/**
 * The user has toggled the checkbox for the display of the downwash
 */
void QMiarex::onDownwash()
{
	m_bDownwash = m_pctrlDownwash->isChecked();
	updateView();
}


/**
 * The user has requested a duplication of the currently selected wing or plane
 */
void QMiarex::onDuplicateCurPlane()
{
	if(!m_pCurPlane) return;
	Plane *pPlane = Objects3D::duplicatePlane(m_pCurPlane);
	if(pPlane)
	{
		m_pCurPlane = pPlane;
		s_pMainFrame->updatePlaneListBox();
		setPlane(m_pCurPlane->planeName());
		emit projectModified();
	}
}



/**
 * The user has requested an edition of the current body
 * Launch the edition interface, and on return, insert the body i.a.w. user instructions
 */
void QMiarex::onEditCurBody()
{
	m_pGL3dView->m_bArcball = false;
	if(!m_pCurPlane || !m_pCurPlane->body()) return;

	Body *pCurBody = m_pCurPlane->body();

	bool bUsed = false;
	int i;
	Plane *pPlane;
	WPolar *pWPolar;
	for (i=0; i< m_poaPlane->size(); i++)
	{
		pPlane = (Plane*)m_poaPlane->at(i);
		if(pPlane->body() && pPlane->body()==pCurBody)
		{
			// Does this plane have results
			for(int j=0; j<m_poaWPolar->size(); j++)
			{
				pWPolar = (WPolar*)m_poaWPolar->at(j);
				if(pWPolar->planeName()==pPlane->planeName() && pWPolar->dataSize())
				{
					bUsed = true;
					break;
				}
			}
			if(bUsed) break;
		}
	}

	Plane *pModPlane = new Plane();
	pModPlane->duplicate(m_pCurPlane);

	GL3dBodyDlg glbDlg(s_pMainFrame);
	glbDlg.m_bEnableName = false;
	glbDlg.initDialog(pModPlane->body());
	glbDlg.move(GL3dBodyDlg::s_WindowPos);
	glbDlg.resize(GL3dBodyDlg::s_WindowSize);
	if(GL3dBodyDlg::s_bWindowMaximized) glbDlg.setWindowState(Qt::WindowMaximized);

	if(glbDlg.exec()!=QDialog::Accepted)
	{
		delete pModPlane;
		return;
	}
	if(glbDlg.m_bChanged) emit projectModified();

	m_bResetTextLegend = true;
	m_bResetglGeom = true;
	m_bResetglMesh = true;
	s_bResetCurves = true;

	ModDlg mdDlg(s_pMainFrame);

	if(bUsed && glbDlg.m_bChanged)
	{
		mdDlg.m_Question = tr("The modification will erase all results associated to this Plane.\nContinue ?");
		mdDlg.initDialog();
		int Ans = mdDlg.exec();

		if (Ans == QDialog::Rejected)
		{
			//restore geometry
			delete pModPlane; // clean up
			return;
		}
		else if(Ans==20)
		{
			//save mods to a new plane object
			m_pCurPlane = Objects3D::setModPlane(pModPlane);

			setPlane(pModPlane->planeName());
			s_pMainFrame->updatePlaneListBox();
			updateView();
			return;
		}
	}

	//then modifications are automatically recorded
	m_pCurPlane->duplicate(pModPlane);

	// in all cases copy new color and texture flag
	if(m_pCurPlane->body())
	{
		m_pCurPlane->body()->bodyColor() = pModPlane->body()->bodyColor();
		m_pCurPlane->body()->textures()  = pModPlane->body()->textures();
	}

	delete pModPlane; // clean up, we don't need it any more

	//plane has been modified, old results are not consistent with new geometry, delete them
	Objects3D::deletePlaneResults(m_pCurPlane, false);
	m_pCurWPolar = NULL;
	m_pCurPOpp = NULL;



	setPlane();

	updateView();
}


/**
 * The user has requested an edition of the current body
 * Launch the edition interface, and on return, insert the body i.a.w. user instructions
 */
void QMiarex::onEditCurBodyObject()
{
	m_pGL3dView->m_bArcball = false;
	if(!m_pCurPlane || !m_pCurPlane->body()) return;

	Body *pCurBody = m_pCurPlane->body();

	bool bUsed = false;
	int i;
	Plane *pPlane;
	WPolar *pWPolar;
	for (i=0; i< m_poaPlane->size(); i++)
	{
		pPlane = (Plane*)m_poaPlane->at(i);
		if(pPlane->body() && pPlane->body()==pCurBody)
		{
			// Does this plane have results
			for(int j=0; j<m_poaWPolar->size(); j++)
			{
				pWPolar = (WPolar*)m_poaWPolar->at(j);
				if(pWPolar->planeName()==pPlane->planeName() && pWPolar->dataSize())
				{
					bUsed = true;
					break;
				}
			}
			if(bUsed) break;
		}
	}

	Plane *pModPlane = new Plane();
	pModPlane->duplicate(m_pCurPlane);

	EditBodyDlg ebDlg(s_pMainFrame);
	ebDlg.initDialog(pModPlane->body());
	ebDlg.move(GL3dBodyDlg::s_WindowPos);
	ebDlg.resize(GL3dBodyDlg::s_WindowSize);
	if(GL3dBodyDlg::s_bWindowMaximized) ebDlg.setWindowState(Qt::WindowMaximized);

	if(ebDlg.exec()!=QDialog::Accepted)
	{
		delete pModPlane;
		return;
	}

	emit projectModified();
	m_bResetTextLegend = true;
	m_bResetglGeom = true;
	m_bResetglMesh = true;
	s_bResetCurves = true;

	ModDlg mdDlg(s_pMainFrame);

	if(bUsed)
	{
		mdDlg.m_Question = tr("The modification will erase all results associated to this Plane.\nContinue ?");
		mdDlg.initDialog();
		int Ans = mdDlg.exec();

		if (Ans == QDialog::Rejected)
		{
			//restore geometry
			delete pModPlane; // clean up
			return;
		}
		else if(Ans==20)
		{
			//save mods to a new plane object
			m_pCurPlane = Objects3D::setModPlane(pModPlane);

			setPlane(pModPlane->planeName());
			s_pMainFrame->updatePlaneListBox();
			updateView();
			return;
		}
	}

	//then modifications are automatically recorded
	m_pCurPlane->duplicate(pModPlane);
	delete pModPlane;
	Objects3D::deletePlaneResults(m_pCurPlane, false);// will also set new surface and Aerochord in WPolars
	m_pCurWPolar = NULL;
	m_pCurPOpp = NULL;
	setPlane();

	updateView();
}



/**
 * The user has requested an edition of the current Plane
 */
void QMiarex::onEditCurObject()
{
	m_pGL3dView->m_bArcball = false;
	if(!m_pCurPlane) return;
	int i;

	WPolar *pWPolar;
	PlaneOpp* pPOpp;
	bool bHasResults = false;
	for (i=0; i< m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if(pWPolar->dataSize() && pWPolar->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	for (i=0; i<m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if(pPOpp->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	Plane* pModPlane= new Plane;
	pModPlane->duplicate(m_pCurPlane);

	EditPlaneDlg voDlg((MainFrame*)s_pMainFrame);
	voDlg.initDialog(pModPlane);

	ModDlg mdDlg(s_pMainFrame);

	if(QDialog::Accepted == voDlg.exec())
	{
		emit projectModified();
		m_bResetTextLegend = true;
		m_bResetglGeom = true;
		m_bResetglMesh = true;
		s_bResetCurves = true;

		if(voDlg.m_bChanged)
		{
			if(bHasResults)
			{
				mdDlg.m_Question = tr("The modification will erase all results associated to this Plane.\nContinue ?");
				mdDlg.initDialog();
				int Ans = mdDlg.exec();

				if (Ans == QDialog::Rejected)
				{
					//restore geometry
					delete pModPlane; // clean up
					return;
				}
				else if(Ans==20)
				{
					//save mods to a new plane object
					m_pCurPlane = Objects3D::setModPlane(pModPlane);

					setPlane(pModPlane->planeName());
					s_pMainFrame->updatePlaneListBox();
					updateView();
					return;
				}
			}

			//then modifications are automatically recorded
			m_pCurPlane->duplicate(pModPlane);

			Objects3D::deletePlaneResults(m_pCurPlane, false);// will also set new surface and Aerochord in WPolars
			m_pCurWPolar = NULL;
			m_pCurPOpp = NULL;
		}

		setPlane(m_pCurPlane->planeName());
		s_pMainFrame->updatePlaneListBox();
		m_bIs2DScaleSet = false;
		setScale();
		onAdjustToWing();
		setControls();

		updateView();
	}

	delete pModPlane; // clean up
}



/**
 * The user has requested an edition of the current Plane
 * Launches the dialog box, and maps the data depending on whether the user wants to overwrite, create a new object, or has cancelled the request.
 */
void QMiarex::onEditCurPlane()
{
	m_pGL3dView->m_bArcball = false;
	if(!m_pCurPlane) return;

	int i;

	WPolar *pWPolar;
	PlaneOpp* pPOpp;
	bool bHasResults = false;
	for (i=0; i< m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if(pWPolar->dataSize() && pWPolar->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	for (i=0; i<m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if(pPOpp->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	Plane* pModPlane= new Plane;

	pModPlane->duplicate(m_pCurPlane);

	PlaneDlg plDlg(s_pMainFrame);
	plDlg.m_pPlane = pModPlane;
	plDlg.m_bAcceptName = false;
	plDlg.initDialog();

	ModDlg mdDlg(s_pMainFrame);

	if(QDialog::Accepted == plDlg.exec())
	{
		m_bResetTextLegend = true;
		m_bResetglGeom = true;
		m_bResetglMesh = true;
		s_bResetCurves = true;
		if(plDlg.m_bDescriptionChanged)
		{
			emit projectModified();
			m_pCurPlane->rPlaneDescription() = pModPlane->planeDescription();
		}

		if(plDlg.m_bChanged)
		{
			if(bHasResults)
			{
				mdDlg.m_Question = tr("The modification will erase all results associated to this Plane.\nContinue ?");
				mdDlg.initDialog();
				int Ans = mdDlg.exec();

				if (Ans == QDialog::Rejected)
				{
					//restore geometry
					delete pModPlane; // clean up
					return;
				}
				else if(Ans==20)
				{
					//save mods to a new plane object
					m_pCurPlane = Objects3D::setModPlane(pModPlane);

					setPlane(pModPlane->planeName());
					s_pMainFrame->updatePlaneListBox();
					updateView();
					return;
				}
			}

			//then modifications are automatically recorded
			m_pCurPlane->duplicate(pModPlane);
			Objects3D::deletePlaneResults(m_pCurPlane, false);// will also set new surface and Aerochord in WPolars
			m_pCurWPolar = NULL;
			m_pCurPOpp = NULL;
		}

		setPlane(m_pCurPlane->planeName());
		s_pMainFrame->updatePlaneListBox();
		m_bIs2DScaleSet = false;
		setScale();
		onAdjustToWing();
		setControls();

		updateView();
	}

	delete pModPlane; // clean up

}



/**
 * @brief The user has requested an edition of one of the wings.
 * Launches the dialog box, and maps the data depending on whether the user wants to overwrite, create a new object, or has cancelled the request.
 */
void QMiarex::onEditCurWing()
{
	m_pGL3dView->m_bArcball = false;
	if(!m_pCurPlane) return;

	int iWing;

	QAction *pAction = qobject_cast<QAction *>(sender());
	if (!pAction) iWing = 0;
	else          iWing = pAction->data().toInt();

	if(!m_pCurPlane->wing(iWing)) return;

	WPolar *pWPolar;
	PlaneOpp* pPOpp;
	bool bHasResults = false;
	for (int i=0; i< m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if(pWPolar->dataSize() && pWPolar->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	for (int i=0; i<m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if(pPOpp->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	Plane* pModPlane= new Plane;

	pModPlane->duplicate(m_pCurPlane);

	GL3dWingDlg plDlg(s_pMainFrame);
	plDlg.m_bAcceptName = false;
	plDlg.initDialog(pModPlane->wing(iWing));


	if(QDialog::Accepted == plDlg.exec())
	{
		m_bResetTextLegend = true;
		m_bResetglGeom = true;
		m_bResetglMesh = true;
		s_bResetCurves = true;

		if(plDlg.m_bDescriptionChanged)
		{
			emit projectModified();
			m_pCurPlane->wing(iWing)->setWingColor(pModPlane->wing(iWing)->wingColor());
			m_pCurPlane->wing(iWing)->m_WingDescription = pModPlane->wing(iWing)->WingDescription();
			m_pCurPlane->wing(iWing)->textures() = pModPlane->wing(iWing)->textures();
		}

		if(plDlg.m_bChanged)
		{

			if(bHasResults)
			{
				ModDlg mdDlg(s_pMainFrame);
				mdDlg.m_Question = tr("The modification will erase all results associated to this Plane.\nContinue ?");
				mdDlg.initDialog();
				int Ans = mdDlg.exec();

				if (Ans == QDialog::Rejected)
				{
					//restore geometry
					delete pModPlane; // clean up
					return;
				}
				else if(Ans==20)
				{
					//save mods to a new plane object
					m_pCurPlane = Objects3D::setModPlane(pModPlane);

					setPlane(pModPlane->planeName());
					s_pMainFrame->updatePlaneListBox();
					updateView();
					return;
				}
			}

			//then modifications are automatically recorded
			m_pCurPlane->duplicate(pModPlane);

			Objects3D::deletePlaneResults(m_pCurPlane, false);// will also set new surface and Aerochord in WPolars
			m_pCurWPolar = NULL;
			m_pCurPOpp = NULL;

		}

		setPlane(m_pCurPlane->planeName());
		s_pMainFrame->updatePlaneListBox();
		m_bIs2DScaleSet = false;
		setScale();
		onAdjustToWing();
		setControls();

		updateView();
	}

	delete pModPlane; // Clean up
}



/**
 * The user has requested that the size of the active wing be scaled.
 * Launches the dialog box, creates a new wing, and overwrites the existing wing or plane,
 * or creates a new one i.a.w. user instructions.
 */
void QMiarex::onScaleWing()
{
	if(!m_pCurPlane) return;

	WPolar *pWPolar;
	PlaneOpp* pPOpp;
	bool bHasResults = false;
	for (int i=0; i< m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if(pWPolar->dataSize() && pWPolar->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	for (int i=0; i<m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if(pPOpp->planeName() == m_pCurPlane->planeName())
		{
			bHasResults = true;
			break;
		}
	}

	Plane* pModPlane= new Plane;
	pModPlane->duplicate(m_pCurPlane);

	WingScaleDlg wsDlg(s_pMainFrame);
	wsDlg.initDialog(pModPlane->planformSpan(),
					 pModPlane->rootChord(),
					 pModPlane->wing()->averageSweep(),
					 pModPlane->wing()->tipTwist(),
					 pModPlane->planformArea(),
					 pModPlane->aspectRatio(),
					 pModPlane->taperRatio());

	if(QDialog::Accepted == wsDlg.exec())
	{
		if (wsDlg.m_bSpan || wsDlg.m_bChord || wsDlg.m_bSweep || wsDlg.m_bTwist || wsDlg.m_bArea || wsDlg.m_bAR || wsDlg.m_bTR)
		{
			if(wsDlg.m_bSpan)  pModPlane->wing()->scaleSpan(wsDlg.m_NewSpan);
			if(wsDlg.m_bChord) pModPlane->wing()->scaleChord(wsDlg.m_NewChord);
			if(wsDlg.m_bSweep) pModPlane->wing()->scaleSweep(wsDlg.m_NewSweep);
			if(wsDlg.m_bTwist) pModPlane->wing()->scaleTwist(wsDlg.m_NewTwist);
			if(wsDlg.m_bArea)  pModPlane->wing()->scaleArea(wsDlg.m_NewArea);
			if(wsDlg.m_bAR)    pModPlane->wing()->scaleAR(wsDlg.m_NewAR);
			if(wsDlg.m_bTR)    pModPlane->wing()->scaleTR(wsDlg.m_NewTR);
			pModPlane->computePlane();

			if(bHasResults)
			{
				ModDlg mdDlg(s_pMainFrame);
				mdDlg.m_Question = tr("The modification will erase all results associated to this Plane.\nContinue ?");
				mdDlg.initDialog();
				int Ans = mdDlg.exec();

				if (Ans == QDialog::Rejected)
				{
					//restore geometry
					delete pModPlane; // clean up
					return;
				}
				else if(Ans==20)
				{
					//save mods to a new plane object
					m_pCurPlane = Objects3D::setModPlane(pModPlane);

					setPlane();
					s_pMainFrame->updatePlaneListBox();
					updateView();
					return;
				}
			}

			//then modifications are automatically recorded
			m_pCurPlane->duplicate(pModPlane);
			Objects3D::deletePlaneResults(m_pCurPlane, false);// will also set new surface and Aerochord in WPolars
			m_pCurWPolar = NULL;
			m_pCurPOpp = NULL;

			m_bResetglGeom = true;
			m_bResetglMesh = true;
			s_bResetCurves = true;
			emit projectModified();
		}

		setPlane();
		s_pMainFrame->updatePlaneListBox();
		m_bIs2DScaleSet = false;
		setScale();
		onAdjustToWing();
		setControls();

		updateView();
	}

	delete pModPlane; // Clean up
}




/**
 * Exports the data from the active WOpp to the text file
 */
void QMiarex::onExportCurPOpp()
{
	if(!m_pCurPOpp)return ;// is there anything to export ?

	int iStrip,j,k,l,p, coef;
	XFLR5::enumTextFileType exporttype;
	QString filter;
	if(Settings::s_ExportFileType==XFLR5::TXT) filter = "Text File (*.txt)";
	else                                       filter = "Comma Separated Values (*.csv)";

	QString FileName, sep, str, strong, Format;

	strong = QString("a=%1_v=%2").arg(m_pCurPOpp->m_pPlaneWOpp[0]->m_Alpha, 5,'f',2).arg(m_pCurPOpp->m_pPlaneWOpp[0]->m_QInf*Units::mstoUnit(),6,'f',2);
	Units::getSpeedUnitLabel(str);
	strong = m_pCurPOpp->m_pPlaneWOpp[0]->m_WingName+"_"+strong+str;

	strong.replace(" ","");
	strong.replace("/", "");
	FileName = QFileDialog::getSaveFileName(this, tr("Export OpPoint"),
											Settings::s_LastDirName +'/'+strong,
											tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
											&filter);

	if(!FileName.length()) return;
	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) Settings::s_ExportFileType = XFLR5::CSV;
	else       Settings::s_ExportFileType = XFLR5::TXT;
	exporttype = Settings::s_ExportFileType;


	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	if(exporttype==XFLR5::TXT) sep = ""; else sep=",";


	out << VERSIONNAME;
	out << "\n\n";

	out << m_pCurPOpp->planeName()<< "\n";

	strong = m_pCurPOpp->polarName() + "\n";
	out << strong;
	strong = QString("QInf  ="+sep+" %1 "+sep).arg(m_pCurPOpp->m_QInf*Units::mstoUnit(),11, 'f', 6);
	Units::getSpeedUnitLabel(str);
	strong+=str+"\n";
	out << strong;

	strong = QString("Alpha = "+sep+"%1\n").arg(m_pCurPOpp->alpha(),11, 'f', 6);
	out << strong;

	strong = QString("Beta  = "+sep+"%1").arg(m_pCurPOpp->m_Beta, 8,'f',3);
	strong += QString::fromUtf8("°\n");
	out << strong;

	strong = QString("Phi   = "+sep+"%1").arg(m_pCurPOpp->m_Bank, 8,'f',3);
	strong += QString::fromUtf8("°\n");
	out << strong;

	strong = QString("Ctrl  = "+sep+"%1\n").arg(m_pCurPOpp->m_Ctrl, 8,'f',3);
	out << strong;

	strong = QString("CL    = "+sep+"%1\n").arg(m_pCurPOpp->m_CL,11, 'f', 6);
	out << strong;

	strong = QString("Cy    = "+sep+"%1\n").arg(m_pCurPOpp->m_CY,11, 'f', 6);
	out << strong;

	if(exporttype==XFLR5::TXT) strong = QString(tr("Cd    = %1     ICd   = %2     PCd   = %3\n"))
		.arg(m_pCurPOpp->m_ICD+m_pCurPOpp->m_VCD,11, 'f', 6)
		.arg(m_pCurPOpp->m_ICD,11, 'f', 6)
		.arg(m_pCurPOpp->m_VCD,11, 'f', 6);
	else        strong = QString(tr("Cd=,%1,ICd=, %2,PCd=, %3\n"))
		.arg(m_pCurPOpp->m_ICD+m_pCurPOpp->m_VCD,11, 'f', 6)
		.arg(m_pCurPOpp->m_ICD,11, 'f', 6)
		.arg(m_pCurPOpp->m_VCD,11, 'f', 6);
	out << strong;

	strong = QString(tr("Cl   = ")+sep+"%1\n").arg(m_pCurPOpp->m_GRm, 11,'g',6);
	out << strong;
	strong = QString(tr("Cm   =")+sep+" %1\n").arg(m_pCurPOpp->m_GCm, 11,'g',6);
	out << strong;

	if(exporttype==XFLR5::TXT) strong = QString(tr("ICn   = %1     PCn   = %2 \n")).arg(m_pCurPOpp->m_IYm, 11, 'f', 6).arg(m_pCurPOpp->m_GYm, 11, 'f', 6);
	else                       strong = QString(tr("ICn=, %1,PCn=, %2\n")).arg(m_pCurPOpp->m_IYm, 11, 'f', 6).arg(m_pCurPOpp->m_GYm, 11, 'f', 6);
	out << strong;

	if(exporttype==XFLR5::TXT) strong = QString("XCP   = %1     YCP   = %2     ZCP   = %3  \n").arg(m_pCurPOpp->m_CP.x, 11, 'f', 6).arg(m_pCurPOpp->m_CP.y, 11, 'f', 6).arg(m_pCurPOpp->m_CP.z, 11, 'f', 6);
	else                       strong = QString("XCP=, %1, YCP=, %2, ZCP=, %3 \n").arg(m_pCurPOpp->m_CP.x, 11, 'f', 6).arg(m_pCurPOpp->m_CP.y, 11, 'f', 6).arg(m_pCurPOpp->m_CP.z, 11, 'f', 6);
	out << strong;

	if(exporttype==XFLR5::TXT) strong = QString("XNP   = %1\n").arg(m_pCurPOpp->m_XNP, 11, 'f', 6);
	else                       strong = QString("XNP=, %1\n").arg(m_pCurPOpp->m_XNP, 11, 'f', 6);
	out << strong;


	strong = QString(tr("Bending =")+sep+" %1\n\n").arg(m_pCurPOpp->m_pPlaneWOpp[0]->m_MaxBending, 11, 'f', 6);
	out << strong;

	if(m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
	{
		//export non dimensional stability derivatives
		if(exporttype==XFLR5::TXT)
		{
//			complex<double> c, angle;
			double u0 = m_pCurPOpp->m_QInf;
			double mac = m_pCurWPolar->referenceArea();
			double b = m_pCurWPolar->referenceSpanLength();

			strong = "\n\n   ___Longitudinal modes____\n\n";
			out << strong;

			strong = QString("      Eigenvalue:  %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
				  .arg(m_pCurPOpp->m_EigenValue[0].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[0].imag(),9, 'g', 4)
				  .arg(m_pCurPOpp->m_EigenValue[1].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[1].imag(),9, 'g', 4)
				  .arg(m_pCurPOpp->m_EigenValue[2].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[2].imag(),9, 'g', 4)
				  .arg(m_pCurPOpp->m_EigenValue[3].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[3].imag(),9, 'g', 4);
			out << strong;
			strong=("                    _____________________________________________________________________________________________________\n");
			out << strong;

			strong = QString("             u/u0: %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(m_pCurPOpp->m_EigenVector[0][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[0][0].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[1][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[1][0].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[2][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[2][0].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[3][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[3][0].imag()/u0, 9, 'g', 4);
			out << strong;
			strong = QString("             w/u0: %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(m_pCurPOpp->m_EigenVector[0][1].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[0][1].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[1][1].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[1][1].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[2][1].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[2][1].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[3][1].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[3][1].imag()/u0, 9, 'g', 4);
			out << strong;
			strong = QString("     q/(2.u0/MAC): %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(m_pCurPOpp->m_EigenVector[0][2].real()/(2.*u0/mac), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[0][2].imag()/(2.*u0/mac), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[1][2].real()/(2.*u0/mac), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[1][2].imag()/(2.*u0/mac), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[2][2].real()/(2.*u0/mac), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[2][2].imag()/(2.*u0/mac), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[3][2].real()/(2.*u0/mac), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[3][2].imag()/(2.*u0/mac), 9, 'g', 4);
			out << strong;
			strong = QString("       theta(rad): %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4)
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4)
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4)
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4);
			out << strong;

			strong = "\n";
			out << strong;


			strong = "\n\n   ___Lateral modes____\n\n";
			out << strong;

			strong = QString("      Eigenvalue:  %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
				  .arg(m_pCurPOpp->m_EigenValue[4].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[4].imag(),9, 'g', 4)
				  .arg(m_pCurPOpp->m_EigenValue[5].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[5].imag(),9, 'g', 4)
				  .arg(m_pCurPOpp->m_EigenValue[6].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[6].imag(),9, 'g', 4)
				  .arg(m_pCurPOpp->m_EigenValue[7].real(),9, 'g', 4).arg(m_pCurPOpp->m_EigenValue[7].imag(),9, 'g', 4);
			out << strong;

			strong=("                    _____________________________________________________________________________________________________\n");
			out << strong;

			strong = QString("            v/u0 : %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(m_pCurPOpp->m_EigenVector[4][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[4][0].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[5][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[5][0].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[6][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[6][0].imag()/u0, 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[7][0].real()/u0, 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[7][0].imag()/u0, 9, 'g', 4);
			out << strong;
			strong = QString("    p/(2.u0/Span): %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(m_pCurPOpp->m_EigenVector[4][1].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[4][1].imag()/(2.0*u0/b), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[5][1].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[5][1].imag()/(2.0*u0/b), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[6][1].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[6][1].imag()/(2.0*u0/b), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[7][1].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[7][1].imag()/(2.0*u0/b), 9, 'g', 4);
			out << strong;
			strong = QString("    r/(2.u0/Span): %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(m_pCurPOpp->m_EigenVector[4][2].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[4][2].imag()/(2.0*u0/b), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[5][2].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[5][2].imag()/(2.0*u0/b), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[6][2].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[6][2].imag()/(2.0*u0/b), 9, 'g', 4)
					  .arg(m_pCurPOpp->m_EigenVector[7][2].real()/(2.0*u0/b), 9, 'g', 4).arg(m_pCurPOpp->m_EigenVector[7][2].imag()/(2.0*u0/b), 9, 'g', 4);
			out << strong;
			strong = QString("         phi(rad): %1+%2i   |   %3+%4i   |   %5+%6i   |   %7+%8i\n")
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4)
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4)
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4)
					  .arg(1.0, 9, 'g', 4).arg(0.0, 9, 'g', 4);
			out << strong;

			strong = "\n";
			out << strong;
		}
		out << "\n\n";
	}


	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pWing(iw))
		{
			out << pWing(iw)->m_WingName;
			for (l=0; l<m_pWOpp[iw]->m_nFlaps; l++)
			{
				strong = QString(tr("Flap ")+sep+"%1"+sep+" moment = "+sep+"%2 ").arg(l+1,4).arg(m_pWOpp[iw]->m_FlapMoment[l]*Units::NmtoUnit(), 9,'f',4);
				Units::getMomentUnitLabel(str);
				strong += str +"\n";
				out << strong;
			}
			out << ("\n");
			bool bCSV = (exporttype != XFLR5::TXT);
			m_pWOpp[iw]->exportWOpp(out, bCSV);
		}
	}

	if(m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
	{
		if(m_pCurPOpp) out << tr("Main Wing Cp Coefficients\n");
		else           out << tr("Wing Cp Coefficients\n");
		coef = 1;

		if(!m_pCurWPolar->bThinSurfaces())
		{
			coef = 2;
		}
		if(exporttype==XFLR5::TXT) out << tr(" Panel     CtrlPt.x        CtrlPt.y        CtrlPt.z       Nx      Ny       Nz        Area       Cp\n");
		else                       out << tr("Panel,CtrlPt.x,CtrlPt.y,CtrlPt.z,Nx,Ny,Nz,Area,Cp\n");

		if(exporttype==XFLR5::TXT) Format = "%1     %2     %3     %4     %5     %6     %7     %8     %9\n";
		else                       Format = "%1, %2, %3, %4, %5, %6, %7, %8, %9\n";


		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(pWing(iw))
			{
				out << pWing(iw)->m_WingName+ tr("Cp Coefficients")+"\n";
				p=0;
				iStrip = 0;
				for (j=0; j<pWing(iw)->m_Surface.size(); j++)
				{
					if(pWing(iw)->m_Surface.at(j)->m_bIsTipLeft && !m_pCurPOpp->m_bThinSurface) p+= pWing(iw)->m_Surface.at(j)->m_NXPanels;

					for(k=0; k<pWing(iw)->m_Surface.at(j)->m_NYPanels; k++)
					{
						iStrip++;
						strong = QString(tr("Strip %1\n")).arg(iStrip);
						out << strong;

						for(l=0; l<pWing(iw)->m_Surface.at(j)->m_NXPanels * coef; l++)
						{
							if(pWing(iw)->m_pWingPanel[p].m_Pos==MIDSURFACE)
							{
								strong = QString(Format).arg(p,4)
														.arg(pWing(iw)->m_pWingPanel[p].CtrlPt.x,11,'e',3)
														.arg(pWing(iw)->m_pWingPanel[p].CtrlPt.y,11,'e',3)
														.arg(pWing(iw)->m_pWingPanel[p].CtrlPt.z,11,'e',3)
														.arg(pWing(iw)->m_pWingPanel[p].Normal.x,11,'f',3)
														.arg(pWing(iw)->m_pWingPanel[p].Normal.y,11,'f',3)
														.arg(pWing(iw)->m_pWingPanel[p].Normal.z,11,'f',3)
														.arg(pWing(iw)->m_pWingPanel[p].Area,11,'e',3)
														.arg(m_pWOpp[iw]->m_dCp[p],11,'f',4);
							}
							else
							{
								strong = QString(Format).arg(p,4)
														.arg(pWing(iw)->m_pWingPanel[p].CollPt.x,11,'e',3)
														.arg(pWing(iw)->m_pWingPanel[p].CollPt.y,11,'e',3)
														.arg(pWing(iw)->m_pWingPanel[p].CollPt.z,11,'e',3)
														.arg(pWing(iw)->m_pWingPanel[p].Normal.x,11,'f',3)
														.arg(pWing(iw)->m_pWingPanel[p].Normal.y,11,'f',3)
														.arg(pWing(iw)->m_pWingPanel[p].Normal.z,11,'f',3)
														.arg(pWing(iw)->m_pWingPanel[p].Area,11,'e',3)
														.arg(m_pWOpp[iw]->m_dCp[p],11,'f',4);
							}
							out << strong;
							p++;
						}
					}
				}
			}
			out << ("\n\n");
		}
	}
	out << ("\n\n");

	XFile.close();
}



/**
 * Exports the data from the active polar to a text file
 */
void QMiarex::onExportCurWPolar()
{
	if (!m_pCurWPolar) return;

	QString FileName, filter;

	if(Settings::s_ExportFileType==XFLR5::TXT) filter = "Text File (*.txt)";
	else                                       filter = "Comma Separated Values (*.csv)";

	FileName = m_pCurWPolar->polarName();
	FileName.replace("/", "_");
	FileName.replace(".", "_");
	FileName = QFileDialog::getSaveFileName(this, tr("Export Polar"),
											Settings::s_LastDirName + "/"+FileName,
											tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
											&filter);

	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");

	if(filter.indexOf("*.txt")>0)
	{
		Settings::s_ExportFileType = XFLR5::TXT;
		if(FileName.indexOf(".txt")<0) FileName +=".txt";
	}
	else if(filter.indexOf("*.csv")>0)
	{
		Settings::s_ExportFileType = XFLR5::CSV;
		if(FileName.indexOf(".csv")<0) FileName +=".csv";
	}


	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

	QTextStream out(&XFile);
	exportToTextStream(m_pCurWPolar, out, Settings::s_ExportFileType);
	XFile.close();

	updateView();
}


void QMiarex::onExportWPolars()
{
	QString fileName, DirName;
	QFile XFile;
	QTextStream out;

	//select the directory for output
	DirName = QFileDialog::getExistingDirectory(this,  tr("Export Directory"), Settings::s_LastDirName);

	WPolar *pWPolar;
	for(int l=0; l<m_poaWPolar->size(); l++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(l);
		fileName = pWPolar->planeName() + "_" + pWPolar->polarName();
		fileName.replace("/", "_");
		fileName.replace(".", "_");
		fileName = DirName + "/" +fileName;
		if(Settings::s_ExportFileType==XFLR5::TXT) fileName += ".txt";
		else                                       fileName += ".csv";

		XFile.setFileName(fileName);
		if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			out.setDevice(&XFile);
			exportToTextStream(pWPolar, out, Settings::s_ExportFileType);
			XFile.close();
		}
	}
}


/**
 * Exports the geometrical data of the active plane to a text file readable by AVL
 */
void QMiarex::onExporttoAVL()
{
	if (!m_pCurPlane) return;
	QString filter =".avl";

	QString FileName, strong;


	FileName = m_pCurPlane->planeName();
	FileName.replace("/", " ");
	FileName = QFileDialog::getSaveFileName(this, tr("Export Plane"),
									Settings::s_LastDirName + "/"+FileName,
									tr("AVL Text File (*.avl)"), &filter);
	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	pos = FileName.indexOf(".avl", Qt::CaseInsensitive);
	if(pos<0) FileName += ".avl";


	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly)) return;

	QTextStream out(&XFile);
	out << "# \n";
	out << "# Note : check consistency of area unit and length units in this file\n";
	out << "# Note : check consistency with inertia units of the .mass file\n";
	out << "# \n";
	out << "# \n";

	strong = MainFrame::s_ProjectName;
	int len = strong.length();
	if (strong.right(1) == "*") strong = strong.left(len-1);
	if(!strong.length()) out << tr("Project");
	else out << strong;
	out << "\n";
	out << "0.0                                 | Mach\n";
	if(m_pCurPlane->m_Wing[0].m_bSymetric) out << ("0     0     0.0                     | iYsym  iZsym  Zsym\n");
	else                                   out << ("0     0     0.0                     | iYsym  iZsym  Zsym\n");

	strong = QString("%1   %2   %3   | Sref   Cref   Bref\n")
				  .arg(m_pCurPlane->planformArea()*Units::mtoUnit()*Units::mtoUnit(),9,'f',5)
				  .arg(m_pCurPlane->m_Wing[0].m_MAChord*Units::mtoUnit(),9,'f',5)
				  .arg(m_pCurPlane->planformSpan()*Units::mtoUnit(), 9,'f',5);
	out << strong;

	if(m_pCurPlane)
		strong = QString("%1   %2   %3   | Xref   Yref   Zref\n")
						   .arg(m_pCurPlane->CoG().x*Units::mtoUnit(),9,'f',5)
						   .arg(m_pCurPlane->CoG().y*Units::mtoUnit(),9,'f',5)
						   .arg(m_pCurPlane->CoG().z*Units::mtoUnit(),9,'f',5);

	out << strong;

	out << (" 0.00                               | CDp  (optional)\n");

	out << ("\n\n\n");

	int index = (double)qrand()/(double)RAND_MAX * 10000;

	exportAVLWing(m_pCurPlane->wing(0), out, index, 0.0, m_pCurPlane->WingTiltAngle(0));

	for(int iw=1; iw<MAXWINGS; iw++)
	{
		if(m_pCurPlane->wing(iw))
			exportAVLWing(m_pCurPlane->wing(iw), out, index+iw, 0.0, m_pCurPlane->WingTiltAngle(iw));
	}
	XFile.close();
}



/**
 * Export the wing geometry to a text file readable by AVL.
 * @param pWing a pointer to the instance of the wing which is to be exported
 * @param out the instance of the QTextStream to which the output will be directed
 * @param index a reference number used by AVL to idnitfy the wing
 * @param y the y value of the translation to be applied to the wing's geometry
 * @param Thetay the rotation about the y-axis to be applied to the geometry
 */
void QMiarex::exportAVLWing(Wing *pWing, QTextStream &out, int index, double y, double Thetay)
{
	if(!pWing) return;
	int j;
	QString strong, str;

	out << ("#========TODO: REMOVE OR MODIFY MANUALLY DUPLICATE SECTIONS IN SURFACE DEFINITION=========\n");
	out << ("SURFACE                      | (keyword)\n");
	out << (pWing->wingName());
	out << ("\n");
	out << ("#Nchord    Cspace   [ Nspan Sspace ]\n");

	strong = QString("%1        %2\n").arg(pWing->NXPanels(0)).arg(1.0,3,'f',1);
	out << (strong);

	out << ("\n");
	out << ("INDEX                        | (keyword)\n");
	strong = QString("%1                         | Lsurf\n").arg(index,4);
	out << (strong);

	if(!pWing->isFin())
	{
		out << ("\n");
		out << ("YDUPLICATE\n");
		out << ("0.0\n");
	}
	else if(pWing->isDoubleFin())
	{
		out << ("\n");
		out << ("YDUPLICATE\n");
		strong = QString("%1\n").arg(y,9,'f',4);
		out << (strong);
	}

	out << ("\n");
	out << ("SCALE\n");
	out << ("1.0  1.0  1.0\n");

	out << ("\n");
	out << ("TRANSLATE\n");
	out << ("0.0  0.0  0.0\n");

	out << ("\n");
	out << ("ANGLE\n");
	strong = QString("%1                         | dAinc\n").arg(Thetay,8,'f',3);
	out << (strong);

	out << ("\n\n");

	//write only right wing surfaces since we have provided YDUPLICATE
	Surface aSurface;
	int iFlap = 1;

	int NSurfaces = pWing->m_Surface.size();

	int startIndex = (pWing->isFin() ? 0 : (int)(NSurfaces/2));

	//write the first section


	for(j=startIndex; j<NSurfaces; j++)
	{
		out << ("#____PANEL ")<<j-startIndex+1<<"_______\n";
		aSurface.copy(pWing->m_Surface.at(j));

		//Remove the twist, since AVL processes it as a mod of the angle of attack thru the dAInc command
		aSurface.m_TwistA = aSurface.m_TwistB = 0.0;
		aSurface.setTwist();
		double mean_angle = 0.0;
		if(aSurface.m_bTEFlap)
		{
			if(aSurface.m_pFoilA && aSurface.m_pFoilB)
					mean_angle = (aSurface.m_pFoilA->m_TEFlapAngle + aSurface.m_pFoilB->m_TEFlapAngle)/2.0;
		}

		out << ("#______________\nSECTION                                                     |  (keyword)\n");

		strong = QString("%1 %2 %3 %4 %5  %6  %7   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n")
				 .arg(aSurface.m_LA.x       *Units::mtoUnit(),9,'f',4)
				 .arg(aSurface.m_LA.y       *Units::mtoUnit(),9,'f',4)
				 .arg(aSurface.m_LA.z       *Units::mtoUnit(),9,'f',4)
				 .arg(aSurface.chord(0.0)   *Units::mtoUnit(),9,'f',4)
				 .arg(pWing->m_Surface.at(j)->m_TwistA,7,'f',3)
				 .arg(aSurface.m_NYPanels,3)
				 .arg(aSurface.m_YDistType,3);
		out << (strong);
		out << ("\n");
		out << ("AFIL 0.0 1.0\n");
		if(aSurface.m_pFoilA)  out << (aSurface.m_pFoilA->foilName() +".dat\n");
		out << ("\n");
		if(aSurface.m_bTEFlap)
		{
			out << ("CONTROL                                                     |  (keyword)\n");
			str = QString("_Flap_%1  ").arg(iFlap);
			strong = pWing->wingName();
			strong.replace(" ", "_");
			strong += str;

			if(fabs(mean_angle)>PRECISION) str = QString("%1  ").arg(1.0/mean_angle,5,'f',2);
			else                           str = "1.0   ";
			strong += str;

			str = QString("%1  %2  %3  %4  -1.0  ")
				  .arg(aSurface.m_pFoilA->m_TEXHinge/100.0,5,'f',3)
				  .arg(aSurface.m_HingeVector.x,10,'f',4)
				  .arg(aSurface.m_HingeVector.y,10,'f',4)
				  .arg(aSurface.m_HingeVector.z,10,'f',4);
			strong +=str + "| name, gain,  Xhinge,  XYZhvec,  SgnDup\n";
			out << (strong);
		}

		//write the end section of the surface
		out << ("\n#______________\nSECTION                                                     |  (keyword)\n");

		strong = QString("%1 %2 %3 %4 %5  %6  %7   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n")
				 .arg(aSurface.m_LB.x       *Units::mtoUnit(),9,'f',4)
				 .arg(aSurface.m_LB.y       *Units::mtoUnit(),9,'f',4)
				 .arg(aSurface.m_LB.z       *Units::mtoUnit(),9,'f',4)
				 .arg(aSurface.chord(1.0)   *Units::mtoUnit(),9,'f',4)
				 .arg(pWing->m_Surface.at(j)->m_TwistB,7,'f',3)
				 .arg(aSurface.m_NYPanels,3)
				 .arg(aSurface.m_YDistType,3);
		out << (strong);
		out << ("\n");
		out << ("AFIL 0.0 1.0\n");
		if(aSurface.m_pFoilB)  out << (aSurface.m_pFoilB->foilName() +".dat\n");
		out << ("\n");

		if(aSurface.m_bTEFlap)
		{
			out << ("CONTROL                                                     |  (keyword)\n");
			str = QString("_Flap_%1  ").arg(iFlap);
			strong = pWing->wingName();
			strong.replace(" ", "_");
			strong += str;

			if(fabs(mean_angle)>PRECISION) str = QString("%1  ").arg(1.0/mean_angle,5,'f',2);
			else                           str = "1.0   ";
			strong += str;
			str = QString("%1  %2  %3  %4  -1.0  ")
				  .arg(aSurface.m_pFoilB->m_TEXHinge/100.0,5,'f',3)
				  .arg(aSurface.m_HingeVector.x,10,'f',4)
				  .arg(aSurface.m_HingeVector.y,10,'f',4)
				  .arg(aSurface.m_HingeVector.z,10,'f',4);
			strong +=str + "| name, gain,  Xhinge,  XYZhvec,  SgnDup\n";
			out << (strong);
			out << ("\n");

			iFlap++;
		}
		out << ("\n");

	}

	out << ("\n\n");
}


/**
 * Export the wing geometry to a text file readable by AVL.
 * @param pWing a pointer to the instance of the wing which is to be exported
 * @param out the instance of the QTextStream to which the output will be directed
 * @param index a reference number used by AVL to idnitfy the wing
 * @param y the y value of the translation to be applied to the wing's geometry
 * @param Thetay the rotation about the y-axis to be applied to the geometry
 */
void QMiarex::exportAVLWing_Old(Wing *pWing, QTextStream &out, int index, double y, double Thetay)
{
	if(!pWing) return;
	int j;
	QString strong, str;

	out << ("#=================================================\n");
	out << ("SURFACE                      | (keyword)\n");
	out << (pWing->wingName());
	out << ("\n");
	out << ("#Nchord    Cspace   [ Nspan Sspace ]\n");

	strong = QString("%1        %2\n").arg(pWing->NXPanels(0)).arg(1.0,3,'f',1);
	out << (strong);

	out << ("\n");
	out << ("INDEX                        | (keyword)\n");
	strong = QString("%1                          | Lsurf\n").arg(index,3);
	out << (strong);

	if(!pWing->isFin())
	{
		out << ("\n");
		out << ("YDUPLICATE\n");
		out << ("0.0\n");
	}
	else if(pWing->isDoubleFin())
	{
		out << ("\n");
		out << ("YDUPLICATE\n");
		strong = QString("%1\n").arg(y,9,'f',4);
		out << (strong);
	}

	out << ("\n");
	out << ("SCALE\n");
	out << ("1.0  1.0  1.0\n");

	out << ("\n");
	out << ("TRANSLATE\n");
	out << ("0.0  0.0  0.0\n");

	out << ("\n");
	out << ("ANGLE\n");
	strong = QString("%1                         | dAinc\n").arg(Thetay,8,'f',3);
	out << (strong);

	out << ("\n\n");

	//write only right wing surfaces since we have provided YDUPLICATE
	Surface ASurface;
	int iFlap = 1;

	int NSurfaces = pWing->m_Surface.size();

	int startIndex = (pWing->isFin() ? 0 : (int)(NSurfaces/2));

	for(j=startIndex; j<NSurfaces; j++)
	{
		ASurface.copy(pWing->m_Surface.at(j));

		//Remove the twist, since AVL processes it as a mod of the angle of attack thru the dAInc command
		ASurface.m_TwistA = ASurface.m_TwistB = 0.0;
		ASurface.setTwist();
		out << ("#______________\nSECTION                                                     |  (keyword)\n");

		strong = QString("%1 %2 %3 %4 %5  %6  %7   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n")
				 .arg(ASurface.m_LA.x          *Units::mtoUnit(),9,'f',4)
				 .arg(ASurface.m_LA.y          *Units::mtoUnit(),9,'f',4)
				 .arg(ASurface.m_LA.z          *Units::mtoUnit(),9,'f',4)
				 .arg(ASurface.chord(0.0)   *Units::mtoUnit(),9,'f',4)
				 .arg(pWing->m_Surface.at(j)->m_TwistA,7,'f',3)
				 .arg(ASurface.m_NYPanels,3)
				 .arg(ASurface.m_YDistType,3);
		out << (strong);
		out << ("\n");
		out << ("AFIL 0.0 1.0\n");
		if(ASurface.m_pFoilA)  out << (ASurface.m_pFoilA->foilName() +".dat\n");
		out << ("\n");
		if(ASurface.m_bTEFlap)
		{
			out << ("CONTROL                                                     |  (keyword)\n");
			str = QString("_Flap_%1  ").arg(iFlap);
			strong = pWing->wingName();
			strong.replace(" ", "_");
			strong += str;
			double mean_angle = 0.0;

			if(ASurface.m_pFoilA && ASurface.m_pFoilB)
					mean_angle = (ASurface.m_pFoilA->m_TEFlapAngle + ASurface.m_pFoilB->m_TEFlapAngle)/2.0;
			if(qAbs(mean_angle)>PRECISION) str = QString("%1  ").arg(1.0/mean_angle,5,'f',2);
			else                           str = "1.0   ";
			strong += str;
			str = QString("%1  %2  %3  %4  -1.0  ")
				  .arg(ASurface.m_pFoilA->m_TEXHinge/100.0,5,'f',3)
				  .arg(ASurface.m_HingeVector.x,10,'f',4)
				  .arg(ASurface.m_HingeVector.y,10,'f',4)
				  .arg(ASurface.m_HingeVector.z,10,'f',4);
			strong +=str + "| name, gain,  Xhinge,  XYZhvec,  SgnDup\n";
			out << (strong);

			//write the same flap at the other end
/*			out << ("\n#______________\nSECTION                                                     |  (keyword)\n");

			strong = QString("%1 %2 %3 %4 %5  %6  %7   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n")
				 .arg(ASurface.m_LB.x          *MainFrame::m_mtoUnit,9,'f',4)
				 .arg(ASurface.m_LB.y          *MainFrame::m_mtoUnit,9,'f',4)
				 .arg(ASurface.m_LB.z          *MainFrame::m_mtoUnit,9,'f',4)
				 .arg(ASurface.GetChord(1.0)   *MainFrame::m_mtoUnit,9,'f',4)
				 .arg(m_Surface.at(j)->m_TwistB,7,'f',3)
				 .arg(ASurface.NYPanels,3)
				 .arg(ASurface.m_YDistType,3);
			out << (strong);

			out << ("\n");
			out << ("AFIL 0.0 1.0\n");
			out << (ASurface.m_pFoilB->foilName() +".dat\n");
			out << ("\n");

			out << ("CONTROL                                                     |  (keyword)\n");
			str = QString("_Flap_%1  ").arg(iFlap);
			strong = m_WingName;
			strong.replace(" ", "_");
			strong += str;

			if(qAbs(mean_angle)>0.0) str = QString("%1  ").arg(1.0/mean_angle,5,'f',2);
			else                     str = "1.0   ";
			strong += str;

			str = QString("%1  %2  %3  %4  -1.0  ")
				  .arg(ASurface.m_pFoilB->m_TEXHinge/100.0,5,'f',3)
				  .arg(ASurface.m_HingeVector.x,10,'f',4)
				  .arg(ASurface.m_HingeVector.y,10,'f',4)
				  .arg(ASurface.m_HingeVector.z,10,'f',4);
			strong +=str + "| name, gain,  Xhinge,  XYZhvec,  SgnDup\n\n";
			out << (strong);
*/
			iFlap++;
		}
	}
	//write last panel, if no flap before
	if(!ASurface.m_bTEFlap)
	{
		out << ("#______________\nSECTION                                                     |  (keyword)\n");
		strong = QString("%1 %2 %3 %4 %5  %6  %7   | Xle Yle Zle   Chord Ainc   [ Nspan Sspace ]\n")
				 .arg(ASurface.m_LB.x          *Units::mtoUnit(),9,'f',4)
				 .arg(ASurface.m_LB.y          *Units::mtoUnit(),9,'f',4)
				 .arg(ASurface.m_LB.z          *Units::mtoUnit(),9,'f',4)
				 .arg(ASurface.chord(1.0)   *Units::mtoUnit(),9,'f',4)
				 .arg(pWing->m_Surface[j-1]->m_TwistB,7,'f',3)
				 .arg(ASurface.m_NYPanels,3)
				 .arg(ASurface.m_YDistType,3);

		out << (strong);
		out << ("\n");
		out << ("AFIL 0.0 1.0\n");
		if(ASurface.m_pFoilB) out << (ASurface.m_pFoilB->foilName() +".dat\n");
		out << ("\n");
	}

	out << ("\n\n");
}


/**
 * The user has toggled the display switch for the fin curve in the OpPoint view
 */
void QMiarex::onFinCurve()
{
	m_bShowWingCurve[3] = !m_bShowWingCurve[3];

	s_bResetCurves = true;
	updateView();
}



/**
 * The user has toggled the display switch for the elevator curve in the OpPoint view
 */
void QMiarex::onStabCurve()
{
	m_bShowWingCurve[2] = !m_bShowWingCurve[2];
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has changed one of the scale in the GL3DScale widget
 */
void QMiarex::onGL3DScale()
{
	if(m_iView != XFLR5::W3DVIEW)
	{
//		m_pctrl3DSettings->setChecked(false);
		return;
	}
	if(s_pMainFrame->m_pctrl3DScalesWidget->isVisible()) s_pMainFrame->m_pctrl3DScalesWidget->hide();
	else                                                 s_pMainFrame->m_pctrl3DScalesWidget->show();

	s_pMainFrame->m_pW3DScalesAct->setChecked(s_pMainFrame->m_pctrl3DScalesWidget->isVisible());
//	if(m_pctrl3DSettings->isChecked()) s_pMainFrame->m_pctrl3DScalesWidget->show();
//	else                               s_pMainFrame->m_pctrl3DScalesWidget->hide();
}




/**
 * The user has requested that all polars curves be hidden
 */
void QMiarex::onHideAllWPolars()
{
	int i;
	WPolar *pWPolar;
	for (i=0; i<m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		pWPolar->isVisible() = false;
//		if(pWPolar->polarType()==XFLR5::STABILITYPOLAR) pWPolar->points() = false;
	}

	emit projectModified();
	setCurveParams();
	s_bResetCurves = true;
	updateView();
}



/**
 * The user has requested that all curves of the oppoints associated to the active polar be hidden
 */
void QMiarex::onHideAllWPlrOpps()
{
	int i;
	m_bCurPOppOnly = false;

	PlaneOpp *pPOpp;
	if(m_pCurPlane)
	{
		for (i=0; i< m_poaPOpp->size(); i++)
		{
			pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
			if (pPOpp->planeName() == m_pCurWPolar->planeName() &&
				pPOpp->polarName()   == m_pCurWPolar->polarName())
			{
				pPOpp->isVisible() = false;
			}
		}
	}
	emit projectModified();
	setCurveParams();

	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested that all oppoint curves be hidden
 */
void QMiarex::onHideAllWOpps()
{
	int i;
	m_bCurPOppOnly = false;

	PlaneOpp *pPOpp;
	for (i=0; i< m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		pPOpp->isVisible() = false;
	}
	emit projectModified();
	setCurveParams();

	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested that all curves of the oppoints associated to the active wing or plane be hidden
 */
void QMiarex::onHidePlaneOpps()
{
	PlaneOpp *pPOpp;
	int i;
	for (i=0; i< m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if (pPOpp->planeName() == m_pCurWPolar->planeName())
		{
			pPOpp->isVisible() = false;
		}
	}

	emit projectModified();
	setCurveParams();
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested that all curves of the oppoints associated to the active polar be hidden
 */
void QMiarex::onHidePlaneWPolars()
{
	if(!m_pCurPlane) return;
	int i;
	QString PlaneName;
	if(m_pCurPlane)     PlaneName = m_pCurPlane->planeName();
	else return;

	WPolar *pWPolar;
	for (i=0; i<m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if (pWPolar->planeName() == PlaneName)
		{
			pWPolar->isVisible() = false;
			if(pWPolar->polarType()==XFLR5::STABILITYPOLAR) pWPolar->points() = false;
		}
	}

	setCurveParams();
	emit projectModified();
	s_bResetCurves = true;
	updateView();
}



/**
 * The user has requested the import of polar results from text file(s).
 * Creates new WPolar object(s), fills them with the data from the text file,
 * and adds them to the array
 */
void QMiarex::onImportWPolars()
{
	bool bRead = true;
	QString polarName, PlaneName;
	QString strong, str;
	QStringList PathNames;
	QString PathName;

	PathNames = QFileDialog::getOpenFileNames(s_pMainFrame, tr("Open File"),
											  Settings::s_LastDirName,
											  tr("Plane Polar Format (*.*)"));
	if(!PathNames.size()) return;

	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

	for (int i=0; i<PathNames.size(); i++)
	{
		PathName = PathNames.at(i);
		QFile XFile(PathName);
		if (!XFile.open(QIODevice::ReadOnly))
		{
			QString strange = tr("Could not read the file\n")+PathName;
			QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
		}
		else
		{
			QTextStream inStream(&XFile);

			strong = inStream.readLine();// XFLR5 version
			strong = inStream.readLine();// blank line
			strong = inStream.readLine();// plane name

			PlaneName = strong.right(strong.length()-19);
			PlaneName = PlaneName.trimmed();

			Plane *pPlane = Objects3D::getPlane(PlaneName);

			if(!pPlane)
			{
				str = tr("No Plane with the name ")+PlaneName;
				str+= tr("\ncould be found. The polar(s) will not be stored");

				QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
			}
			else
			{
				WPolar *pWPolar = new WPolar();

				pWPolar->planeName() = PlaneName;
				pWPolar->referenceArea()        = pPlane->planformArea();
				pWPolar->referenceChordLength() = pPlane->mac();
				pWPolar->referenceSpanLength()  = pPlane->planformSpan();

				strong = inStream.readLine();
				polarName = strong.right(strong.length()-19);
				pWPolar->polarName() = polarName;

				strong = inStream.readLine();// blank line

				strong = inStream.readLine();// "   alpha      CL          ICd   ..."

				bRead = true;
				while( bRead)
				{
					strong = inStream.readLine(); // one line with polar results
					if(strong.length())
					{
						QStringList values = strong.split(" ", QString::SkipEmptyParts);
			//			alpha      Beta       CL          CDi        CDv        CD         CY         Cl         Cm         Cn        Cni       QInf        XCP

						if(values.length()>=12)
						{
							PlaneOpp *pPOpp = new PlaneOpp;

							pPOpp->m_Alpha  = values.at(0).toDouble();
							pPOpp->m_Beta   = values.at(1).toDouble();
							pPOpp->m_CL     = values.at(2).toDouble();
							pPOpp->m_ICD    = values.at(3).toDouble();
							pPOpp->m_VCD    = values.at(4).toDouble();
			//				pPOpp->m_TCd    = values.at(5).toDouble();
							pPOpp->m_CY     = values.at(6).toDouble();
							pPOpp->m_GRm    = values.at(7).toDouble();
							pPOpp->m_GCm    = values.at(8).toDouble();
							pPOpp->m_GYm    = values.at(9).toDouble();
							pPOpp->m_IYm    = values.at(10).toDouble();
							pPOpp->m_QInf   = values.at(11).toDouble();
							pPOpp->m_CP.x   = values.at(12).toDouble();

							pWPolar->addPlaneOpPoint(pPOpp);
						}
						else bRead = false;
					}
					else bRead = false;
				}

				pWPolar->curveColor() = randomColor(!Settings::isLightTheme());
				Objects3D::addWPolar(pWPolar);
				XFile.close();
			}
		}
	}

	setWPolar();
	s_pMainFrame->updateWPolarListBox();
	updateView();
	emit projectModified();
}


/**
 * Toggles the flag which requests the initialization of the start parameters at the launch of an LLT analysis
 */
void QMiarex::onInitLLTCalc()
{
	m_bInitLLTCalc = m_pctrlInitLLTCalc->isChecked();
}


/**
 * The user has requested to store the active curve in the Cp graph display
 * Duplicates the curve and adds it to the graph
 */
void QMiarex::onKeepCpSection()
{
	Curve *pCurrentCurve, *pNewCurve;

	pCurrentCurve = m_CpGraph.curve(0);
	pNewCurve = m_CpGraph.addCurve();
	pNewCurve->copyData(pCurrentCurve);
	pNewCurve->duplicate(pCurrentCurve);

//	pNewCurve->setCurveName(pCurrentCurve->curveName());
//	pNewCurve->setColor(pCurrentCurve->color());

	m_CpLineStyle.m_Color = randomColor(!Settings::isLightTheme());
	pCurrentCurve->setColor(m_CpLineStyle.m_Color);

	m_CpLineStyle.m_Style = 0;
	m_CpLineStyle.m_Width = 1;
	m_CpLineStyle.m_PointStyle = 0;
	setCurveParams();

	createCpCurves();
	updateView();
}



/**
 * The user has requested the launch of the dialog box used to manage the array of planes
 */
void QMiarex::onManagePlanes()
{
	QString PlaneName = "";
	if(m_pCurPlane)     PlaneName = m_pCurPlane->planeName();

	ManagePlanesDlg uDlg(s_pMainFrame);
	uDlg.initDialog(PlaneName);
	uDlg.exec();

	m_pCurPlane = NULL;
	m_pCurWPolar = NULL;

	if(uDlg.m_pPlane) setPlane(uDlg.m_pPlane->planeName());
	else setPlane();

	if(uDlg.m_bChanged) emit projectModified();

	s_pMainFrame->updatePlaneListBox();
	setControls();

	m_bResetglGeom = true;
	m_bResetglMesh = true;
	updateView();
}


/**
 * The user has toggled the display of moments in the 3D view
 **/
void QMiarex::onMoment()
{
	m_bMoments = m_pctrlMoment->isChecked();
	updateView();
}




/**
 * The user has requested the creation of a new plane.
 * Launches the dialog box, and stores the plane in the array i.a.w. user instructions
 */
void QMiarex::onNewPlane()
{
	Plane* pPlane = new Plane;

	PlaneDlg plDlg(s_pMainFrame);
	plDlg.m_pPlane = pPlane;
	plDlg.m_bAcceptName = true;
	plDlg.initDialog();

	if(QDialog::Accepted == plDlg.exec())
	{
		emit projectModified();

		if(Objects3D::getPlane(pPlane->planeName()))
			m_pCurPlane = Objects3D::setModPlane(pPlane);
		else
		{
			Objects3D::addPlane(pPlane);
			m_pCurPlane = pPlane;
		}
		setPlane();
		s_pMainFrame->updatePlaneListBox();
		m_bResetglLegend = true;
	}
	else
	{
		delete pPlane;
	}

	setControls();
	updateView();
}



/**
 * The user has requested the creation of a new plane.
 * Launches the dialog box, and stores the plane in the array i.a.w. user instructions
 */
void QMiarex::onNewPlaneObject()
{
	Plane* pPlane = new Plane;

	EditPlaneDlg eplDlg(s_pMainFrame);
	eplDlg.initDialog(pPlane);

	if(QDialog::Accepted == eplDlg.exec())
	{
		emit projectModified();

		if(Objects3D::getPlane(pPlane->planeName()))
			m_pCurPlane = Objects3D::setModPlane(pPlane);
		else
		{
			Objects3D::addPlane(pPlane);
			m_pCurPlane = pPlane;
		}
		setPlane();
		s_pMainFrame->updatePlaneListBox();
		m_bResetglLegend = true;
	}
	else
	{
		delete pPlane;
	}

	setControls();
	updateView();
}



/**
 * Reads the analysis input from the dialog boxes
 */
void QMiarex::onReadAnalysisData()
{
	m_bSequence = m_pctrlSequence->isChecked();
	m_bInitLLTCalc = m_pctrlInitLLTCalc->isChecked();

	if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::FIXEDAOAPOLAR)
	{
		m_QInfMin   = m_pctrlAlphaMin->value()         /Units::mstoUnit();
		m_QInfMax   = m_pctrlAlphaMax->value()         /Units::mstoUnit();
		m_QInfDelta = qAbs(m_pctrlAlphaDelta->value()) /Units::mstoUnit();
		if(qAbs(m_QInfDelta)<0.1)
		{
			m_QInfDelta = 1.0;
			m_pctrlAlphaDelta->setValue(1.0);
		}
	}
	else if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::BETAPOLAR)
	{
		m_BetaMin   = m_pctrlAlphaMin->value();
		m_BetaMax   = m_pctrlAlphaMax->value();
		m_BetaDelta = qAbs(m_pctrlAlphaDelta->value());
		if(qAbs(m_BetaDelta)<0.01)
		{
			m_BetaDelta = 0.01;
			m_pctrlAlphaDelta->setValue(0.01);
		}
	}
	else if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
	{
		m_ControlMin   = m_pctrlAlphaMin->value();
		m_ControlMax   = m_pctrlAlphaMax->value();
		m_ControlDelta = qAbs(m_pctrlAlphaDelta->value());
		if(qAbs(m_ControlDelta)<0.001)
		{
			m_ControlDelta = 0.001;
			m_pctrlAlphaDelta->setValue(0.001);
		}
	}
	else if(m_pCurWPolar)
	{
		m_AlphaMin   = m_pctrlAlphaMin->value();
		m_AlphaMax   = m_pctrlAlphaMax->value();
		m_AlphaDelta = qAbs(m_pctrlAlphaDelta->value());

		if(qAbs(m_AlphaDelta)<0.01)
		{
			m_AlphaDelta = 0.01;
			m_pctrlAlphaDelta->setValue(0.01);
		}
	}
}



/**
 * The user has requested a change to the type of polars which ought to be displayed
 */
void QMiarex::onPolarFilter()
{
	PolarFilterDlg pfDlg(s_pMainFrame);
	pfDlg.m_bMiarex = true;
	pfDlg.m_bType1 = m_bType1;
	pfDlg.m_bType2 = m_bType2;
	pfDlg.m_bType4 = m_bType4;
	pfDlg.m_bType7 = m_bType7;

	pfDlg.InitDialog();

	if(pfDlg.exec()==QDialog::Accepted)
	{
		m_bType1 = pfDlg.m_bType1;
		m_bType2 = pfDlg.m_bType2;
		m_bType4 = pfDlg.m_bType4;
		m_bType7 = pfDlg.m_bType7;
		s_bResetCurves = true;
		updateView();
	}
}


/**
 * The user has requested that the active polar be renames
 * Changes the polar name and updates the references in all child oppoints
 */
void QMiarex::onRenameCurWPolar()
{
	if(!m_pCurWPolar) return;
	if(!m_pCurPlane) return;

	WPolar *pWPolar = NULL;
	WPolar *pOldWPolar = NULL;

	//make a list of existing WPolar names for that Plane
	QStringList NameList;
	for(int k=0; k<m_poaWPolar->size(); k++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(k);
		if(pWPolar->planeName()==m_pCurPlane->planeName())
			NameList.append(pWPolar->polarName());
	}

	RenameDlg dlg;
	dlg.initDialog(&NameList, m_pCurWPolar->polarName(), QObject::tr("Enter the new name for the Polar:"));
	int resp = dlg.exec();
	if(resp==QDialog::Rejected)
	{
		return;
	}
	else if(resp==10)
	{
		//the user wants to overwrite an existing name
		if(dlg.newName()==m_pCurWPolar->polarName()) return; //what's the point ?

		// it's a real overwrite
		// so find and delete the existing WPolar with the new name
		pWPolar = NULL;
		for(int ipb=0; ipb<m_poaWPolar->size(); ipb++)
		{
			pOldWPolar = (WPolar*)m_poaWPolar->at(ipb);
			if(pOldWPolar->polarName()==dlg.newName() && pOldWPolar->planeName()==m_pCurPlane->planeName())
			{
				Objects3D::deleteWPolar(pOldWPolar);
				break;
			}
		}
	}

	//ready to insert
	//remove the WPolar from its current position in the array
	for (int l=0; l<m_poaWPolar->size();l++)
	{
		pOldWPolar = (WPolar*)m_poaWPolar->at(l);
		if(pOldWPolar==m_pCurWPolar)
		{
			m_poaWPolar->removeAt(l);
			break;
		}
	}

	//set the new name
	for (int l=m_poaPOpp->size()-1;l>=0; l--)
	{
		PlaneOpp *pPOpp = (PlaneOpp*)m_poaPOpp->at(l);
		if (pPOpp->planeName() == m_pCurPlane->planeName() && pPOpp->polarName()==m_pCurWPolar->polarName())
		{
			pPOpp->polarName() = dlg.newName();
		}
	}

	m_pCurWPolar->polarName() = dlg.newName();

	//insert alphabetically
	bool bInserted = false;
	for (int l=0; l<m_poaWPolar->size();l++)
	{
		pOldWPolar = (WPolar*)m_poaWPolar->at(l);

		if(pOldWPolar->polarName().compare(m_pCurWPolar->polarName(), Qt::CaseInsensitive) >0)
		{
			//then insert before
			m_poaWPolar->insert(l, m_pCurWPolar);
			bInserted = true;
			break;
		}
	}

	if(!bInserted) m_poaWPolar->append(m_pCurWPolar);


	s_pMainFrame->updateWPolarListBox();

	emit projectModified();

	s_bResetCurves = true;
	updateView();
}



/**
 * The user has requested that the active wing ot plane be renames
 * Changes the name and updates the references in all child polars and oppoints
 */
void QMiarex::onRenameCurPlane()
{
	//Rename the currently selected Plane

	if(!m_pCurPlane)	return;
	Objects3D::renamePlane(m_pCurPlane->planeName());
	s_pMainFrame->updatePlaneListBox();
	m_bResetTextLegend = true;
	updateView();

	emit projectModified();
}


/**
 * The user has requested a deletion of all the previously stored curves in the Cp Graph
 */
void QMiarex::onResetCpSection()
{
	for(int i=m_CpGraph.curveCount()-1; i>3 ;i--)	m_CpGraph.deleteCurve(i);
	createCpCurves();
	updateView();
}


/**
 * The user has requested that the results data of the current CWPolar object be deleted.
 * Deletes it and all its child operating points, and updates the graphs
 */
void QMiarex::onResetCurWPolar()
{
	if (!m_pCurWPolar) return;
	QString strong = tr("Are you sure you want to reset the content of the polar :\n")+  m_pCurWPolar->polarName() +"?\n";
	if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), strong,
												  QMessageBox::Yes|QMessageBox::No,
												  QMessageBox::Yes)) return;
	m_bResetTextLegend = true;
	m_pCurWPolar->clearData();
	PlaneOpp *pPOpp;
	if(m_pCurPlane)
	{
		for(int i=m_poaPOpp->size()-1; i>=0; --i)
		{
			pPOpp = (PlaneOpp*) m_poaPOpp->at(i);
			if(pPOpp->polarName()==m_pCurWPolar->polarName() && pPOpp->planeName()==m_pCurPlane->planeName())
			{
				m_poaPOpp->removeAt(i);
				delete pPOpp;
			}
		}
	}
	s_pMainFrame->updatePOppListBox();
	m_pCurPOpp = NULL;

	if(m_iView==XFLR5::WPOLARVIEW)
	{
		if(m_pCurWPolar)
		{
			QString PolarProps;
			getPolarProperties(m_pCurWPolar, PolarProps);
			m_pctrlPolarProps->setText(PolarProps);
		}
	}

	emit projectModified();
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has toggled the switch for a sequential analyis
 */
void QMiarex::onSequence()
{
	m_bSequence = m_pctrlSequence->isChecked();
	m_pctrlAlphaMax->setEnabled(m_bSequence);
	m_pctrlAlphaDelta->setEnabled(m_bSequence);
}



/**
 * The user has requested the display of all the operating point curves
 */
void QMiarex::onShowAllWOpps()
{
	int i;
	//Switch all WOpps view to on for all Plane and WPolar
	m_bCurPOppOnly = false;
	s_pMainFrame->m_pShowCurWOppOnly->setChecked(false);

	PlaneOpp *pPOpp;
	for (i=0; i< m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		pPOpp->isVisible() = true;
	}

	emit projectModified();
	setCurveParams();

	s_bResetCurves = true;
	updateView();
}



/**
 * The user has requested the display of all the polar curves
 */
void QMiarex::onShowAllWPolars()
{
	int i;
	WPolar *pWPolar;
	for (i=0; i<m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		pWPolar->isVisible() = true;
	}

	emit projectModified();
	setCurveParams();
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested the display exclusively of all the polar curves associated to the active wing or plane.
 * The display of all other polar curves is turned off
 */
void QMiarex::onShowPlaneWPolarsOnly()
{
	if(!m_pCurPlane) return;

	WPolar *pWPolar;
	for (int i=0; i<m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		pWPolar->isVisible() = (pWPolar->planeName() == m_pCurPlane->planeName());
	}

	setCurveParams();
	emit projectModified();
	s_bResetCurves = true;
	updateView();
}


/** Displays only the operating points of the currently selected WPolar */
void QMiarex::onShowWPolarOppsOnly()
{
	if(!m_pCurPlane || !m_pCurWPolar) return;
	for(int i=0; i<m_poaPOpp->size(); i++)
	{
		PlaneOpp *pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if(pPOpp->planeName().compare(m_pCurPlane->planeName())==0 && pPOpp->polarName().compare(m_pCurWPolar->polarName())==0)
		{
			pPOpp->isVisible() = true;
		}
		else pPOpp->isVisible() = false;
	}
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested the display of all the polar curves associated to the active wing or plane
 */
void QMiarex::onShowPlaneWPolars()
{
	if(!m_pCurPlane) return;
	int i;
	QString PlaneName;
	if(m_pCurPlane)     PlaneName = m_pCurPlane->planeName();
	else return;

	WPolar *pWPolar;
	for (i=0; i<m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if (pWPolar->planeName() == PlaneName) pWPolar->isVisible() = true;
	}


	setCurveParams();
	emit projectModified();
	s_bResetCurves = true;
	updateView();
}



/**
 * The user has requested the display of all the operating point curves for the active wing or plane
 */
void QMiarex::onShowPlaneOpps()
{
	PlaneOpp *pPOpp;
	int i;
	for (i=0; i< m_poaPOpp->size(); i++)
	{
		pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
		if (pPOpp->planeName() == m_pCurWPolar->planeName())
		{
			pPOpp->isVisible() = true;
		}
	}

	emit projectModified();
	setCurveParams();
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested the display of all the operating point curves for the active polar
 */
void QMiarex::onShowAllWPlrOpps()
{
	int i;
	//Switch all WOpps view to on for the current Plane and WPolar
	m_bCurPOppOnly = false;

	PlaneOpp *pPOpp;
	if(m_pCurPlane)
	{
		for (i=0; i< m_poaPOpp->size(); i++)
		{
			pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
			if (pPOpp->planeName() == m_pCurWPolar->planeName() &&
				pPOpp->polarName()   == m_pCurWPolar->polarName())
			{
				pPOpp->isVisible() = true;
			}
		}
	}
	emit projectModified();
	setCurveParams();

	s_bResetCurves = true;
	updateView();
}


/**
 * The user has toggled the display of flap moment value together with the other
 * operating point results in the 3D and operating point views
 */
void QMiarex::onShowFlapMoments()
{
	m_bShowFlapMoments = !m_bShowFlapMoments;
	m_bResetTextLegend = true;
	updateView();
}


/**
 * The user has toggled the display of the target curve in the lift graph in the operating point view
 */
void QMiarex::onShowTargetCurve()
{
	TargetCurveDlg dlg;
	dlg.initDialog(m_bShowEllipticCurve, m_bShowBellCurve, m_bMaxCL, m_BellCurveExp);
	dlg.exec();

	m_BellCurveExp = dlg.m_BellCurveExp;
	m_bMaxCL = dlg.m_bMaxCL;
	m_bShowEllipticCurve = dlg.m_bShowEllipticCurve;
	m_bShowBellCurve     = dlg.m_bShowBellCurve;

	s_bResetCurves = true;
	updateView();
}


/**
 * The user has toggled the display of the moment reference point in the operating point view
 */
void QMiarex::onShowXCmRef()
{
	QAction *pAction = qobject_cast<QAction *>(sender());
	m_bXCmRef = pAction->isChecked();
	updateView();
}

/**
 * The user has toggled the display of the forces acting on the panels in the 3D view
 */
void QMiarex::onPanelForce()
{
	m_bPanelForce	 = m_pctrlPanelForce->isChecked();
	m_bResetTextLegend = true;
	if(m_bPanelForce)
	{
		m_b3DCp =false;
		m_pctrlCp->setChecked(false);
	}
	if(m_iView == XFLR5::W3DVIEW)
	{
		if(!m_bAnimateWOpp) updateView();
	}
}


/**
 * The user has toggled the display of the lift in the operating point view or in the 3D view
 */
void QMiarex::onShowLift()
{
	m_bXCP	 = m_pctrlLift->isChecked();
	if(m_iView==XFLR5::WOPPVIEW || m_iView == XFLR5::W3DVIEW)
	{
		if(!m_bAnimateWOpp) updateView();
	}
}


/**
 * The user has toggled the display of the induced drag forces in the 3D view
 */
void QMiarex::onShowIDrag()
{
	m_bICd = m_pctrlIDrag->isChecked();
	m_bResetglDrag = true;
	if(m_iView==XFLR5::WOPPVIEW || m_iView == XFLR5::W3DVIEW)
	{
		if(!m_bAnimateWOpp) updateView();
	}
}


/**
 * The user has toggled the display of the viscous drag forces in the 3D view
 */
void QMiarex::onShowVDrag()
{
	m_bVCd = m_pctrlVDrag->isChecked();
	m_bResetglDrag = true;
	if(m_iView==XFLR5::WOPPVIEW || m_iView == XFLR5::W3DVIEW)
	{
		if(!m_bAnimateWOpp) updateView();
	}
}


/**
 * The user has toggled the display of the laminar to turbulent transition lines in the 3D view
 */
void QMiarex::onShowTransitions()
{
	m_bXTop = m_pctrlTrans->isChecked();
	m_bXBot = m_pctrlTrans->isChecked();
	if(m_iView==XFLR5::WOPPVIEW || m_iView == XFLR5::W3DVIEW)
	{
		if(!m_bAnimateWOpp) updateView();
	}
}


/**
 * The user has toggled the display of the active curve
 */
void QMiarex::onShowCurve()
{
	m_bCurveVisible = m_pctrlShowCurve->isChecked();
	updateCurve();
}


/**
 * The user has toggled the display of stability results between longitudinal and lateral directions
 */
void QMiarex::onStabilityDirection()
{
	//the user has clicked either the longitudinal or lateral mode display
	//so update the view accordingly
	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;

	m_bLongitudinal = pStabView->m_pctrlLongDynamics->isChecked();

	m_iRootLocusView = XFLR5::ONEGRAPH;

	for(int ig=0; ig<MAXTIMEGRAPHS; ig++) m_TimeGraph[ig]->deleteCurves();

	pStabView->m_pCurve = NULL;
	pStabView->fillCurveList();

//	if(m_bLongitudinal) m_pCurRLStabGraph = m_StabPlrGraph.at(0);
//	else                m_pCurRLStabGraph = m_StabPlrGraph.at(1);

//	if(m_iView==XFLR5::STABPOLARVIEW) m_pCurGraph = m_pCurRLStabGraph;

	pStabView->setMode();
	pStabView->setControls();
	setStabGraphTitles();

	setCurveParams();
	setControls();
	setGraphTiles(); //needed to switch between longitudinal and lateral graphs

	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested to change the display of stability results to the time view
 */
void QMiarex::onStabTimeView()
{
	stopAnimate();
	m_iView =  XFLR5::STABTIMEVIEW;
	setGraphTiles();

	m_bResetTextLegend = true;


	setGraphTiles();
	s_pMainFrame->setMainFrameCentralWidget();

	setCurveParams();
	setControls();

	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested to change the display of stability results to the root locus view
 */
void QMiarex::onRootLocusView()
{
	stopAnimate();
	m_iView = XFLR5::STABPOLARVIEW;
	m_bResetTextLegend = true;

	setGraphTiles();
	s_pMainFrame->setMainFrameCentralWidget();
	setCurveParams();
	setControls();
	s_bResetCurves = true;
	updateView();

}


/**
 * The user has requested to change the display of stability results to the modal view
 */
void QMiarex::onModalView()
{
	m_iView = XFLR5::W3DVIEW;

	m_bResetTextLegend = true;

	setGraphTiles();
	s_pMainFrame->setMainFrameCentralWidget();
	setCurveParams();
	setControls();
	updateView();
}


/**
 * The user has toggled the display of the streamlines in the 3D view
 */
void QMiarex::onStreamlines()
{
	m_bStream = m_pctrlStream->isChecked();
	if(m_pctrlStream->isChecked())
	{
//		m_bResetglStream = true;
	}
	if(m_iView==XFLR5::W3DVIEW) updateView();
	m_pGL3dView->setFocus();
}



/**
 * The user has toggled the display of the surfaces in the 3D view
 */
void QMiarex::onSurfaces()
{
	m_pGL3dView->m_bSurfaces = m_pctrlSurfaces->isChecked();
	if(m_pGL3dView->m_bSurfaces)
	{
		m_b3DCp = false;
		m_pctrlCp->setChecked(false);
	}
	updateView();
}



/**
 * The user has toggled the display of the velocity vectors on the surfaces in the 3D view
 */
void QMiarex::onSurfaceSpeeds()
{
	m_bSurfVelocities = m_pctrlSurfVel->isChecked();
	if(m_pctrlSurfVel->isChecked())
	{
//		m_bResetglStream = true;
	}
	if(m_iView==XFLR5::W3DVIEW) updateView();
	m_pGL3dView->setFocus();
}



/**
 * The user has requested a modification of the light settings in the 3D view
 */
void QMiarex::onSetupLight()
{
	if(m_iView!=XFLR5::W3DVIEW) return;

	s_pMainFrame->m_glLightDlg.setgl3dView(m_pGL3dView);
	s_pMainFrame->m_glLightDlg.show();
}


/**
 * The user has toggled the request to store or not the operating points of an analysis
 */
void QMiarex::onStoreWOpp()
{
	PlaneOpp::s_bStoreOpps = m_pctrlStoreWOpp->isChecked();
}


/**
 * The user has toggled the display of the curves for the second wing in case of a bi-plane
 *@todo not thouroughly tested
 */
void QMiarex::onWing2Curve()
{
	m_bShowWingCurve[1] = !m_bShowWingCurve[1];
	s_bResetCurves = true;
	updateView();
}


/**
 * The user has requested the edition of the inertia data for the current wing or plane
 * Updates the inertia, resets the depending polars, and deletes the obsolete operating points
 * Updates the display
 */
void QMiarex::onPlaneInertia()
{
	if(!m_pCurPlane) return;

	InertiaDlg iDlg(s_pMainFrame);
	iDlg.m_pPlane = NULL;
	iDlg.m_pWing  = NULL;
	iDlg.m_pBody  = NULL;

	Plane *pSavePlane = new Plane;
	Wing *pSaveWing = new Wing;
	WPolar *pWPolar;
	QString PlaneName;
	bool bHasResults = false;

	if(m_pCurPlane)
	{
		PlaneName = m_pCurPlane->planeName();
		pSavePlane->duplicate(m_pCurPlane);
		iDlg.m_pPlane = m_pCurPlane;
	}

	for (int i=0; i< m_poaWPolar->size(); i++)
	{
		pWPolar = (WPolar*)m_poaWPolar->at(i);
		if(pWPolar->dataSize() && pWPolar->planeName()==PlaneName && pWPolar->m_bAutoInertia)
		{
//			if(pWPolar->polarType()==XFLR5::STABILITYPOLAR)
//			{
				bHasResults = true;
				break;
//			}
		}
	}

	iDlg.initDialog();

	ModDlg mdDlg(s_pMainFrame);

	if(iDlg.exec()==QDialog::Accepted)
	{
		if(bHasResults)
		{
			mdDlg.m_Question = tr("The modification will erase all polar results associated to this Plane.\nContinue ?");
			mdDlg.initDialog();
			int Ans = mdDlg.exec();

			if (Ans == QDialog::Rejected)
			{
				//restore saved Plane
				if(m_pCurPlane)     m_pCurPlane->duplicate(pSavePlane);
				return;
			}
			else if(Ans==20)
			{
				if(m_pCurPlane)
				{
					//save mods to a new plane object
					Plane* pNewPlane= new Plane;
					pNewPlane->duplicate(m_pCurPlane);

					//restore geometry for initial plane
					m_pCurPlane->duplicate(pSavePlane);

					//set the new current plane
					m_pCurPlane = Objects3D::setModPlane(pNewPlane);
				}
				setPlane();
				s_pMainFrame->updatePlaneListBox();
				updateView();
				return;
			}

			//last case, user wants to overwrite, so reset all polars, WOpps and POpps with autoinertia associated to the Plane

			for (int i=0; i<m_poaWPolar->size(); i++)
			{
				pWPolar = (WPolar*)m_poaWPolar->at(i);
				if(pWPolar && pWPolar->planeName()==PlaneName && pWPolar->m_bAutoInertia)
				{
					pWPolar->clearData();
					if(m_pCurPlane) pWPolar->retrieveInertia(m_pCurPlane);

					for (int i=m_poaPOpp->size()-1; i>=0; i--)
					{
						PlaneOpp *pPOpp = (PlaneOpp*)m_poaPOpp->at(i);
						if(pPOpp && pPOpp->planeName()==PlaneName && pPOpp->polarName()==pWPolar->polarName())
						{
							m_poaPOpp->removeAt(i);
							delete pPOpp;
						}
					}
				}
			}

			m_pCurPOpp = NULL;
		}
		setWPolar();
		emit projectModified();
		s_bResetCurves = true;
		updateView();
	}
	else
	{
		//restore saved Plane
		if(m_pCurPlane)    m_pCurPlane->duplicate(pSavePlane);
	}
	delete pSavePlane;
	delete pSaveWing;
}


/**
 * The user ha requested to switch to the operating point view
 */
void QMiarex::onWOppView()
{
	m_bResetTextLegend = true;

	if(m_iView==XFLR5::WOPPVIEW)
	{
		setControls();
		updateView();
		return;
	}

	m_iView=XFLR5::WOPPVIEW;
	setGraphTiles();

	s_pMainFrame->setMainFrameCentralWidget();

	m_bIs2DScaleSet = false;
	setCurveParams();
	setControls();

	s_bResetCurves = true;
	updateView();
}



/**
 * The user has requested to switch to the polar view
 */
void QMiarex::onWPolarView()
{
	if (m_bAnimateWOpp) stopAnimate();

	m_bResetTextLegend = true;

	if(m_iView==XFLR5::WPOLARVIEW)
	{
		setControls();
		updateView();
		return;
	}
	m_iView=XFLR5::WPOLARVIEW;
	setGraphTiles();

//	if(!m_pCurWPlrGraph) m_pCurGraph = NULL;
//	else                 m_pCurGraph = m_pCurWPlrGraph;

	s_pMainFrame->setMainFrameCentralWidget();

	setCurveParams();
	setControls();

	s_bResetCurves = true;
	updateView();
}




/**
 * Draws the wing legend in the 2D operating point view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 */
void QMiarex::paintPlaneLegend(QPainter &painter, Plane *pPlane, WPolar *pWPolar, QRect drawRect)
{
	if(!pPlane) return;
	painter.save();

	QString Result, str, strong;
	QString str1;
	int margin,dheight;
	float ratio = 1.0;

	QPen textPen(Settings::s_TextColor);
	painter.setPen(textPen);
	QFont font(Settings::s_TextFont);
	ratio = m_pGL3dView->devicePixelRatio();
	font.setPointSize(Settings::s_TextFont.pointSize()*ratio);
	painter.setFont(font);
	painter.setRenderHint(QPainter::Antialiasing);

	margin = 10*ratio;

	QFontMetrics fm(font);
	dheight = fm.height();
	int D = 0;
	int LeftPos = margin;
	int ZPos    = drawRect.height()-13*dheight;

	if(pWPolar)
	{
		ZPos -= dheight;
		if(pWPolar->dataSize()>1) ZPos -= dheight;
	}


//	double area = pPlane->m_Wing[0].s_RefArea;
	if(pPlane && pWing(2)) ZPos -= dheight;

	painter.drawText(LeftPos, ZPos, pPlane->planeName());
	D+=dheight;
	QString length, surface;
	Units::getLengthUnitLabel(length);
	Units::getAreaUnitLabel(surface);

	str1 = QString(tr("Wing Span      =")+"%1 ").arg(pPlane->planformSpan()*Units::mtoUnit(),10,'f',3);
	str1 += length;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("xyProj. Span   =")+"%1 ").arg(pPlane->projectedSpan()*Units::mtoUnit(),10,'f',3);
	str1 += length;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Wing Area      =")+"%1 ").arg(pPlane->planformArea() * Units::m2toUnit(),10,'f',3);
	str1 += surface;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("xyProj. Area   =")+"%1 ").arg(pPlane->projectedArea() * Units::m2toUnit(),10,'f',3);
	str1 += surface;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	Units::getWeightUnitLabel(str);
	Result = QString(tr("Plane Mass     =")+"%1 ").arg(m_pCurPlane->totalMass()*Units::kgtoUnit(),10,'f',3);
	Result += str;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	Units::getAreaUnitLabel(strong);
	Result = QString(tr("Wing Load      =")+"%1 ").arg(m_pCurPlane->totalMass()*Units::kgtoUnit()/pPlane->projectedArea()/Units::m2toUnit(),10,'f',3);
	Result += str + "/" + strong;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	if(pPlane && pWing(2))
	{
		str1 = QString(tr("Tail Volume    =")+"%1").arg(pPlane->tailVolume(),10,'f',3);
		painter.drawText(LeftPos, ZPos+D, str1);
		D+=dheight;
	}

	str1 = QString(tr("Root Chord     =")+"%1 ").arg(pPlane->m_Wing[0].rootChord()*Units::mtoUnit(), 10,'f', 3);
	Result = str1+length;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	str1 = QString(tr("MAC            =")+"%1 ").arg(pPlane->mac()*Units::mtoUnit(), 10,'f', 3);
	Result = str1+length;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	str1 = QString(tr("TipTwist       =")+"%1").arg(pPlane->m_Wing[0].tipTwist(), 10,'f', 3) + QString::fromUtf8("°");
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Aspect Ratio   =")+"%1").arg(pPlane->aspectRatio(),10,'f',3);
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Taper Ratio    =")+"%1").arg(pPlane->taperRatio(),10,'f',3);
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Root-Tip Sweep =")+"%1").arg(pPlane->m_Wing[0].averageSweep(), 10,'f',3) + QString::fromUtf8("°");
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	if(pWPolar)
	{
		if(pWPolar->dataSize()>1)
		{
			str1.sprintf( "XNP = d(XCp.Cl)/dCl =%10.3f ", pWPolar->m_XNeutralPoint * Units::mtoUnit());
			str1 += length;
			painter.drawText(LeftPos, ZPos+D, str1);
			D+=dheight;
		}
		str1 = QString(tr("Mesh elements  =")+"%1").arg(m_theTask.calculateMatSize(),6);
		painter.drawText(LeftPos, ZPos+D, str1);
		D+=dheight;
	}
	painter.restore();
}


/**
 * Draws the legend of the operating point in the 2D and 3D operating point views
 * @param painter a reference to the QPainter object on which the view shall be drawn
 */
void QMiarex::paintPlaneOppLegend(QPainter &painter, QRect drawRect)
{
	if(!m_pCurPOpp) return;

	painter.save();

	QString Result, str;

	int i;
	int margin = 10;
	int dwidth, dheight;
	float ratio = 1.0;


	QPen textPen(Settings::s_TextColor);
	painter.setPen(textPen);
	QFont font(Settings::s_TextFont);
	if (m_iView == XFLR5::W3DVIEW)
		ratio = m_pGL3dView->devicePixelRatio();
	margin *= ratio;
	font.setPointSize(Settings::s_TextFont.pointSize()*ratio);
	painter.setFont(font);
	painter.setRenderHint(QPainter::Antialiasing);


	QFontMetrics fm(font);
	dheight = fm.height();
	dwidth = fm.averageCharWidth()*50;
	int D = 0;

	int RightPos = drawRect.right()-margin - dwidth;
	int ZPos	 = drawRect.height()-13*dheight;

	if(m_pCurPOpp && m_pCurPOpp->m_WPolarType==XFLR5::STABILITYPOLAR) ZPos -= dheight;
	if(m_pCurPOpp && m_pCurPOpp->m_bOut)                              ZPos -= dheight;
	if(m_pCurPOpp && m_pCurPOpp->analysisMethod()!=XFLR5::LLTMETHOD && m_bShowFlapMoments)
	{
		if(m_pCurPOpp->m_pPlaneWOpp[0]) ZPos -= dheight*m_pCurPOpp->m_pPlaneWOpp[0]->m_nFlaps;
		if(m_pCurPOpp->m_pPlaneWOpp[2]) ZPos -= dheight*m_pCurPOpp->m_pPlaneWOpp[2]->m_nFlaps;
		if(m_pCurPOpp->m_pPlaneWOpp[3]) ZPos -= dheight*m_pCurPOpp->m_pPlaneWOpp[3]->m_nFlaps;
	}

	if(m_pCurPOpp->m_bOut)
	{
		Result = tr("Point is out of the flight envelope");
		painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);
		D+=dheight;
	}

	Units::getSpeedUnitLabel(str);
	int l = str.length();
	if(l==2)      Result = QString("V = %1 ").arg(m_pCurPOpp->m_QInf*Units::mstoUnit(),8,'f',3);
	else if(l==3) Result = QString("V = %1 ").arg(m_pCurPOpp->m_QInf*Units::mstoUnit(),7,'f',2);
	else if(l==4) Result = QString("V = %1 ").arg(m_pCurPOpp->m_QInf*Units::mstoUnit(),6,'f',1);
	else          Result = "No unit defined for speed...";

	Result += str;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	int e = 8, f=3;


	Result = QString("Alpha = %1").arg(m_pCurPOpp->alpha(), e,'f',f) + QString::fromUtf8("°  ");
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("Beta = %1").arg(m_pCurPOpp->m_Beta, e,'f',f) + QString::fromUtf8("°  ");
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("CL = %1   ").arg(m_pCurPOpp->m_CL, e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("CD = %1   " ).arg(m_pCurPOpp->m_VCD+m_pCurPOpp->m_ICD, e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	/*		oswald=CZ^2/CXi/PI/allongement;*/
	double cxielli=m_pCurPOpp->m_CL*m_pCurPOpp->m_CL/PI/m_pCurPlane->m_Wing[0].m_AR;
	Result = QString("Efficiency = %1   ").arg(cxielli/m_pCurPOpp->m_ICD, e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("CL/CD = %1   ").arg(m_pCurPOpp->m_CL/(m_pCurPOpp->m_ICD+m_pCurPOpp->m_VCD), e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("Cm = %1   ").arg(m_pCurPOpp->m_GCm, e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("Cl = %1   ").arg(m_pCurPOpp->m_GRm, e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("Cn = %1   ").arg(m_pCurPOpp->m_GYm, e,'f',f);
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Units::getLengthUnitLabel(str);
	l = str.length();
	int c=8, d=3;
	if(l==1)  str+=" ";
	if(m_pCurPOpp->m_WPolarType==XFLR5::STABILITYPOLAR)
	{
		Result = QString("X_NP = %1 ").arg(m_pCurPOpp->m_XNP*Units::mtoUnit(), c,'f',d);
		Result += str;
		D+=dheight;
		painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);
	}

	Result = QString("X_CP = %1 ").arg(m_pCurPOpp->m_CP.x*Units::mtoUnit(), c, 'f', d);
	Result += str;
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	Result = QString("X_CG = %1 ").arg(m_pCurWPolar->CoG().x*Units::mtoUnit(), c, 'f', d);
	Result += str;
	D+=dheight;
	painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);

	if(m_pCurPOpp->analysisMethod()!=XFLR5::LLTMETHOD && m_bShowFlapMoments)
	{
		if(m_pCurPOpp->m_pPlaneWOpp[0])
		{
			for(i=0; i<m_pCurPOpp->m_pPlaneWOpp[0]->m_nFlaps; i++)
			{
			   Result.sprintf("Wing Flap %d Moment =%8.4f ", i+1, m_pCurPOpp->m_pPlaneWOpp[0]->m_FlapMoment[i]*Units::NmtoUnit());
			   Units::getMomentUnitLabel(str);
			   Result += str;
			   D+=dheight;
			   painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);
			}
		}
		if(m_pCurPOpp->m_pPlaneWOpp[2])
		{
			for(i=0; i<m_pCurPOpp->m_pPlaneWOpp[2]->m_nFlaps; i++)
			{
			   Result.sprintf("Elev Flap %d Moment =%8.4f ", i+1, m_pCurPOpp->m_pPlaneWOpp[2]->m_FlapMoment[i]*Units::NmtoUnit());
			   Units::getMomentUnitLabel(str);
			   Result += str;
			   D+=dheight;
			   painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);
			}
		}
		if(m_pCurPOpp->m_pPlaneWOpp[3])
		{
			for(i=0; i<m_pCurPOpp->m_pPlaneWOpp[3]->m_nFlaps; i++)
			{
			   Result.sprintf("Fin  Flap %d Moment =%8.4f ", i+1, m_pCurPOpp->m_pPlaneWOpp[3]->m_FlapMoment[i]*Units::NmtoUnit());
			   Units::getMomentUnitLabel(str);
			   Result += str;
			   D+=dheight;
			   painter.drawText(RightPos, ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, Result);
			}
		}
	}

	painter.restore();
}



/**
 * Launches a 3D panel analysis
 * @param V0 the initial aoa
 * @param VMax the final aoa
 * @param VDelta the increment
 * @param bSequence if true, the analysis will for a sequence of aoa from V0 to Vmax, if not only V0 shall be calculated
 */
void QMiarex::panelAnalyze(double V0, double VMax, double VDelta, bool bSequence)
{
	if(!m_pCurPlane || !m_pCurWPolar) return;


/*	//Join surfaces together
	int i,pl, pr;
	pl = 0;
	pr = m_theTask.m_SurfaceList.at(0)->m_NElements;
	for (i=0; i<m_theTask.m_SurfaceList.size()-1; i++)
	{
		if(!m_theTask.m_SurfaceList.at(i)->m_bIsTipRight)
		{
			if(m_theTask.m_SurfaceList.at(i)->m_bJoinRight)
				m_theTask.joinSurfaces(m_pCurWPolar, m_theTask.m_SurfaceList.at(i), m_theTask.m_SurfaceList.at(i+1), pl, pr);
		}
		pl  = pr;
		pr += m_theTask.m_SurfaceList.at(i+1)->m_NElements;
	}*/

//	m_pPanelAnalysisDlg->deleteTask();

//	PlaneAnalysisTask *pTask = new PlaneAnalysisTask;
//	memcpy(pTask, &m_theTask, sizeof(PlaneAnalysisTask));

	m_theTask.initializeTask(m_pCurPlane, m_pCurWPolar, V0, VMax, VDelta, bSequence);
	m_theTask.stitchSurfaces();
	m_pPanelAnalysisDlg->setTask(&m_theTask);
	m_pPanelAnalysisDlg->initDialog();
	m_pPanelAnalysisDlg->show();
	m_pPanelAnalysisDlg->analyze();

	if(!m_bLogFile || !PanelAnalysis::s_bWarning)
		m_pPanelAnalysisDlg->hide();

	setPlaneOpp(false, V0);
	s_pMainFrame->updatePOppListBox();

//	delete pTask;
	emit projectModified();
}



/**
 * Saves the user settings to the QSettings object
 * @param pSettings a pointer to the QSettings object
 * @return true if the save was successfull, false if an error was encountered
 */
bool QMiarex::saveSettings(QSettings *pSettings)
{
	QString strong;
	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;

	onReadAnalysisData();

	pSettings->beginGroup("Miarex");
	{
		pSettings->setValue("bXCmRef", m_bXCmRef);
		pSettings->setValue("bXTop", m_bXTop);
		pSettings->setValue("bXBot", m_bXBot);
		pSettings->setValue("bXCP", m_bXCP);
		pSettings->setValue("bPanelForce", m_bPanelForce);
		pSettings->setValue("bICd", m_bICd);
		pSettings->setValue("bVCd", m_bVCd);
		pSettings->setValue("bSurfaces", m_pGL3dView->m_bSurfaces);
		pSettings->setValue("bOutline", m_pGL3dView->m_bOutline);
		pSettings->setValue("bVLMPanels", m_pGL3dView->m_bVLMPanels);
		pSettings->setValue("bAxes", m_pGL3dView->m_bAxes);
		pSettings->setValue("b3DCp", m_b3DCp);
		pSettings->setValue("bDownwash", m_bDownwash);
		pSettings->setValue("bMoments", m_bMoments);
		pSettings->setValue("bAutoCpScale", s_bAutoCpScale);
		pSettings->setValue("bShowCpScale", m_bShowCpScale);
		pSettings->setValue("CurWOppOnly", m_bCurPOppOnly);
		pSettings->setValue("bShowElliptic", m_bShowEllipticCurve);
		pSettings->setValue("bShowTargetCurve", m_bShowBellCurve);
		pSettings->setValue("BellCurveExp",m_BellCurveExp);
		pSettings->setValue("CurveMaxCL",m_bMaxCL);
		pSettings->setValue("LogFile", m_bLogFile);
		pSettings->setValue("bVLM1", WPolarDlg::s_WPolar.bVLM1());
		pSettings->setValue("Dirichlet", m_bDirichlet);
		pSettings->setValue("KeepOutOpps", PlaneOpp::s_bKeepOutOpps);
		pSettings->setValue("ResetWake", m_bResetWake );
		pSettings->setValue("ShowWing", m_bShowWingCurve[0]);
		pSettings->setValue("ShowWing2", m_bShowWingCurve[1]);
		pSettings->setValue("ShowStab", m_bShowWingCurve[2]);
		pSettings->setValue("ShowFin", m_bShowWingCurve[3]);
		pSettings->setValue("StoreWOpp", PlaneOpp::s_bStoreOpps);
		pSettings->setValue("Sequence", m_bSequence );
		pSettings->setValue("AlphaMin", m_AlphaMin);
		pSettings->setValue("AlphaMax", m_AlphaMax);
		pSettings->setValue("AlphaDelta", m_AlphaDelta);
		pSettings->setValue("BetaMin", m_BetaMin);
		pSettings->setValue("BetaMax", m_BetaMax);
		pSettings->setValue("BetaDelta", m_BetaDelta);
		pSettings->setValue("QInfMin", m_QInfMin );
		pSettings->setValue("QInfMax", m_QInfMax );
		pSettings->setValue("QInfDelta", m_QInfDelta );
		pSettings->setValue("ControlMin", m_ControlMin);
		pSettings->setValue("ControlMax", m_ControlMax);
		pSettings->setValue("ControlDelta", m_ControlDelta);
		pSettings->setValue("bAutoInertia", WPolarDlg::s_WPolar.m_bAutoInertia);
		pSettings->setValue("showFlapMoments", m_bShowFlapMoments);

		pSettings->setValue("CpStyle", m_CpLineStyle.m_Style);
		pSettings->setValue("CpWidth", m_CpLineStyle.m_Width);
		pSettings->setValue("CpColor", m_CpLineStyle.m_Color);
		pSettings->setValue("CpPointStyle", m_CpLineStyle.m_PointStyle);

		pSettings->setValue("CvPrec", LLTAnalysis::s_CvPrec);
		pSettings->setValue("RelaxMax", LLTAnalysis::s_RelaxMax);
		pSettings->setValue("NLLTStations", LLTAnalysis::s_NLLTStations);

		pSettings->setValue("Trefftz", PanelAnalysis::s_bTrefftz);


		switch(m_iView)
		{
			case XFLR5::WOPPVIEW:
			{
				pSettings->setValue("iView", 0);
				break;
			}
			case XFLR5::WPOLARVIEW:
			{
				pSettings->setValue("iView", 1);
				break;
			}
			case XFLR5::W3DVIEW:
			{
				pSettings->setValue("iView", 2);
				break;
			}
			case XFLR5::WCPVIEW:
			{
				pSettings->setValue("iView", 3);
				break;
			}
			case XFLR5::STABTIMEVIEW:
			{
				pSettings->setValue("iView", 4);
				break;
			}
			case XFLR5::STABPOLARVIEW:
			{
				pSettings->setValue("iView", 5);
				break;
			}
		case XFLR5::OTHERVIEW:
				break;
		}

		switch(m_iWingView)
		{
			case XFLR5::ONEGRAPH:
				pSettings->setValue("iWingView", 1);
				break;
			case XFLR5::TWOGRAPHS:
				pSettings->setValue("iWingView", 2);
				break;
			case XFLR5::FOURGRAPHS:
				pSettings->setValue("iWingView", 4);
				break;
			default:
				pSettings->setValue("iWingView", 0);
				break;
		}


		switch(m_iWPlrView)
		{
			case XFLR5::ONEGRAPH:
				pSettings->setValue("iWPlrView", 1);
				break;
			case XFLR5::TWOGRAPHS:
				pSettings->setValue("iWPlrView", 2);
				break;
			case XFLR5::FOURGRAPHS:
				pSettings->setValue("iWPlrView", 4);
				break;
			default:
				pSettings->setValue("iWPlrView", 0);
				break;
		}


		switch(m_iRootLocusView)
		{
			case XFLR5::ONEGRAPH:
				pSettings->setValue("iRootLocusView", 1);
				break;
			case XFLR5::TWOGRAPHS:
				pSettings->setValue("iRootLocusView", 2);
				break;
			case XFLR5::FOURGRAPHS:
				pSettings->setValue("iRootLocusView", 4);
				break;
			default:
				pSettings->setValue("iRootLocusView", 0);
				break;
		}
		switch(m_iStabTimeView)
		{
			case XFLR5::ONEGRAPH:
				pSettings->setValue("iStabTimeView", 1);
				break;
			case XFLR5::TWOGRAPHS:
				pSettings->setValue("iStabTimeView", 2);
				break;
			case XFLR5::FOURGRAPHS:
				pSettings->setValue("iStabTimeView", 5);
				break;
			default:
				pSettings->setValue("iStabTimeView", 0);
				break;
		}

		pSettings->setValue("Iter", m_LLTMaxIterations);
		pSettings->setValue("InducedDragPoint", m_InducedDragPoint);
//		pSettings->setValue("NHoopPoints", GL3dBodyDlg::s_NHoopPoints);
//		pSettings->setValue("NXPoints", GL3dBodyDlg::s_NXPoints);

		pSettings->setValue("LiftScale", s_LiftScale);
		pSettings->setValue("DragScale", s_DragScale);
		pSettings->setValue("VelocityScale", s_VelocityScale);

		pSettings->setValue("WakeInterNodes", m_WakeInterNodes);
		pSettings->setValue("CtrlPos",   Panel::s_CtrlPos);
		pSettings->setValue("VortexPos", Panel::s_VortexPos);
		pSettings->setValue("CoreSize", Panel::s_CoreSize);
		pSettings->setValue("MinPanelSize", Wing::s_MinPanelSize);
		pSettings->setValue("TotalTime", m_TotalTime);
		pSettings->setValue("Delta_t", m_Deltat);
		pSettings->setValue("RampTime", m_RampTime);
		pSettings->setValue("RampAmplitude", m_RampAmplitude);
		pSettings->setValue("TimeIn0", m_TimeInput[0]);
		pSettings->setValue("TimeIn1", m_TimeInput[1]);
		pSettings->setValue("TimeIn2", m_TimeInput[2]);
		pSettings->setValue("TimeIn3", m_TimeInput[3]);
		pSettings->setValue("DynamicsMode", m_bLongitudinal);
		pSettings->setValue("StabCurveType",m_StabilityResponseType);

//		pSettings->setValue("AVLControls", StabPolarDlg::s_StabPolar.m_bAVLControls);

		pStabView->readControlModelData();
		for(int i=0; i<20; i++)
		{
			strong = QString("ForcedTime%1").arg(i);
			pSettings->setValue(strong, pStabView->m_Time[i]);
		}
		for(int i=0; i<20; i++)
		{
			strong = QString("ForcedAmplitude%1").arg(i);
			pSettings->setValue(strong, pStabView->m_Amplitude[i]);
		}

		pSettings->setValue("StabPolarAutoInertia", StabPolarDlg::s_StabWPolar.m_bAutoInertia);
		pSettings->setValue("StabPolarMass",   StabPolarDlg::s_StabWPolar.mass());
		pSettings->setValue("StabPolarCoGx",   StabPolarDlg::s_StabWPolar.CoG().x);
		pSettings->setValue("StabPolarCoGy",   StabPolarDlg::s_StabWPolar.CoG().y);
		pSettings->setValue("StabPolarCoGz",   StabPolarDlg::s_StabWPolar.CoG().z);
		pSettings->setValue("StabPolarCoGIxx", StabPolarDlg::s_StabWPolar.m_CoGIxx);
		pSettings->setValue("StabPolarCoGIyy", StabPolarDlg::s_StabWPolar.m_CoGIyy);
		pSettings->setValue("StabPolarCoGIzz", StabPolarDlg::s_StabWPolar.m_CoGIzz);
		pSettings->setValue("StabPolarCoGIxz", StabPolarDlg::s_StabWPolar.m_CoGIxz);

		pSettings->setValue("Temperature", AeroDataDlg::s_Temperature);
		pSettings->setValue("Altitude", AeroDataDlg::s_Altitude);
	}
	pSettings->endGroup();

	m_CpGraph.saveSettings(pSettings);
	m_StabPlrGraph.at(0)->saveSettings(pSettings);
	m_StabPlrGraph.at(1)->saveSettings(pSettings);

	for(int ig=0; ig<m_WPlrGraph.count(); ig++) m_WPlrGraph[ig]->saveSettings(pSettings);
	for(int ig=0; ig<m_WingGraph.count(); ig++) m_WingGraph[ig]->saveSettings(pSettings);
	for(int ig=0; ig<m_TimeGraph.count(); ig++) m_TimeGraph[ig]->saveSettings(pSettings);

	GLLightDlg::saveSettings(pSettings);
	GL3dBodyDlg::saveSettings(pSettings);
	EditPlaneDlg::saveSettings(pSettings);
	EditBodyDlg::saveSettings(pSettings);

	return true;
}



/**
 * Sets the scale for the 3d view
 */
void QMiarex::setScale()
{
	if(/*m_iView==XFLR5::W3DVIEW && */m_pCurPlane && W3dPrefsDlg::s_bAutoAdjustScale)
	{
		double bodyLength = 0.0;
		if(m_pCurPlane->body()) bodyLength = m_pCurPlane->body()->length();
		m_pGL3dView->set3DScale(std::max(m_pCurPlane->span(), bodyLength));
	}
}


/**
 * Initializes the input parameters depending on the type of the active polar
 */
void QMiarex::setAnalysisParams()
{
	m_pctrlSequence->setChecked(m_bSequence);

	m_pctrlAlphaMax->setEnabled(m_bSequence);
	m_pctrlAlphaDelta->setEnabled(m_bSequence);

	if (!m_pCurWPolar)
	{
		m_pctrlSequence->setEnabled(false);
		m_pctrlAlphaMin->setEnabled(false);
		m_pctrlAlphaMax->setEnabled(false);
		m_pctrlAlphaDelta->setEnabled(false);
		m_pctrlInitLLTCalc->setEnabled(false);
		m_pctrlStoreWOpp->setEnabled(false);
		return;
	}
	else
	{
		m_pctrlSequence->setEnabled(true);
		m_pctrlAlphaMin->setEnabled(true);

		m_pctrlAlphaMax->setEnabled(m_bSequence);
		m_pctrlAlphaDelta->setEnabled(m_bSequence);

		m_pctrlInitLLTCalc->setEnabled(true);
		m_pctrlStoreWOpp->setEnabled(true);
	}

	m_pctrlInitLLTCalc->setChecked(m_bInitLLTCalc);
	m_pctrlStoreWOpp->setChecked(PlaneOpp::s_bStoreOpps);

	if (!m_pCurWPolar || (m_pCurWPolar && m_pCurWPolar->polarType() < XFLR5::FIXEDAOAPOLAR))
	{
		m_pctrlAlphaMin->setValue(m_AlphaMin);
		m_pctrlAlphaMax->setValue(m_AlphaMax);
		m_pctrlAlphaDelta->setValue(m_AlphaDelta);
	}
	else if(m_pCurWPolar  && m_pCurWPolar->polarType() == XFLR5::FIXEDAOAPOLAR)
	{
		m_pctrlAlphaMin->setValue(m_QInfMin*Units::mstoUnit());
		m_pctrlAlphaMax->setValue(m_QInfMax*Units::mstoUnit());
		m_pctrlAlphaDelta->setValue(m_QInfDelta*Units::mstoUnit());
	}
	else if (m_pCurWPolar && m_pCurWPolar->polarType() == XFLR5::BETAPOLAR)
	{
		m_pctrlAlphaMin->setValue(m_BetaMin);
		m_pctrlAlphaMax->setValue(m_BetaMax);
		m_pctrlAlphaDelta->setValue(m_BetaDelta);
	}
	else if (m_pCurWPolar && (m_pCurWPolar->polarType() == XFLR5::STABILITYPOLAR))
	{
		m_pctrlAlphaMin->setValue(m_ControlMin);
		m_pctrlAlphaMax->setValue(m_ControlMax);
		m_pctrlAlphaDelta->setValue(m_ControlDelta);
	}
}


/**
 * Initializes the input parameters in the style dialog boxes depending on the type of view and on the active polar or operating point
 */
void QMiarex::setCurveParams()
{
	if(m_iView==XFLR5::WPOLARVIEW || m_iView==XFLR5::STABPOLARVIEW)
	{
		if(m_pCurWPolar)
		{
			m_pctrlShowCurve->setChecked(m_pCurWPolar->isVisible());
			m_bCurveVisible     = m_pCurWPolar->isVisible();
			m_LineStyle.m_Color = m_pCurWPolar->curveColor();
			m_LineStyle.m_Style = m_pCurWPolar->curveStyle();
			m_LineStyle.m_Width = m_pCurWPolar->curveWidth();
			m_LineStyle.m_PointStyle = m_pCurWPolar->points();
			fillComboBoxes();
		}
		else
		{
			fillComboBoxes(false);
		}
	}
	else if(m_iView==XFLR5::STABTIMEVIEW)
	{
		StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
		if(pStabView->m_pCurve)
		{
			m_bCurveVisible     = pStabView->m_pCurve->isVisible();
			m_LineStyle.m_Color = pStabView->m_pCurve->color();
			m_LineStyle.m_Style = pStabView->m_pCurve->style();
			m_LineStyle.m_Width = pStabView->m_pCurve->width();
			m_LineStyle.m_PointStyle = pStabView->m_pCurve->pointStyle();
			m_pctrlShowCurve->setChecked(pStabView->m_pCurve->isVisible());
			fillComboBoxes();
		}
	}
	else if(m_iView==XFLR5::WOPPVIEW)
	{
		//set OpPoint params
		if(m_pCurPOpp)
		{
			m_pctrlShowCurve->setChecked(m_pCurPOpp->isVisible());

			m_bCurveVisible     = m_pCurPOpp->isVisible();
			m_LineStyle.m_Color = m_pCurPOpp->color();
			m_LineStyle.m_Style = m_pCurPOpp->style();
			m_LineStyle.m_Width = m_pCurPOpp->width();
			m_LineStyle.m_PointStyle = m_pCurPOpp->points();
			fillComboBoxes();
		}
		else
		{
			fillComboBoxes(false);
		}
	}
	else if(m_iView==XFLR5::WCPVIEW)
	{
		//set Cp params
		if(m_pCurPOpp)
		{
			m_pctrlShowCurve->setChecked(true);
			m_bCurveVisible = true;
			m_LineStyle = m_CpLineStyle;
			fillComboBoxes();
		}
		else
		{
			fillComboBoxes(false);
		}
	}
	else
	{
		fillComboBoxes(false);
	}


	if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::FIXEDAOAPOLAR)
	{
		QString str;
		Units::getSpeedUnitLabel(str);
		m_pctrlUnit1->setText(str);
		m_pctrlUnit2->setText(str);
		m_pctrlUnit3->setText(str);

		m_pctrlParameterName->setText("Freestream velocity");
		QFont fontSymbol(Settings::s_TextFont);
		fontSymbol.setBold(true);
		fontSymbol.setPointSize(Settings::s_TextFont.pointSize()+2);
		m_pctrlParameterName->setFont(fontSymbol);
	}
	else if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
	{
		m_pctrlUnit1->setText("");
		m_pctrlUnit2->setText("");
		m_pctrlUnit3->setText("");

		m_pctrlParameterName->setText("Control parameter");
		QFont fontSymbol(Settings::s_TextFont);
		fontSymbol.setBold(true);
		m_pctrlParameterName->setFont(fontSymbol);
	}
	else if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::BETAPOLAR)
	{
		m_pctrlUnit1->setText(QString::fromUtf8("°"));
		m_pctrlUnit2->setText(QString::fromUtf8("°"));
		m_pctrlUnit3->setText(QString::fromUtf8("°"));

		m_pctrlParameterName->setText("b");
		QFont fontSymbol("Symbol");
		fontSymbol.setBold(true);
		m_pctrlParameterName->setFont(fontSymbol);
	}
	else
	{
		m_pctrlUnit1->setText(QString::fromUtf8("°"));
		m_pctrlUnit2->setText(QString::fromUtf8("°"));
		m_pctrlUnit3->setText(QString::fromUtf8("°"));

		m_pctrlParameterName->setText("a");
		QFont fontSymbol("Symbol");
		fontSymbol.setBold(true);
		m_pctrlParameterName->setFont(fontSymbol);
	}
}



/**
 * Sets the active plane
 * Sets an active polar and operating point for this plane, if any are available
 * @param PlaneName the name of the plane to be set as active
 */
void QMiarex::setPlane(QString PlaneName)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_bResetTextLegend = true;

	//try the plane's name first
	Plane *pPlane = Objects3D::getPlane(PlaneName);
	if(!pPlane)
	{
		//get the first one in the list
		if(Objects3D::s_oaPlane.count())
		{
			pPlane = (Plane*)Objects3D::s_oaPlane.at(0);
		}
	}

	m_pCurPlane = m_theTask.setPlaneObject(pPlane);
	if(!m_pCurPlane)
	{
		// no plane,
		//clear the pointers
		for (int iw=0; iw<MAXWINGS; iw++)
		{
			m_pWOpp[iw]     = NULL;
		}

		//clear the GUI
		m_pCurWPolar = NULL;
		m_pCurPOpp  = NULL;
		s_bResetCurves = true;

		setCurveParams();
		s_bResetCurves = true;

		updateView();
		QApplication::restoreOverrideCursor();
		return;
	}

	// we have a plane, initialize the pointers and the  GUI
	m_bResetglGeom = true;
	m_bResetglMesh = true;
	m_bResetTextLegend = true;

	if(m_pCurPlane->body()) m_bResetglBody   = true;
	else                    m_bResetglBody   = false;

	s_pMainFrame->selectPlane(m_pCurPlane);
	s_pMainFrame->updateWPolarListBox();

	if (m_pCurWPolar)
	{
		// try to set the same as the existing polar
		setWPolar(false, m_pCurWPolar->polarName());
	}
	else
	{
		setWPolar();
	}

	s_pMainFrame->m_glLightDlg.setModelSize(m_pCurPlane->planformSpan());

//	setScale();
	setWGraphScale();

	s_bResetCurves = true;

	QApplication::restoreOverrideCursor();
}


/**
 * Constructs the layout of the QMiarex widget
 */
void QMiarex::setupLayout()
{
	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

	QSizePolicy szPolicyMinimum;
	szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
	szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

	QSizePolicy szPolicyMaximum;
	szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
	szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

	QSizePolicy szPolicyPreferred;
	szPolicyPreferred.setHorizontalPolicy(QSizePolicy::Preferred);
	szPolicyPreferred.setVerticalPolicy(QSizePolicy::Preferred);

//_______________________Analysis

	QGroupBox *pAnalysisBox = new QGroupBox(tr("Analysis settings"));
	{
		QVBoxLayout *pAnalysisGroupLayout = new QVBoxLayout;
		{
			m_pctrlSequence = new QCheckBox(tr("Sequence"));
			QGridLayout *pSequenceGroupLayout = new QGridLayout;
			{
				m_pctrlParameterName = new QLabel("Alpha");
				QLabel *AlphaMinLab   = new QLabel(tr("Start="));
				QLabel *AlphaMaxLab   = new QLabel(tr("End="));
				QLabel *AlphaDeltaLab = new QLabel(tr("D="));
				AlphaDeltaLab->setFont(QFont("Symbol"));
				AlphaDeltaLab->setAlignment(Qt::AlignRight);
				AlphaMinLab->setAlignment(Qt::AlignRight);
				AlphaMaxLab->setAlignment(Qt::AlignRight);
				m_pctrlAlphaMin     = new DoubleEdit(0.0, 3);
				m_pctrlAlphaMax     = new DoubleEdit(1., 3);
				m_pctrlAlphaDelta   = new DoubleEdit(0.5, 3);

				m_pctrlUnit1 = new QLabel(QString::fromUtf8("°"));
				m_pctrlUnit2 = new QLabel(QString::fromUtf8("°"));
				m_pctrlUnit3 = new QLabel(QString::fromUtf8("°"));

				m_pctrlAlphaMin->setAlignment(Qt::AlignRight);
				m_pctrlAlphaMax->setAlignment(Qt::AlignRight);
				m_pctrlAlphaDelta->setAlignment(Qt::AlignRight);
				pSequenceGroupLayout->addWidget(m_pctrlParameterName,1,1,1,3, Qt::AlignVCenter|Qt::AlignCenter);
				pSequenceGroupLayout->addWidget(AlphaMinLab,2,1);
				pSequenceGroupLayout->addWidget(AlphaMaxLab,3,1);
				pSequenceGroupLayout->addWidget(AlphaDeltaLab,4,1);
				pSequenceGroupLayout->addWidget(m_pctrlAlphaMin,2,2);
				pSequenceGroupLayout->addWidget(m_pctrlAlphaMax,3,2);
				pSequenceGroupLayout->addWidget(m_pctrlAlphaDelta,4,2);
				pSequenceGroupLayout->addWidget(m_pctrlUnit1,2,3);
				pSequenceGroupLayout->addWidget(m_pctrlUnit2,3,3);
				pSequenceGroupLayout->addWidget(m_pctrlUnit3,4,3);
			}
			QHBoxLayout *pAnalysisSettingsLayout = new QHBoxLayout;
			{
				m_pctrlInitLLTCalc = new QCheckBox(tr("Init LLT"));
				m_pctrlStoreWOpp    = new QCheckBox(tr("Store OpPoint"));
				pAnalysisSettingsLayout->addWidget(m_pctrlInitLLTCalc);
				pAnalysisSettingsLayout->addWidget(m_pctrlStoreWOpp);
			}

			m_pctrlAnalyze     = new QPushButton(tr("Analyze"));

			pAnalysisGroupLayout->addWidget(m_pctrlSequence);
			pAnalysisGroupLayout->addLayout(pSequenceGroupLayout);
//			pAnalysisGroupLayout->addStretch(1);
			pAnalysisGroupLayout->addLayout(pAnalysisSettingsLayout);
			pAnalysisGroupLayout->addWidget(m_pctrlAnalyze);
		}
		pAnalysisBox->setLayout(pAnalysisGroupLayout);
	}

//_______________________Display
	QGroupBox *pDisplayBox = new QGroupBox(tr("Results"));
	{
		QGridLayout *pCheckDispLayout = new QGridLayout;
		{
			m_pctrlPanelForce = new QCheckBox("F/s=q.Cp");
			m_pctrlPanelForce->setToolTip(tr("Display the force 1/2.rho.V2.S.Cp acting on the panel"));
			m_pctrlLift           = new QCheckBox(tr("Lift"));
			m_pctrlIDrag          = new QCheckBox(tr("Ind. Drag"));
			m_pctrlVDrag          = new QCheckBox(tr("Visc. Drag"));
			m_pctrlTrans          = new QCheckBox(tr("Trans."));
			m_pctrlMoment         = new QCheckBox(tr("Moment"));
			m_pctrlDownwash       = new QCheckBox(tr("Downwash"));
			m_pctrlCp             = new QCheckBox(tr("Cp"));
			m_pctrlSurfVel        = new QCheckBox(tr("Surf. Vel."));
			m_pctrlStream         = new QCheckBox(tr("Stream"));
			m_pctrlWOppAnimate    = new QCheckBox(tr("Animate"));

			m_pctrlAnimateWOppSpeed  = new QSlider(Qt::Horizontal);
			m_pctrlAnimateWOppSpeed->setSizePolicy(szPolicyMinimum);
			m_pctrlAnimateWOppSpeed->setMinimum(0);
			m_pctrlAnimateWOppSpeed->setMaximum(500);
			m_pctrlAnimateWOppSpeed->setSliderPosition(250);
			m_pctrlAnimateWOppSpeed->setTickInterval(50);
			m_pctrlAnimateWOppSpeed->setTickPosition(QSlider::TicksBelow);
			pCheckDispLayout->addWidget(m_pctrlCp,       1,1);
			pCheckDispLayout->addWidget(m_pctrlPanelForce, 1, 2);
			pCheckDispLayout->addWidget(m_pctrlLift,     2, 1);
			pCheckDispLayout->addWidget(m_pctrlMoment,   2, 2);
			pCheckDispLayout->addWidget(m_pctrlIDrag,    3, 1);
			pCheckDispLayout->addWidget(m_pctrlVDrag,    3, 2);
			pCheckDispLayout->addWidget(m_pctrlTrans,    4, 1);
			pCheckDispLayout->addWidget(m_pctrlDownwash, 4, 2);
			pCheckDispLayout->addWidget(m_pctrlSurfVel,  5, 1);
			pCheckDispLayout->addWidget(m_pctrlStream,   5, 2);
			pCheckDispLayout->addWidget(m_pctrlWOppAnimate,  6, 1);
			pCheckDispLayout->addWidget(m_pctrlAnimateWOppSpeed,6,2);
			pCheckDispLayout->setRowStretch(7,1);
		}
		pDisplayBox->setLayout(pCheckDispLayout);
	}

	QGroupBox *pPolarPropsBox = new QGroupBox(tr("Polar properties"));
	{
		m_pctrlPolarProps = new MinTextEdit(this);
		m_pctrlPolarProps->setFontFamily("Courier");
		m_pctrlPolarProps->setReadOnly(true);

		QHBoxLayout *pPolarPropsLayout = new QHBoxLayout;
		{
			pPolarPropsLayout->addWidget(m_pctrlPolarProps);
		}
		pPolarPropsBox->setLayout(pPolarPropsLayout);
	}

//_______________________Curve params
	QGroupBox *pCurveBox = new QGroupBox(tr("Curve settings"));
	{
		QVBoxLayout *pCurveGroupLayout = new QVBoxLayout;
		{
			m_pctrlShowCurve   = new QCheckBox(tr("Curve"));
			m_pctrlCurveStyle  = new LineCbBox(this);
			m_pctrlCurveWidth  = new LineCbBox(this);
			m_pctrlCurvePoints = new LineCbBox(this);
			m_pctrlCurvePoints->showPoints(true);
			m_pctrlCurveColor  = new LineBtn();

			for (int i=0; i<5; i++)
			{
				m_pctrlCurveStyle->addItem(tr("item"));
				m_pctrlCurveWidth->addItem(tr("item"));
				m_pctrlCurvePoints->addItem(tr("item"));
			}

			m_pStyleDelegate = new LineDelegate(m_pctrlCurveStyle);
			m_pWidthDelegate = new LineDelegate(m_pctrlCurveWidth);
			m_pPointDelegate = new LineDelegate(m_pctrlCurvePoints);
			m_pctrlCurveStyle->setItemDelegate(m_pStyleDelegate);
			m_pctrlCurveWidth->setItemDelegate(m_pWidthDelegate);
			m_pctrlCurvePoints->setItemDelegate(m_pPointDelegate);

			QGridLayout *pCurveStyleLayout = new QGridLayout;
			{
				QLabel *lab200 = new QLabel(tr("Style"));
				QLabel *lab201 = new QLabel(tr("Width"));
				QLabel *lab202 = new QLabel(tr("Color"));
				QLabel *lab203 = new QLabel(tr("Points"));
				lab200->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
				lab201->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
				lab202->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
				lab203->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
				pCurveStyleLayout->addWidget(lab203,1,1);
				pCurveStyleLayout->addWidget(lab200,2,1);
				pCurveStyleLayout->addWidget(lab201,3,1);
				pCurveStyleLayout->addWidget(lab202,4,1);
				pCurveStyleLayout->addWidget(m_pctrlCurvePoints,1,2);
				pCurveStyleLayout->addWidget(m_pctrlCurveStyle,2,2);
				pCurveStyleLayout->addWidget(m_pctrlCurveWidth,3,2);
				pCurveStyleLayout->addWidget(m_pctrlCurveColor,4,2);
				pCurveStyleLayout->setColumnStretch(2,5);
			}

			pCurveGroupLayout->addWidget(m_pctrlShowCurve);
			pCurveGroupLayout->addLayout(pCurveStyleLayout);
//			pCurveGroupLayout->addStretch(1);
			setCurveParams();
		}
		pCurveBox->setLayout(pCurveGroupLayout);
	}
//_______________________Cp Params
	QGroupBox *pCpBox = new QGroupBox(tr("Cp Sections"));
	{
		QVBoxLayout *pCpParams = new QVBoxLayout;
		{
			m_pctrlCpSectionSlider = new QSlider(Qt::Horizontal);
			m_pctrlCpSectionSlider->setSizePolicy(szPolicyMinimum);
			m_pctrlCpSectionSlider->setMinimum(-100);
			m_pctrlCpSectionSlider->setMaximum(100);
			m_pctrlCpSectionSlider->setSliderPosition(00);
			m_pctrlCpSectionSlider->setTickInterval(10);
			m_pctrlCpSectionSlider->setTickPosition(QSlider::TicksBelow);
			QHBoxLayout *CpPos = new QHBoxLayout;
			{
				QLabel *label1000 = new QLabel(tr("Span Position"));
				m_pctrlSpanPos = new DoubleEdit(0.0, 3);
				CpPos->addWidget(label1000);
				CpPos->addWidget(m_pctrlSpanPos);
			}
			QHBoxLayout *pCpSections = new QHBoxLayout;
			{
				m_pctrlKeepCpSection  = new QPushButton(tr("Keep"));
				m_pctrlResetCpSection = new QPushButton(tr("Reset"));
				pCpSections->addWidget(m_pctrlKeepCpSection);
				pCpSections->addWidget(m_pctrlResetCpSection);
			}
			pCpParams->addWidget(m_pctrlCpSectionSlider);
			pCpParams->addLayout(CpPos);
			pCpParams->addLayout(pCpSections);
			pCpParams->addStretch(1);
		}
		pCpBox->setLayout(pCpParams);
	}


//_______________________3D view controls
	QGroupBox *pThreeDViewBox = new QGroupBox(tr("Display"));
	{
		QVBoxLayout *pThreeDViewControlsLayout = new QVBoxLayout;
		{
			QGridLayout *pThreeDParamsLayout = new QGridLayout;
			{
				m_pctrlAxes         = new QCheckBox(tr("Axes"), this);
//				m_pctrlLight      = new QCheckBox(tr("Light"), this);
				m_pctrlSurfaces     = new QCheckBox(tr("Surfaces"), this);
				m_pctrlOutline      = new QCheckBox(tr("Outline"), this);
				m_pctrlPanels       = new QCheckBox(tr("Panels"), this);
				m_pctrlFoilNames    = new QCheckBox(tr("Foil Names"), this);
				m_pctrlMasses       = new QCheckBox(tr("Masses"), this);

				pThreeDParamsLayout->addWidget(m_pctrlAxes, 1,1);
				pThreeDParamsLayout->addWidget(m_pctrlPanels, 1,2);
				pThreeDParamsLayout->addWidget(m_pctrlSurfaces, 2,1);
				pThreeDParamsLayout->addWidget(m_pctrlOutline, 2,2);
				pThreeDParamsLayout->addWidget(m_pctrlFoilNames, 3,1);
				pThreeDParamsLayout->addWidget(m_pctrlMasses, 3,2);
			}

			QVBoxLayout *pThreeDViewLayout = new QVBoxLayout;
			{
				QHBoxLayout *pAxisViewLayout = new QHBoxLayout;
				{
					m_pctrlX          = new QToolButton;
					m_pctrlY          = new QToolButton;
					m_pctrlZ          = new QToolButton;
					m_pctrlIso        = new QToolButton;
					m_pctrlFlip       = new QToolButton;
					if(m_pctrlX->iconSize().height()<=48)
					{
						m_pctrlX->setIconSize(QSize(24,24));
						m_pctrlY->setIconSize(QSize(24,24));
						m_pctrlZ->setIconSize(QSize(24,24));
						m_pctrlIso->setIconSize(QSize(24,24));
						m_pctrlFlip->setIconSize(QSize(24,24));
					}
					m_pXView    = new QAction(QIcon(":/images/OnXView.png"), tr("X View"), this);
					m_pYView    = new QAction(QIcon(":/images/OnYView.png"), tr("Y View"), this);
					m_pZView    = new QAction(QIcon(":/images/OnZView.png"), tr("Z View"), this);
					m_pIsoView  = new QAction(QIcon(":/images/OnIsoView.png"), tr("Iso View"), this);
					m_pFlipView = new QAction(QIcon(":/images/OnFlipView.png"), tr("Flip View"), this);
					m_pXView->setCheckable(true);
					m_pYView->setCheckable(true);
					m_pZView->setCheckable(true);
					m_pIsoView->setCheckable(true);
					m_pFlipView->setCheckable(false);

					m_pctrlX->setDefaultAction(m_pXView);
					m_pctrlY->setDefaultAction(m_pYView);
					m_pctrlZ->setDefaultAction(m_pZView);
					m_pctrlIso->setDefaultAction(m_pIsoView);
					m_pctrlFlip->setDefaultAction(m_pFlipView);
					pAxisViewLayout->addWidget(m_pctrlX);
					pAxisViewLayout->addWidget(m_pctrlY);
					pAxisViewLayout->addWidget(m_pctrlZ);
					pAxisViewLayout->addWidget(m_pctrlIso);
					pAxisViewLayout->addWidget(m_pctrlFlip);
				}

				pThreeDViewLayout->addLayout(pAxisViewLayout);
				m_pctrl3DResetScale = new QPushButton(tr("Reset scale"));
				m_pctrl3DResetScale->setStatusTip(tr("Resets the display scale so that the plane fits in the window"));
				pThreeDViewLayout->addWidget(m_pctrl3DResetScale);
			}

			QHBoxLayout *pClipLayout = new QHBoxLayout;
			{
				QLabel *ClipLabel = new QLabel(tr("Clip:"));
				m_pctrlClipPlanePos = new QSlider(Qt::Horizontal);
				m_pctrlClipPlanePos->setSizePolicy(szPolicyMinimum);
				m_pctrlClipPlanePos->setMinimum(-300);
				m_pctrlClipPlanePos->setMaximum(300);
				m_pctrlClipPlanePos->setSliderPosition(0);
				m_pctrlClipPlanePos->setTickInterval(30);
				m_pctrlClipPlanePos->setTickPosition(QSlider::TicksBelow);
				pClipLayout->addWidget(ClipLabel);
				pClipLayout->addWidget(m_pctrlClipPlanePos,1);
			}
			pThreeDViewControlsLayout->addLayout(pThreeDParamsLayout);
			pThreeDViewControlsLayout->addStretch(1);
			pThreeDViewControlsLayout->addLayout(pThreeDViewLayout);
			pThreeDViewControlsLayout->addLayout(pClipLayout);
			pThreeDViewControlsLayout->addStretch(1);

		}
		pThreeDViewBox->setLayout(pThreeDViewControlsLayout);
	}

//_________________________Main Layout
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		m_pctrlMiddleControls = new QStackedWidget;
		m_pctrlMiddleControls->addWidget(pDisplayBox);
		m_pctrlMiddleControls->addWidget(pPolarPropsBox);
		m_pctrlMiddleControls->addWidget(pCpBox);

		m_pctrlBottomControls = new QStackedWidget;
		m_pctrlBottomControls->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		pCurveBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		pThreeDViewBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

		m_pctrlBottomControls->addWidget(pCurveBox);
		m_pctrlBottomControls->addWidget(pThreeDViewBox);

		pMainLayout->addWidget(pAnalysisBox);
//		pMainLayout->addStretch();
		pMainLayout->addWidget(m_pctrlMiddleControls);
//		pMainLayout->addStretch();
		pMainLayout->addWidget(m_pctrlBottomControls);
	}
	setLayout(pMainLayout);
}



/**
 * Sets the scale of the graphs in the operating point view.
 *The scale is set i.a.w. with wing span, if any
 */
void QMiarex::setWGraphScale()
{
	if(!m_pCurPlane)
	{
		for(int ig=0; ig<MAXWINGGRAPHS; ig++)
		{
			m_WingGraph[ig]->setAuto(false);
			m_WingGraph[ig]->setXUnit(10.0);
			m_WingGraph[ig]->setXMin(-100.0);
			m_WingGraph[ig]->setXMax( 100.0);
		}
	}
	else
	{
		double halfspan = m_pCurPlane->planformSpan()/2.0;

		for(int ig=0; ig<MAXWINGGRAPHS; ig++)
		{
			m_WingGraph[ig]->setAutoX(false);
			m_WingGraph[ig]->setXMin(-halfspan*Units::mtoUnit());
			m_WingGraph[ig]->setXMax( halfspan*Units::mtoUnit());
			m_WingGraph[ig]->setAutoXUnit();
		}
	}
}

/**
 * Sets the active polar
 * Builds the array of panels depending on the polar type
 * @param bCurrent if true, the active polar is set anew
 * @param WPlrName the name of the polar to set for the active wing or plane
 */
void QMiarex::setWPolar(bool bCurrent, QString WPlrName)
{
	m_bResetTextLegend = true;

	if(!m_pCurPlane) return;

	WPolar *pWPolar = NULL;
	if(bCurrent)
	{
		//if we already know which WPolar object
		pWPolar = m_pCurWPolar;
	}

	if(!pWPolar && WPlrName.length())
	{
		//if we know its name
		pWPolar = Objects3D::getWPolar(m_pCurPlane, WPlrName);
	}

	m_pCurWPolar = pWPolar;

	if(!pWPolar)
	{
		//if we don't know anything, find the first for this plane
		for (int i=0; i<Objects3D::s_oaWPolar.size(); i++)
		{
			WPolar *pOldWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
			if (pOldWPolar->planeName() == m_pCurPlane->planeName())
			{
				m_pCurWPolar = pOldWPolar;
				break;
			}
		}
	}

	m_pCurWPolar = m_theTask.setWPolarObject(m_pCurPlane, m_pCurWPolar);

	s_pMainFrame->selectWPolar(m_pCurWPolar);

	if(!m_pCurWPolar)
	{
		m_pCurPOpp = NULL;
		setCurveParams();
		s_bResetCurves = true;
		return;
	}

/** @todo restore ?
	int i,j,k,m, NStation;
	double SpanPos;

	if(m_pCurWPolar->m_AnalysisMeth>XFLR5::LLTMETHOD)
	{
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(s_pWingList[iw])
			{
				s_pWingList[iw]->ComputeChords();

				NStation = 0;
				m=0;
				SpanPos = 0;
				for (j=0; j<s_pWingList[iw]->m_Surface.size(); j++)	NStation += s_pWingList[iw]->m_Surface.at(j)->m_NYPanels;

				for (j=(int)(s_pWingList[iw]->m_Surface.size()/2); j<s_pWingList[iw]->m_Surface.size(); j++)
				{
					for(k=0; k<s_pWingList[iw]->m_Surface.at(j)->m_NYPanels; k++)
					{
						s_pWingList[iw]->m_SpanPos[m+NStation/2] = SpanPos + s_pWingList[iw]->m_Surface.at(j)->GetStripSpanPos(k);
						m++;
					}
					SpanPos += s_pWingList[iw]->m_Surface.at(j)->m_Length;
				}

				for(m=0; m<NStation/2; m++) s_pWingList[iw]->m_SpanPos[m] = -s_pWingList[iw]->m_SpanPos[NStation-m-1];
			}
		}
	}*/

	s_pMainFrame->updatePOppListBox();

	double x = 0.0;
	if(m_pCurPOpp)
	{
		if(m_pCurWPolar && !m_pCurWPolar->isBetaPolar())
			x = m_pCurPOpp->alpha();
		else if(m_pCurWPolar && m_pCurWPolar->isBetaPolar())
			x = m_pCurPOpp->m_Beta;
	}

	setPlaneOpp(false, x);


	if(m_pCurPlane && m_pCurWPolar)
	{
		m_bCurveVisible = m_pCurWPolar->isVisible();
		m_LineStyle.m_PointStyle  = m_pCurWPolar->points();

		//make sure the polar is up to date with the latest plane data
		if(m_pCurWPolar->bAutoInertia())
		{
			if(m_pCurPlane)
			{
				m_pCurWPolar->mass()   = m_pCurPlane->totalMass();
				m_pCurWPolar->CoG()    = m_pCurPlane->CoG();
				m_pCurWPolar->CoGIxx() = m_pCurPlane->CoGIxx();
				m_pCurWPolar->CoGIyy() = m_pCurPlane->CoGIyy();
				m_pCurWPolar->CoGIzz() = m_pCurPlane->CoGIzz();
				m_pCurWPolar->CoGIxz() = m_pCurPlane->CoGIxz();
			}
		}

		if(m_pCurWPolar->referenceDim()!=XFLR5::MANUALREFDIM)
		{
			// get the latest dimensions from the plane definition
			// should have been updated at the time when the plane was created or edited
			// just a safety precaution
			if(m_pCurWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)
			{
				m_pCurWPolar->referenceArea()       = m_pCurPlane->planformArea();
				m_pCurWPolar->referenceSpanLength() = m_pCurPlane->planformSpan();
			}
			else if(m_pCurWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)
			{
				m_pCurWPolar->referenceArea()       = m_pCurPlane->projectedArea();
				m_pCurWPolar->referenceSpanLength() = m_pCurPlane->projectedSpan();
			}
		}


		QString PolarProps;
		getPolarProperties(m_pCurWPolar, PolarProps);
		m_pctrlPolarProps->setText(PolarProps);
	}
	else m_pctrlPolarProps->clear();


	setAnalysisParams();
	setCurveParams();

	m_bResetglLegend = true;
	s_bResetCurves = true;


	if(m_pCurWPolar && m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
	{
		StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
		pStabView->setControls();
		pStabView->setMode();
		s_pMainFrame->m_pctrlStabViewWidget->show();
		pStabView->show();
	}

	QApplication::restoreOverrideCursor();
}



/**
 * Sets the y-axis titles of the stability time response graphs, depending on the selected units
 */
void QMiarex::setStabGraphTitles()
{
	QString strLength;
	Units::getSpeedUnitLabel(strLength);

	if(m_bLongitudinal)
	{
		m_TimeGraph[0]->setYTitle("u ("+strLength+")");
		m_TimeGraph[1]->setYTitle("w ("+strLength+")");
		m_TimeGraph[2]->setYTitle("q ("+QString::fromUtf8("°") +"/s)");
		m_TimeGraph[3]->setYTitle("theta "+QString::fromUtf8("(°)"));
	}
	else
	{
		m_TimeGraph[0]->setYTitle("v ("+strLength+")");
		m_TimeGraph[1]->setYTitle("p ("+QString::fromUtf8("°") +"/s)");
		m_TimeGraph[2]->setYTitle("r ("+QString::fromUtf8("°") +"/s)");
		m_TimeGraph[3]->setYTitle("phi "+QString::fromUtf8("(°)"));
	}
}



/**
 * Sets the x and y axis titles of the polar graphs
 */
void QMiarex::setWGraphTitles(Graph* pGraph)
{
	QString Title;

	Title  = QMiarex::WPolarVariableName(pGraph->xVariable());
	pGraph->setXTitle(Title);

	Title  = QMiarex::WPolarVariableName(pGraph->yVariable());
	pGraph->setYTitle(Title);
}


/**
 * Overrides the parent's widget showEvent method
 * Fills the main dialog box with default or selected data
 * @param event unused
 */
void QMiarex::showEvent(QShowEvent *event)
{
	setAnalysisParams();
	setCurveParams();
	event->accept();

//	QWidget *pWidget = new QWidget;
//	pWidget->show();
}



/**
 * Captures the pixels of the client area and writes them to a file/
 * @deprecated QGLWidget::grabFrameBuffer() is used instead.
 * @param FileName the name of the destination image file.
 */
void QMiarex::snapClient(QString const &FileName)
{
	int NbBytes, bitsPerPixel;
	QSize size(m_pGL3dView->rect().width(),m_pGL3dView->rect().height());

	bitsPerPixel = 24;
	int width = size.width();
	switch(bitsPerPixel)
	{
		case 8:
		{
			QMessageBox::warning(s_pMainFrame,tr("Warning"),tr("Cannot (yet ?) save 8 bit depth opengl screen images... Sorry"));
			return;
		}
		case 16:
		{
			QMessageBox::warning(s_pMainFrame,tr("Warning"),tr("Cannot (yet ?) save 16 bit depth opengl screen images... Sorry"));
			size.setWidth(width - size.width() % 2);
			return;
		}
		case 24:
		{
			NbBytes = 4 * size.width() * size.height();//24 bits type BMP
//			size.setWidth(width - size.width() % 4);
			break;
		}
		case 32:
		{
			NbBytes = 4 * size.width() * size.height();//32 bits type BMP
			break;
		}
		default:
		{
			QMessageBox::warning(s_pMainFrame,tr("Warning"),tr("Unidentified bit depth... Sorry"));
			return;
		}
	}
	uchar *pPixelData = new uchar[NbBytes];

	// Copy from OpenGL
	glReadBuffer(GL_FRONT);
	switch(bitsPerPixel)
	{
		case 8: return;
		case 16: return;
		case 24:
		{
#if QT_VERSION >= 0x040400
			  glReadPixels(0,0,size.width(),size.height(),GL_RGB,GL_UNSIGNED_BYTE,pPixelData);
			  QImage Image(pPixelData, size.width(),size.height(), QImage::Format_RGB888);
			  QImage FlippedImaged;
			  FlippedImaged = Image.mirrored();	//flip vertically
			  FlippedImaged.save(FileName);
#else
			  QMessageBox::warning(s_pMainFrame,tr("Warning"),"The version of Qt used to compile the code is older than 4.4 and does not support 24 bit images... Sorry");
#endif
			  break;
		}
		case 32:
		{
			glReadPixels(0,0,size.width(),size.height(),GL_RGBA,GL_UNSIGNED_BYTE,pPixelData);
			QImage Image(pPixelData, size.width(),size.height(), QImage::Format_ARGB32);
			QImage FlippedImaged;
			FlippedImaged = Image.mirrored();	//flip vertically
			FlippedImaged.save(FileName);

			break;
		}
		default: break;
	}
}




/**
 * Cancels the animation whatever the active view
 */
void QMiarex::stopAnimate()
{
	m_bAnimateWOpp = false;
	m_pctrlWOppAnimate->setChecked(false);
	m_pTimerWOpp->stop();
	m_pTimerMode->stop();

	StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
	m_bAnimateMode = false;

	pStabView->m_pctrlAnimate->setChecked(false);

	if(m_pCurPlane)
	{
		setPlaneOpp(true);
		s_pMainFrame->selectPlaneOpp(m_pCurPOpp);
	}
}


/**
 * Updates the active curve with the data from the currently active operating points or polars.
 */
void QMiarex::updateCurve()
{
	bool bCurveVisible = m_pctrlShowCurve->isChecked();
	if(m_iView==XFLR5::WPOLARVIEW && m_pCurWPolar)
	{
		m_pCurWPolar->curveStyle()  = m_LineStyle.m_Style;
		m_pCurWPolar->curveWidth()  = m_LineStyle.m_Width;
		m_pCurWPolar->curveColor()  = m_LineStyle.m_Color;
		m_pCurWPolar->points()      = m_LineStyle.m_PointStyle;
		m_pCurWPolar->isVisible()   = bCurveVisible;
	}
	else if(m_iView==XFLR5::STABTIMEVIEW && m_pCurWPolar)
	{
		StabViewDlg *pStabView = (StabViewDlg*)s_pMainFrame->m_pStabView;
		pStabView->setTimeCurveStyle(m_LineStyle.m_Color, m_LineStyle.m_Style, m_LineStyle.m_Width, bCurveVisible, m_LineStyle.m_PointStyle);
	}
	else if(m_iView==XFLR5::STABPOLARVIEW)
	{
		m_pCurWPolar->curveStyle()  = m_LineStyle.m_Style;
		m_pCurWPolar->curveWidth()  = m_LineStyle.m_Width;
		m_pCurWPolar->curveColor()  = m_LineStyle.m_Color;
		m_pCurWPolar->points()      = m_LineStyle.m_PointStyle;
		m_pCurWPolar->isVisible() = m_pctrlShowCurve->isChecked();
	}
	else if (m_iView==XFLR5::WOPPVIEW)
	{
		if(m_pCurPOpp)
		{
			m_pCurPOpp->style()  = m_LineStyle.m_Style;
			m_pCurPOpp->width()  = m_LineStyle.m_Width;
			m_pCurPOpp->color()  = m_LineStyle.m_Color;
			m_pCurPOpp->points()      = m_LineStyle.m_PointStyle;
			m_pCurPOpp->isVisible() = bCurveVisible;
		}
	}
	else if (m_iView==XFLR5::WCPVIEW && m_pCurPOpp)
	{
		m_CpLineStyle  = m_LineStyle;
		m_bShowCp  = bCurveVisible;
	}

	s_bResetCurves = true;
	updateView();
	emit projectModified();
}


/**
 * Updates the graphs and the views after a change of units
*/
void QMiarex::updateUnits()
{
	if(m_iView==XFLR5::WPOLARVIEW)
	{
		for(int ig=0; ig<m_WPlrGraph.count(); ig++)
			setWGraphTitles(m_WPlrGraph[ig]);
	}
	else if(m_iView==XFLR5::STABTIMEVIEW || m_iView==XFLR5::STABPOLARVIEW)
	{
		setStabGraphTitles();
	}
	else
	{
		if(!m_pCurPlane) return;
		if (m_iView==XFLR5::WOPPVIEW)
		{
			onAdjustToWing();
		}
		else if(m_iView==XFLR5::WCPVIEW) createCpCurves();
		else if(m_iView==XFLR5::W3DVIEW) m_bResetglLegend = true;
		else if(m_iView==XFLR5::STABTIMEVIEW || m_iView==XFLR5::STABPOLARVIEW) m_bResetglLegend = true;
	}
	setCurveParams();
	s_bResetCurves = true;
	m_bResetTextLegend = true;
	updateView();
}


/**
 * Dispatches the drawing request depending on the type of the active view
 */
void QMiarex::updateView()
{
	if(m_iView==XFLR5::W3DVIEW)
	{
		m_pGL3dView->update();
	}
	else
	{
		if(s_bResetCurves)
		{
			if (m_iView==XFLR5::WPOLARVIEW)
			{
				createWPolarCurves();
			}
			else if (m_iView==XFLR5::WOPPVIEW)
			{
				createWOppCurves();
			}
			else if (m_iView==XFLR5::WCPVIEW)
			{
				createCpCurves();
			}
			else if(m_iView==XFLR5::STABTIMEVIEW)
			{
				if(m_StabilityResponseType==1)  createStabRungeKuttaCurves();
				else                            createStabTimeCurves();
			}
			else if(m_iView==XFLR5::STABPOLARVIEW)
			{
				createStabRLCurves();
			}
		}
		s_pMainFrame->m_pMiarexTileWidget->update();

	}
}


void QMiarex::setView(XFLR5::enumGraphView eView)
{
	switch (m_iView)
	{
		case XFLR5::WOPPVIEW:
		{
			m_iWingView = eView;
			break;
		}
		case XFLR5::WPOLARVIEW:
			m_iWPlrView = eView;
			break;
		case XFLR5::STABPOLARVIEW:
		{
			m_iRootLocusView = eView;
			break;
		}
		case XFLR5::STABTIMEVIEW:
		{
			m_iStabTimeView = eView;
			break;
		}
		default:
		{
			break;
		}
	}
}


/**
 * The user has requested a detailed display of the data of the active polar
 */
void QMiarex::onWPolarProperties()
{
	if(!m_pCurWPolar) return;
	ObjectPropsDlg opDlg(s_pMainFrame);
	QString strangeProps;
	getPolarProperties(m_pCurWPolar, strangeProps);
	opDlg.initDialog(tr("Polar properties"), strangeProps);
	opDlg.exec();
}


/**
 * The user has requested a detailed display of the data of the active operating point
 */
void QMiarex::onPlaneOppProperties()
{
	if(!m_pCurPOpp) return;
	ObjectPropsDlg opDlg(s_pMainFrame);
	QString strangeProps;
	m_pCurPOpp->getPlaneOppProperties(strangeProps, Units::lengthUnitLabel(), Units::weightUnitLabel(), Units::speedUnitLabel(),
									  Units::mtoUnit(), Units::kgtoUnit(), Units::mstoUnit());
	opDlg.initDialog(tr("Operating point Properties"), strangeProps);
	opDlg.exec();
}


/**
 * Paints and overlays the labels associated to the Cp color scale in the 3D view
 * @param painter the painter associated to the 3D widget
 */
void QMiarex::paintCpLegendText(QPainter &painter)
{
	if (!m_b3DCp || !m_pCurPOpp || m_pCurPOpp->analysisMethod()<XFLR5::VLMMETHOD) return;

	int i;
	QString strong;

	double labellength;

	double f;
	double range, delta;
	float ratio = 1.0;

	painter.save();

	QFont font(Settings::s_TextFont);
	ratio = m_pGL3dView->devicePixelRatio();
	font.setPointSize(Settings::s_TextFont.pointSize()*ratio);
	painter.setFont(font);
	painter.setRenderHint(QPainter::Antialiasing);

	QFontMetrics fm(font);
	int back = fm.averageCharWidth() * 5;

	double h = m_pGL3dView->rect().height()*ratio;
	double y0 = 2.*h/5.0;


	int ixPos, iyPos, dy;

	ixPos  = m_pGL3dView->rect().width()*ratio-back;

	dy     = (int) (h/MAXCPCOLORS/2);
	iyPos  = (int) (y0 - 12.0*dy);

	range = s_LegendMax - s_LegendMin;
	delta = range / 20;

	QPen textPen(Settings::s_TextColor);
	painter.setPen(textPen);
	painter.setRenderHint(QPainter::Antialiasing);

	strong = "Cp";
	labellength = fm.width(strong)+5;
	painter.drawText(ixPos-labellength, iyPos-dy, strong);

	for (i=0; i<=20; i ++)
	{
		f = s_LegendMax - (double)i * delta;
		strong = QString("%1").arg(f, 5,'f',2);
		labellength = (fm.width(strong)+5);
		painter.drawText(ixPos-labellength, iyPos+i*dy, strong);
	}

	painter.restore();
}



/**
 * Paints the labels associated to the Panel forces color scale in the 3D view
 * @param painter the painter to write on
 */
void QMiarex::paintPanelForceLegendText(QPainter &painter)
{
	if(!m_pCurWPolar || !m_pCurPOpp) return;
	if(!m_bPanelForce || m_pCurPOpp->analysisMethod()<XFLR5::VLMMETHOD) return;

	QString strPressure, strong;
	int p, i;
	int labellength;
	double f;
	double rmin, rmax, range, delta;
	float ratio = 1.0;

	painter.save();
	QFont font(Settings::s_TextFont);
	ratio = m_pGL3dView->devicePixelRatio();
	font.setPointSize(Settings::s_TextFont.pointSize()*ratio);
	painter.setFont(font);
	painter.setRenderHint(QPainter::Antialiasing);
	QPen textPen(Settings::s_TextColor);
	painter.setPen(textPen);

	WingOpp *pWOppList[MAXWINGS];
	for(int ip=0; ip<MAXWINGS; ip++)
	{
		pWOppList[ip] = m_pWOpp[ip];
	}


	//define the range of values to set the colors in accordance
	rmin = 1.e10;
	rmax = -rmin;
	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pWing(iw))
		{
			for (p=0; p<pWing(iw)->m_MatSize; p++)
			{
				rmax = qMax(rmax, pWOppList[iw]->m_dCp[p]*pWing(iw)->m_pWingPanel[p].area());
				rmin = qMin(rmin, pWOppList[iw]->m_dCp[p]*pWing(iw)->m_pWingPanel[p].area());
			}
		}
	}
	rmin *= 0.5*m_pCurWPolar->density() *pWOppList[0]->m_QInf*pWOppList[0]->m_QInf  *Units::PatoUnit();
	rmax *= 0.5*m_pCurWPolar->density() *pWOppList[0]->m_QInf*pWOppList[0]->m_QInf  *Units::PatoUnit();
	range = rmax - rmin;


	QFontMetrics fm(font);
	int back = fm.averageCharWidth() * 5;

	double h = (double)m_pGL3dView->rect().height()*ratio;
	double y0 = 2.*h/5.0;


	int ixPos, iyPos, dy;

	ixPos  = m_pGL3dView->rect().width()*ratio-back;

	dy     = (int) (h/40.0);
	iyPos  = (int) (y0 - 12.0*dy);

	delta = range / 20.0;

	strPressure = Units::pressureUnitLabel();
	labellength = fm.width(strPressure)+2+5;
	painter.drawText(ixPos-labellength, iyPos-dy, "("+strPressure+")");

	for (i=0; i<=20; i++)
	{
		f = rmin + (double)i * delta;
		strong.sprintf("%6.3f", f);
		labellength = (fm.width(strong)+5);
		painter.drawText(ixPos-labellength, iyPos+i*dy, strong);
	}

	painter.restore();
}





/**
* Searches for an operating point with aoa or velocity or control paramter x, for the active polar
* Sets it as active, if valid
* else sets the current PlaneOpp, if any
* else sets the comboBox's first, if any
* else sets it to NULL
*@param bCurrent, if true, uses the x value of the current operating point; this is useful if the user has changed the polar, but wants to display the same aoa for instance
*@return true if a new valid operating point has been selected
*/
bool QMiarex::setPlaneOpp(bool bCurrent, double x)
{
	if(!m_pCurPlane || !m_pCurWPolar)
	{
		m_pCurPOpp = NULL;
		s_bResetCurves = true;

		setCurveParams();
		return false;
	}

	m_bResetTextLegend = true;

	m_pCurPOpp = setPlaneOppObject(m_pCurPlane, m_pCurWPolar, m_pCurPOpp, bCurrent, x);

	if(m_pCurPOpp)
	{
		for(int iw=0; iw<MAXWINGS;iw++)
		{
			if(m_pCurPOpp->m_pPlaneWOpp[iw]) m_pWOpp[iw] = m_pCurPOpp->m_pPlaneWOpp[iw];
			else                             m_pWOpp[iw] = NULL;
		}

		//select m_pCurPOpp in the listbox
		s_pMainFrame->selectPlaneOpp(m_pCurPOpp);
	}
	else
	{
		m_pWOpp[0] = m_pWOpp[1] = m_pWOpp[2] = m_pWOpp[3] = NULL;
	}

	if(m_iView==XFLR5::STABTIMEVIEW || m_iView==XFLR5::STABPOLARVIEW)
	{
		StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
		pStabView->setControls();
		pStabView->setMode();
	}
	else if(m_iView==XFLR5::W3DVIEW)
	{
		StabViewDlg *pStabView =(StabViewDlg*)s_pMainFrame->m_pStabView;
		pStabView->setControls();
		pStabView->setMode();
	}

	m_bResetglMesh = true;
	m_bResetglOpp    = true;
	m_bResetglStream = true;
	m_bResetglSurfVelocities = true;
	m_bResetglLegend = true;

	setControls();

	s_bResetCurves = true;

	if(!m_pCurPOpp) return false;
	else if(m_iView==XFLR5::WOPPVIEW)
	{
		m_bCurveVisible = m_pCurPOpp->isVisible();
		m_LineStyle.m_PointStyle  = m_pCurPOpp->points();
	}

	return true;
}


/**
* Searches for an operating point with aoa or velocity or control paramter x, for the active polar
* Sets it as active, if valid
* else sets the current WOpp, if any
* else sets the comboBox's first, if any
* else sets it to NULL
* @param bCurrent, if true, uses the x value of the current operating point; this is useful if the user has changed the polar, but wants to display the same aoa for instance
* @return true if a new valid operating point has been selected
*/
PlaneOpp* QMiarex::setPlaneOppObject(Plane *pPlane, WPolar *pWPolar, PlaneOpp *pCurPOpp, bool bCurrent, double x)
{
	if(!pPlane || !pWPolar) return NULL;

	PlaneOpp *pPOpp = NULL;
	if(bCurrent) pPOpp = pCurPOpp;
	else         pPOpp = Objects3D::getPlaneOpp(pPlane, pWPolar, x);
	if(!pPOpp)   pPOpp = Objects3D::getPlaneOpp(pPlane, pWPolar, m_LastAlpha);

	if(!pPOpp)
	{
		//try to select the first in the object array
		for(int iPOpp=0; iPOpp<Objects3D::s_oaPOpp.size(); iPOpp++)
		{
			PlaneOpp *pOldPOpp = (PlaneOpp *)Objects3D::s_oaPOpp.at(iPOpp);
			if(pOldPOpp->planeName()==pPlane->planeName() && pOldPOpp->polarName()==pWPolar->polarName())
			{
				pPOpp = pOldPOpp;
				break;
			}
		}
	}


	if(pPOpp && pWPolar)
	{
		m_LastAlpha = pPOpp->alpha();
		m_LastBeta  = pPOpp->m_Beta;
/*		for(int iw=0; iw<MAXWINGS;iw++)
		{
			if(pCurPOpp->m_pPlaneWOpp[iw]) m_pWOpp[iw] = pCurPOpp->m_pPlaneWOpp[iw];
			else                           m_pWOpp[iw] = NULL;
		}*/

		if(pWPolar->polarType()==XFLR5::BETAPOLAR)
		{
			//set sideslip
//			Vector3d RefPoint(0.0, 0.0, 0.0);
			// Standard Convention in mechanic of flight is to have Beta>0 with nose to the left
			// The yaw moment has the opposite convention...
//			m_theTask.m_pthePanelAnalysis->rotateGeomZ(pPOpp->m_Beta, RefPoint, pWPolar->m_NXWakePanels);
		}
		else if(pWPolar->polarType()==XFLR5::STABILITYPOLAR)
		{
			//if we have a type 7 polar, set the panels in the control's position
			int nCtrls;
			QString strong;
			m_theTask.m_pthePanelAnalysis->setControlPositions(pPOpp->m_Ctrl, nCtrls, strong, false);
		}
	}

	return pPOpp;
}


/**
 * Creates the offscreen pixmap with the text legend which will be overlayed on the 3D or 2D view
 */
void QMiarex::drawTextLegend()
{
	QRect rect;
	if(m_iView==XFLR5::W3DVIEW)
	{
		QRect tempRect = m_pGL3dView->rect();
		float ratio = m_pGL3dView->devicePixelRatio();
		rect.moveTopLeft(tempRect.topLeft()*ratio);
		rect.setSize(tempRect.size()*ratio);
	}
	else if(m_iView==XFLR5::WOPPVIEW) rect = s_pMainFrame->m_pMiarexTileWidget->pWingWidget()->rect();

	if(!m_PixText.isNull())	m_PixText = m_PixText.scaled(rect.size());
	if(m_PixText.isNull()) return;

	m_PixText.fill(Qt::transparent);

	QPainter paint(&m_PixText);

	paintPlaneLegend(paint, m_pCurPlane, m_pCurWPolar, rect);
	if(m_pCurPOpp)
	{
		paintPlaneOppLegend(paint, rect);
		if(m_iView==XFLR5::W3DVIEW)
		{
			if(m_b3DCp)            paintCpLegendText(paint);
			else if(m_bPanelForce) paintPanelForceLegendText(paint);
		}
	}
	m_bResetTextLegend = false;
}






/**
 * Finds the intersection of a line defined by its origin and its direction with the current Plane object.
 * @param O the point which defines the line's origin
 * @param U the Vector which defines the line's direction
 * @param I the point of intersection
 * @return true if an intersection point has been found, false otherwise
 */
bool QMiarex::intersectObject(Vector3d O,  Vector3d U, Vector3d &I)
{
	if(!m_pCurPlane) return false;
	Wing *pWingList[MAXWINGS] = {m_pCurPlane->wing(), m_pCurPlane->wing2(), m_pCurPlane->stab(), m_pCurPlane->fin()};

	if(m_pCurPOpp)
	{
		Vector3d Origin(0.0,0.0,0.0);
		Vector3d Y(0.0,1.0,0.0);
		O.rotate(Origin, Y, -m_pCurPOpp->alpha());
		U.rotate(Y, -m_pCurPOpp->alpha());
	}

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if (pWingList[iw] && pWingList[iw]->intersectWing(O, U, I)) return true;
	}

	if(m_pCurPlane->body())
	{
		if(m_pCurPlane->body()->intersectFlatPanels(O, O+U*10, I))return true;
	}
	return false;
}



/**
 * Sets the graph tiles in accordance with the requested view
 */
void QMiarex::setGraphTiles()
{
	int maxWidgets = s_pMainFrame->m_pMiarexTileWidget->graphWidgetCount();

	switch(m_iView)
	{
		case XFLR5::WPOLARVIEW:
		{
			switch(m_iWPlrView)
			{
				case XFLR5::ONEGRAPH:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WPlrGraph, 1);
					break;
				case XFLR5::TWOGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WPlrGraph, 2);
					break;
				case XFLR5::FOURGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WPlrGraph, 4);
					break;
				case XFLR5::ALLGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WPlrGraph, MAXPOLARGRAPHS);
					break;
			}
			break;
		}

		case XFLR5::WOPPVIEW:
		{
			switch(m_iWingView)
			{
				case XFLR5::ONEGRAPH:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WingGraph, 1);
					break;
				case XFLR5::TWOGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WingGraph, 2);
					break;
				case XFLR5::FOURGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WingGraph, 4);
					break;
				case XFLR5::ALLGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_WingGraph, MAXWINGGRAPHS);
					break;
			}
			break;
		}


		case XFLR5::WCPVIEW:
		{
			QList<QGraph*> pGraphList;
			pGraphList.append(&m_CpGraph);
			s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, pGraphList, 1, 0, Qt::Vertical);
			break;
		}

		case XFLR5::STABPOLARVIEW:
		{
			if(m_iRootLocusView==XFLR5::TWOGRAPHS)
			{
				s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_StabPlrGraph, 2);
			}
			else if(m_bLongitudinal) s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_StabPlrGraph, 1, 0);
			else                     s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_StabPlrGraph, 1, 1);
			break;
		}

		case XFLR5::STABTIMEVIEW:
		{
			switch(m_iStabTimeView)
			{
				case XFLR5::ONEGRAPH:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_TimeGraph, 1);
					break;
				case XFLR5::TWOGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_TimeGraph, 2);
					break;
			case XFLR5::FOURGRAPHS:
			case XFLR5::ALLGRAPHS:
					s_pMainFrame->m_pMiarexTileWidget->setMiarexGraphList(m_iView, m_TimeGraph, 4);
					break;
			}
			break;
		}

		default:
		{
			for(int ig=0; ig<maxWidgets; ig++)
			{
				s_pMainFrame->m_pMiarexTileWidget->graphWidget(ig)->setGraph(NULL);
			}
		}
	}
}


/**
 * Returns a pointer to the Plane's wing with the given index, or NULL if there is no active Plane object
 * @param iw the index of the wing
 * @return a pointer to the wing
 */
Wing *QMiarex::pWing(int iw)
{
	if(!m_pCurPlane) return NULL;
	return m_pCurPlane->wing(iw);
}


/**
 * Exports the geometrical data of the acitve wing or plane to a text file readable by AVL
 *@todo AVL expects consistency of the units, need to check all lines and cases
 */
void QMiarex::onExporttoSTL()
{
	if (!m_pCurPlane) return;
	QString filter ="STL File (*.stl)";
	QString FileName;

	FileName = m_pCurPlane->planeName();
	FileName.replace("/", " ");

	STLExportDialog dlg;
	if(dlg.exec()==QDialog::Rejected) return;

	QFileDialog Fdlg(this);
	FileName = Fdlg.getSaveFileName(this, tr("Export to STL File"),
									Settings::s_LastDirName + "/"+FileName+".stl",
									tr("STL File (*.stl)"),
									&filter, QFileDialog::DontConfirmOverwrite);

	if(!FileName.length()) return;


	bool bBinary = STLExportDialog::s_bBinary;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
	if(pos<0) FileName += ".stl";

	QFile XFile(FileName);

	if(STLExportDialog::s_iObject>0)
	{
		if(bBinary)
		{
			if (!XFile.open(QIODevice::WriteOnly)) return ;
			QDataStream out(&XFile);
			out.setByteOrder(QDataStream::LittleEndian);
			pWing(STLExportDialog::s_iObject-1)->exportSTLBinary(out, STLExportDialog::s_iChordPanels, STLExportDialog::s_iSpanPanels);
		}
		else
		{
			if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;
			QTextStream out(&XFile);
			pWing(STLExportDialog::s_iObject-1)->exportSTLText(out, STLExportDialog::s_iChordPanels, STLExportDialog::s_iSpanPanels);
		}
	}
	else if(STLExportDialog::s_iObject==0 && m_pCurPlane->body())
	{
		if(bBinary)
		{
			if (!XFile.open(QIODevice::WriteOnly)) return ;
			QDataStream out(&XFile);
			out.setByteOrder(QDataStream::LittleEndian);
			m_pCurPlane->body()->exportSTLBinary(out, STLExportDialog::s_iChordPanels, STLExportDialog::s_iSpanPanels);
		}
		else
		{
			if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;
			QTextStream out(&XFile);
		}
	}

	XFile.close();
}



void QMiarex::onImportSTLFile()
{
	if (!m_pCurPlane) return;
	QString filter ="STL Binary File (*.stl)";
	QString FileName;

	FileName = m_pCurPlane->planeName();
	FileName.replace("/", " ");
/*	QFileDialog dlg(this);
	FileName = dlg.getOpenFileName(this, tr("Import stl"),
									Settings::s_LastDirName + "/"+FileName+".stl",
									tr("STL Text File (*.stl);;STL Binary File (*.stl)"),
									&filter);

	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
	if(pos<0) FileName += ".stl";*/
	FileName = Settings::s_LastDirName + "/" + "0zPlane.stl";

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::ReadOnly)) return ;
	QDataStream inStream(&XFile);



	inStream.setByteOrder(QDataStream::LittleEndian);
//	uint u = inStream.byteOrder();
//qDebug()<<in.byteOrder()<<QDataStream::BigEndian<<QDataStream::LittleEndian;

	//80 character header, avoid word "solid"
	//                       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

	QString strong;
	qint8  ch;
	strong.clear();
	for(int j=0; j<80;j++)
	{
		strong += " ";
		inStream >> ch;
		strong[j] = char(ch);
	}


	int nTriangles=0;
	inStream >> nTriangles;

	float f,g,h;

	char buffer[12];
	for (int j=0; j<nTriangles; j++)
	{
		readFloat(inStream, f);
		readFloat(inStream, g);
		readFloat(inStream, h);
		readFloat(inStream, f);
		readFloat(inStream, g);
		readFloat(inStream, h);
		readFloat(inStream, f);
		readFloat(inStream, g);
		readFloat(inStream, h);
		readFloat(inStream, f);
		readFloat(inStream, g);
		readFloat(inStream, h);
		inStream.readRawData(buffer, 2);
		if(j>2) break;
	}


	XFile.close();
}




/**
 * Imports an XML file containing the definition of either a plane or a WPolar
 */
void QMiarex::onImportFromXml()
{
	QString PathName;
	PathName = QFileDialog::getOpenFileName(s_pMainFrame, tr("Open XML File"),
											Settings::s_xmlDirName,
											tr("XML file")+"(*.xml)");
	if(!PathName.length())		return ;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_xmlDirName = PathName.left(pos);

	QFile xmlFile(PathName);
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n")+PathName;
		QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
		return;
	}
	QXmlStreamReader xmlReader(&xmlFile);

	do{
		xmlReader.readNextStartElement();
        if (xmlReader.name().compare(QString("Plane_Polar"), Qt::CaseInsensitive)==0 && xmlReader.attributes().value("version") == "1.0")
		{
			// the file contains the definition of a WPolar
			xmlFile.close();
			importWPolarFromXML(xmlFile);
			break;
		}
        else if (xmlReader.name().compare(QString("explane"), Qt::CaseInsensitive)==0 && xmlReader.attributes().value("version") == "1.0")
		{
			// the file contains the definition of a Plane
			xmlFile.close();
			importPlaneFromXML(xmlFile);
			break;
		}
	} while(!xmlReader.atEnd() && !xmlReader.hasError() );

}



/**
 * Imports the plane geometry from an XML file
 */
void QMiarex::onImportPlanesfromXML()
{
	QStringList pathNames;
	pathNames = QFileDialog::getOpenFileNames(s_pMainFrame, tr("Open XML File"),
											  Settings::s_xmlDirName,
											  tr("Plane XML file")+"(*.xml)");
	if(!pathNames.size()) return;
	int pos = pathNames.at(0).lastIndexOf("/");
	if(pos>0) Settings::s_xmlDirName = pathNames.at(0).left(pos);

	for(int iFile=0; iFile<pathNames.size(); iFile++)
	{
		QFile XFile(pathNames.at(iFile));
		importPlaneFromXML(XFile);
	}
}


/**
 * Imports the analysis definition from an XML file
 */
void QMiarex::onImportAnalysisFromXML()
{
	QStringList pathNames;
	pathNames = QFileDialog::getOpenFileNames(s_pMainFrame, tr("Open XML File"),
											  Settings::s_xmlDirName,
											  tr("Analysis XML file")+"(*.xml)");
	if(!pathNames.size()) return ;
	int pos = pathNames.at(0).lastIndexOf("/");
	if(pos>0) Settings::s_xmlDirName = pathNames.at(0).left(pos);

	for(int iFile=0; iFile<pathNames.size(); iFile++)
	{
		QFile XFile(pathNames.at(iFile));
		importWPolarFromXML(XFile);
	}
}


/**
 * Imports the WPolar definition from an XML file
 */
void QMiarex::importWPolarFromXML(QFile &xmlFile)
{
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n")+xmlFile.fileName();
		QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
		return;
	}

	WPolar *pWPolar = new WPolar;
	XmlWPolarReader polarReader(xmlFile, pWPolar);
	polarReader.readXMLPolarFile();

	if(polarReader.hasError())
	{
		QString errorMsg = polarReader.errorString() + QString("\nline %1 column %2").arg(polarReader.lineNumber()).arg(polarReader.columnNumber());
		QMessageBox::warning(s_pMainFrame, "XML read", errorMsg, QMessageBox::Ok);
	}
	else
	{
		Plane *pPlane = Objects3D::getPlane(pWPolar->planeName());
		if(!pPlane && m_pCurPlane)
		{
			s_pMainFrame->statusBar()->showMessage(tr("Attaching the analysis to the active plane"));
			pWPolar->planeName() = m_pCurPlane->planeName();
			pPlane = m_pCurPlane;
		}
		else if(!pPlane)
		{
			s_pMainFrame->statusBar()->showMessage(tr("No plane to attach the polar to"));
			delete pWPolar;
			return;
		}

		m_pCurWPolar = Objects3D::insertNewWPolar(pWPolar, pPlane);
		m_pCurPOpp = NULL;

		m_bResetglGeom = true;
		m_bResetglMesh = true;
		m_bResetglOpp  = true;

		setWPolar();

		s_pMainFrame->updateWPolarListBox();
		emit projectModified();
	}
	updateView();
}



/**
 * Imports the plane geometry from an XML file
 */
void QMiarex::importPlaneFromXML(QFile &xmlFile)
{
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n")+xmlFile.fileName();
		QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
		return;
	}

	Plane *pPlane = new Plane;
	XMLPlaneReader planeReader(xmlFile, pPlane);
	planeReader.readXMLPlaneFile();

	if(planeReader.hasError())
	{
		QString errorMsg = planeReader.errorString() + QString("\nline %1 column %2").arg(planeReader.lineNumber()).arg(planeReader.columnNumber());
		QMessageBox::warning(s_pMainFrame, "XML read", errorMsg, QMessageBox::Ok);
	}
	else
	{
		if(Objects3D::planeExists(pPlane->planeName())) m_pCurPlane = Objects3D::setModPlane(pPlane);
		else                                            m_pCurPlane = Objects3D::addPlane(pPlane);

		setPlane();
		s_pMainFrame->updatePlaneListBox();
		emit projectModified();
	}
	updateView();
}





/**
 * Exports the plane geometry to an XML file
 */
void QMiarex::onExportPlanetoXML()
{
	if(!m_pCurPlane)return ;// is there anything to export ?


	QString filter = "XML file (*.xml)";
	QString FileName, strong;

	strong = m_pCurPlane->planeName();
	FileName = QFileDialog::getSaveFileName(s_pMainFrame, tr("Export plane definition to xml file"),
											Settings::s_xmlDirName +'/'+strong,
											filter,
											&filter);

	if(!FileName.length()) return;
	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_xmlDirName = FileName.left(pos);

	if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";


	QFile XFile(FileName);
	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;


	XMLPlaneWriter planeWriter(XFile);
	planeWriter.writeXMLPlane(m_pCurPlane);

	XFile.close();
}


/**
 * Exports the analysis data to an XML file
 */
void QMiarex::onExportAnalysisToXML()
{
	if(!m_pCurPlane || !m_pCurWPolar) return ;// is there anything to export ?

	QString filter = "XML file (*.xml)";
	QString FileName, strong;

	strong = m_pCurPlane->planeName()+"_"+m_pCurWPolar->polarName();
	strong.replace("/", "_");
	strong.replace(".", "_");

	FileName = QFileDialog::getSaveFileName(s_pMainFrame, tr("Export analysis definition to xml file"),
											Settings::s_xmlDirName +'/'+strong,
											filter,
											&filter);

	if(!FileName.length()) return;
	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_xmlDirName = FileName.left(pos);

	if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";


	QFile XFile(FileName);
	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;


	XmlWPolarWriter wpolarWriter(XFile);
	wpolarWriter.writeXMLWPolar(m_pCurWPolar);

	XFile.close();
}





/**
 * Returns a QString object holding the description and value of the polar's parameters
 * @param *pWPolar a pointer to the polar object
 * @param &PolarProperties the reference of the QString object to be filled with the description
 * @param bData true if the analysis data should be appended to the string
 */
void QMiarex::getPolarProperties(WPolar *pWPolar, QString &polarProps, bool bData)
{
	QString strong, lenunit, massunit, speedunit, areaunit;
	Units::getLengthUnitLabel(lenunit);
	Units::getAreaUnitLabel(areaunit);
	Units::getWeightUnitLabel(massunit);
	Units::getSpeedUnitLabel(speedunit);

	QString inertiaunit = massunit+"."+lenunit+QString::fromUtf8("²");

	Plane *pPlane = Objects3D::getPlane(pWPolar->planeName());

	polarProps.clear();

	if(pWPolar->polarType()==XFLR5::FIXEDSPEEDPOLAR)     strong = "Type 1: "+QObject::tr("Fixed speed") +"\n";
	else if(pWPolar->polarType()==XFLR5::FIXEDLIFTPOLAR) strong = "Type 2: "+QObject::tr("Fixed lift") +"\n";
	else if(pWPolar->polarType()==XFLR5::FIXEDAOAPOLAR)  strong = "Type 4: "+QObject::tr("Fixed angle of attack") +"\n";
	else if(pWPolar->polarType()==XFLR5::STABILITYPOLAR) strong = "Type 7: "+QObject::tr("Stability analysis") +"\n";
	else if(pWPolar->polarType()==XFLR5::BETAPOLAR)      strong = "Type 5: "+QObject::tr("Sideslip analysis") +"\n";
	polarProps += strong;

	if(pWPolar->polarType()==XFLR5::FIXEDSPEEDPOLAR)
	{
		strong  = QString(QObject::tr("VInf =")+"%1 ").arg(pWPolar->velocity()*Units::mstoUnit(),7,'g',2);
		polarProps += strong + speedunit+"\n";
	}
	else if(pWPolar->polarType()==XFLR5::FIXEDAOAPOLAR)
	{
		strong  = QString(QObject::tr("Alpha =")+"%1").arg(pWPolar->Alpha(),7,'f',2);
		polarProps += strong +QString::fromUtf8("°")+"\n";
	}
	else if(pWPolar->polarType()==XFLR5::BETAPOLAR)
	{
		strong  = QString(QObject::tr("Alpha =")+"%1").arg(pWPolar->Alpha(),7,'f',2);
		polarProps += strong +QString::fromUtf8("°")+"\n";
		strong  = QString(QObject::tr("VInf =")+"%1 ").arg(pWPolar->velocity()*Units::mstoUnit(),7,'g',2);
		polarProps += strong + speedunit+"\n";
	}

	if(pWPolar->polarType() != XFLR5::BETAPOLAR && qAbs(pWPolar->Beta())>PRECISION)
	{
		strong  = QString(QObject::tr("Beta")+" = %1").arg(pWPolar->Beta(),7,'f',2);
		polarProps += strong +QString::fromUtf8("°")+"\n";
	}

//	PolarProperties += QObject::tr("Method")+" = ";
	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)                                       polarProps +=QObject::tr("LLT");
	else if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD && !pWPolar->bThinSurfaces())   polarProps +=QObject::tr("3D-Panels");
	else if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD && pWPolar->bVLM1())            polarProps +=QObject::tr("3D-Panels/VLM1");
	else if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD && !pWPolar->bVLM1())           polarProps +=QObject::tr("3D-Panels/VLM2");
	polarProps +="\n";


	//Control data
	//Mass and inertia controls
	QString strLen, strMass, strInertia;
	Units::getLengthUnitLabel(strLen);
	Units::getWeightUnitLabel(strMass);
	strInertia = strMass+"."+strLen+QString::fromUtf8("²");


	//Angle controls
	if(pWPolar->m_ControlGain.size()<pWPolar->m_nControls && pWPolar->polarType()==XFLR5::STABILITYPOLAR && pPlane)
	{
		int j;
		int iCtrl = 0;

		strong = "AVL type controls\n";
		polarProps +=strong;


		if(!pPlane->isWing())
		{
			if(qAbs(pWPolar->m_ControlGain[iCtrl])>PRECISION)
			{
				strong = QString::fromUtf8("Wing Tilt: gain=%1°/unit\n").arg(pWPolar->m_ControlGain[iCtrl],0,'f',2);
				polarProps +=strong;
			}
			iCtrl=1;
			if(pPlane->stab())
			{
				if(qAbs(pWPolar->m_ControlGain[iCtrl])>PRECISION)
				{
					strong = QString::fromUtf8("Elev. Tilt: gain=%1°/unit\n").arg(pWPolar->m_ControlGain[iCtrl],0,'f',2);
					polarProps +=strong;
				}
				iCtrl=2;
			}
		}

		Wing *pStab, *pFin, *pWing;
		pStab = pFin = pWing = NULL;
		if(pPlane)
		{
			pWing = pPlane->wing();
			pStab = pPlane->stab();
			pFin  = pPlane->fin();
		}

		// flap controls
		//wing first
		int nFlap = 0;
		if(pWing)
		{
			for (j=0; j<pWing->m_Surface.size(); j++)
			{
				if(pWing->m_Surface.at(j)->m_bTEFlap)
				{
					if(qAbs(pWPolar->m_ControlGain[iCtrl])>PRECISION)
					{
						strong = QString(QString::fromUtf8("Wing Flap %1: g=%2°/unit\n"))
										.arg(nFlap+1)
										.arg(pWPolar->m_ControlGain[iCtrl],5,'f',2);
						polarProps +=strong;
					}
					nFlap++;
					iCtrl++;
				}
			}
		}

		//elevator next
		nFlap = 0;
		if(pStab)
		{
			for (j=0; j<pStab->m_Surface.size(); j++)
			{
				if(pStab->m_Surface.at(j)->m_bTEFlap)
				{
					if(qAbs(pWPolar->m_ControlGain[iCtrl])>PRECISION)
					{
						strong = QString(QString::fromUtf8("Elev. Flap %1: gain=%2°/unit\n"))
										.arg(nFlap+1)
										.arg(pWPolar->m_ControlGain[iCtrl],5,'f',2);
						polarProps +=strong;
					}
					nFlap++;
					iCtrl++;
				}
			}
		}

		nFlap = 0;
		if(pFin)
		{
			for (j=0; j<pFin->m_Surface.size(); j++)
			{
				if(pFin->m_Surface.at(j)->m_bTEFlap)
				{
					if(qAbs(pWPolar->m_ControlGain[iCtrl])>PRECISION)
					{
						strong = QString(QString::fromUtf8("Fin Flap %1: gain=%2°/unit\n"))
										.arg(nFlap+1)
										.arg(pWPolar->m_ControlGain[iCtrl],5,'f',2);
						polarProps +=strong;
					}
					nFlap++;
					iCtrl++;
				}
			}
		}
	}


	if(pWPolar->bAutoInertia())
	{
		polarProps += "Using plane inertia\n";
	}

	strong  = QString(QObject::tr("Mass")+" = %1 ").arg(pWPolar->mass()*Units::kgtoUnit(),7,'f',3);
	polarProps += strong + massunit;
	if(qAbs(pWPolar->m_inertiaGain[0])>PRECISION)
	{
		strong = QString::fromUtf8(" - g=%1").arg(pWPolar->m_inertiaGain[0]*Units::kgtoUnit(), 0,'f',2);
		strong += Units::weightUnitLabel() + "/ctrl\n";
		polarProps +=strong;
	}
	else polarProps +="\n";

	strong  = QString(QObject::tr("CoG.x")+" = %1 ").arg(pWPolar->CoG().x*Units::mtoUnit(),7,'g',4);
	polarProps += strong + lenunit;

	if(qAbs(pWPolar->m_inertiaGain[1])>PRECISION)
	{
		strong = QString::fromUtf8(" - g=%1").arg(pWPolar->m_inertiaGain[1]*Units::mtoUnit(), 0,'f',2);
		strong += Units::lengthUnitLabel() + "/ctrl\n";
		polarProps +=strong;
	}
	else polarProps +="\n";

	strong  = QString(QObject::tr("CoG.z")+" = %1 ").arg(pWPolar->CoG().z*Units::mtoUnit(),7,'g',4);
	polarProps += strong + lenunit + "\n";
	if(qAbs(pWPolar->m_inertiaGain[2])>PRECISION)
	{
		strong = QString::fromUtf8(" - g=%1").arg(pWPolar->m_inertiaGain[2]*Units::mtoUnit(), 0,'f',2);
		strong += Units::lengthUnitLabel() + "/ctrl";
		polarProps +=strong;
	}
	else polarProps +="\n";

	if(pWPolar->polarType()==XFLR5::STABILITYPOLAR)
	{
		strong  = QString("Ixx = %1 ").arg(pWPolar->CoGIxx()*Units::mtoUnit()*Units::mtoUnit()*Units::kgtoUnit(),7,'g',4);
		polarProps += strong + inertiaunit;
		if(qAbs(pWPolar->m_inertiaGain[3])>PRECISION)
		{
			strong = QString(QString::fromUtf8(" - g=%1")).arg(pWPolar->m_inertiaGain[3]*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit(),0,'f',2);
			polarProps += strInertia + "/ctrl";
			polarProps +=strong;
		}
		else polarProps +="\n";

		strong  = QString("Iyy = %1 ").arg(pWPolar->CoGIyy()*Units::mtoUnit()*Units::mtoUnit()*Units::kgtoUnit(),7,'g',4);
		polarProps += strong + inertiaunit;
		if(qAbs(pWPolar->m_inertiaGain[4])>PRECISION)
		{
			strong = QString(QString::fromUtf8(" - g=%1")).arg(pWPolar->m_inertiaGain[4]*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit(),0,'f',2);
			polarProps += strInertia + "/ctrl";
			polarProps +=strong;
		}
		else polarProps +="\n";

		strong  = QString("Izz = %1 ").arg(pWPolar->CoGIzz()*Units::mtoUnit()*Units::mtoUnit()*Units::kgtoUnit(),7,'g',4);
		polarProps += strong + inertiaunit;
		if(qAbs(pWPolar->m_inertiaGain[5])>PRECISION)
		{
			strong = QString(QString::fromUtf8(" - g=%1")).arg(pWPolar->m_inertiaGain[5]*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit(),0,'f',2);
			polarProps += strInertia + "/ctrl";
			polarProps +=strong;
		}
		else polarProps +="\n";

		strong  = QString("Ixz = %1 ").arg(pWPolar->CoGIxz()*Units::mtoUnit()*Units::mtoUnit()*Units::kgtoUnit(),7,'g',4);
		polarProps += strong + inertiaunit;
		if(qAbs(pWPolar->m_inertiaGain[6])>PRECISION)
		{
			strong = QString(QString::fromUtf8(" - g=%1")).arg(pWPolar->m_inertiaGain[6]*Units::kgtoUnit()*Units::mtoUnit()*Units::mtoUnit(),0,'f',2);
			polarProps += strInertia + "/ctrl";
			polarProps +=strong;
		}
		else polarProps +="\n";
	}


	if(pWPolar->analysisMethod() !=XFLR5::LLTMETHOD)
	{
		if(pWPolar->boundaryCondition()==XFLR5::DIRICHLET)  strong  = QObject::tr("B.C. = Dirichlet");
		else                                                strong  = QObject::tr("B.C. = Neumann");
		polarProps += strong +"\n";
	}

	polarProps += QObject::tr("Analysis type")+" = ";
	if(pWPolar->bViscous()) polarProps += QObject::tr("Viscous")+"\n";
	else                    polarProps += QObject::tr("Inviscid")+"\n";

	if(pPlane && pPlane->body())
	{
		polarProps += QObject::tr("Body panels")+" = ";
		if(pWPolar->bIgnoreBodyPanels()) polarProps += QObject::tr("ignored");
		else                             polarProps += QObject::tr("included");
		polarProps += "\n";
	}

	if(pWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)       polarProps += QObject::tr("Ref. dimensions = ")+QObject::tr("Planform")+"\n";
	else if(pWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM) polarProps += QObject::tr("Ref. dimensions = ")+QObject::tr("Projected")+"\n";

	polarProps += QObject::tr("Ref. area  =") + QString("%1").arg(pWPolar->referenceArea()*Units::m2toUnit(),7,'f',3) + areaunit +"\n";
	polarProps += QObject::tr("Ref. span  =") + QString("%1").arg(pWPolar->referenceSpanLength()*Units::mtoUnit(),7,'f',3) +lenunit +"\n";
	polarProps += QObject::tr("Ref. chord =") + QString("%1").arg(pWPolar->referenceChordLength()*Units::mtoUnit(),7,'f',3) + lenunit +"\n";


	if(pWPolar->bTilted()) polarProps += QObject::tr("Tilted geometry")+"\n";

	if(pWPolar->bGround())
	{
		strong = QString(QObject::tr("Ground height")+" = %1").arg(pWPolar->m_Height*Units::mtoUnit())+lenunit+"\n";
		polarProps += strong;
	}

	strong  = QString(QObject::tr("Density =")+"%1 kg/m3\n").arg(pWPolar->density(),0,'g',4);
	polarProps += strong;

	strong  = QString(QObject::tr("Viscosity =")+"%1").arg(pWPolar->viscosity(),0,'g',4);
	strong += QString::fromUtf8("m²/s\n");
	polarProps += strong;

	strong = QString(QObject::tr("Data points") +" = %1\n").arg(pWPolar->dataSize());
	polarProps += "\n"+strong;

	for(int ix=0; ix<MAXEXTRADRAG; ix++)
	{
		if(fabs(pWPolar->m_ExtraDragArea[ix])>PRECISION && fabs(pWPolar->m_ExtraDragCoef[ix])>PRECISION)
		{
			strong = QString("Extra drag: area=%1 ").arg(pWPolar->m_ExtraDragArea[ix]*Units::m2toUnit(), 7,'f',3)+Units::areaUnitLabel();
			strong +="  //  ";
			strong += QString("coeff.=%1").arg(pWPolar->m_ExtraDragCoef[ix], 7,'f',3);
			polarProps += "\n"+strong;
		}
	}

	if(!bData || pWPolar->dataSize()==0) return;
	QTextStream out;
	strong.clear();
	out.setString(&strong);
	exportToTextStream(pWPolar, out, Settings::s_ExportFileType, true);
	polarProps += "\n"+strong;
}



/**
 * Exports the data of the polar to a text stream
 * @param out the instance of output QTextStream
 * @param FileType TXT if the data is separated by spaces, CSV for a comma separator
 * @param bDataOnly true if the analysis parameters should not be output
 */
void QMiarex::exportToTextStream(WPolar *pWPolar, QTextStream &out, XFLR5::enumTextFileType FileType, bool bDataOnly)
{
	int j;
	QString Header, strong, str;

	if (FileType==XFLR5::TXT)
	{
		if(!bDataOnly)
		{
			strong = VERSIONNAME;
			strong += "\n\n";
			out << strong;

			strong ="Plane name :        "+ pWPolar->planeName() + "\n";
			out << strong;

			strong ="Polar name :        "+ pWPolar->polarName()+ "\n";
			out << strong;

			Units::getSpeedUnitLabel(str);
			str +="\n\n";

			if(pWPolar->polarType()==XFLR5::FIXEDSPEEDPOLAR)
			{
				strong = QString("Freestream speed : %1 ").arg(pWPolar->velocity()*Units::mstoUnit(),7,'f',3);
				strong +=str + "\n";
			}
			else if(pWPolar->polarType()==XFLR5::FIXEDAOAPOLAR)
			{
				strong = QString("Alpha = %1").arg(pWPolar->Alpha()) + QString::fromUtf8("°") + "\n";
			}
			else strong = "\n";

			out << strong;
		}

		Header = "   alpha      Beta       CL          CDi        CDv        CD         CY         Cl         Cm         Cn        Cni       QInf        XCP\n";
		out << Header;
		for (j=0; j<pWPolar->dataSize(); j++)
		{
			strong = QString(" %1  %2  %3  %4  %5  %6  %7  %8  %9  %10  %11  %12  %13\n")
					 .arg(pWPolar->m_Alpha[j],8,'f',3)
					 .arg(pWPolar->m_Beta[j],8,'f',3)
					 .arg(pWPolar->m_CL[j], 9,'f',6)
					 .arg(pWPolar->m_ICd[j],9,'f',6)
					 .arg(pWPolar->m_PCd[j],9,'f',6)
					 .arg(pWPolar->m_TCd[j],9,'f',6)
					 .arg(pWPolar->m_CY[j] ,9,'f',6)
					 .arg(pWPolar->m_GRm[j],9,'f',6)
					 .arg(pWPolar->m_GCm[j],9,'f',6)
					 .arg(pWPolar->m_GYm[j],9,'f',6)
					 .arg(pWPolar->m_IYm[j],9,'f',6)
					 .arg(pWPolar->m_QInfinite[j],8,'f',4)
					 .arg(pWPolar->m_XCP[j],9,'f',4);

			out << strong;
		}
	}
	else if (FileType==XFLR5::CSV)
	{
		if(!bDataOnly)
		{
			strong = VERSIONNAME;
			strong += "\n\n";
			out << strong;

			strong ="Plane name :, "+ pWPolar->planeName() + "\n";
			out << strong;

			strong ="Polar name :, "+ pWPolar->polarName() + "\n";
			out << strong;

			Units::getSpeedUnitLabel(str);
			str +="\n\n";
			strong = QString("Freestream speed :, %1 ").arg(pWPolar->velocity()*Units::mstoUnit(),3,'f',1);
			strong +=str;
			out << strong;
		}

		Header = "alpha, Beta, CL, CDi, CDv, CD, CY, Cl, Cm, Cn, Cni, QInf, XCP\n";
		out << Header;
		for (j=0; j<pWPolar->dataSize(); j++)
		{
//			strong.Format(" %8.3f,  %9.6f,  %9.6f,  %9.6f,  %9.6f,  %9.6f,  %9.6f,  %9.6f,  %9.6f,  %9.6f,  %8.4f,  %9.4f\n",
			strong = QString(" %1,  %2,  %3,  %4,  %5,  %6,  %7,  %8,  %9,  %10,  %11,  %12, %13\n")
					 .arg(pWPolar->m_Alpha[j],8,'f',3)
					 .arg(pWPolar->m_Beta[j], 8,'f',3)
					 .arg(pWPolar->m_CL[j], 9,'f',6)
					 .arg(pWPolar->m_ICd[j],9,'f',6)
					 .arg(pWPolar->m_PCd[j],9,'f',6)
					 .arg(pWPolar->m_TCd[j],9,'f',6)
					 .arg(pWPolar->m_CY[j] ,9,'f',6)
					 .arg(pWPolar->m_GRm[j],9,'f',6)
					 .arg(pWPolar->m_GCm[j],9,'f',6)
					 .arg(pWPolar->m_GYm[j],9,'f',6)
					 .arg(pWPolar->m_IYm[j],9,'f',6)
					 .arg(pWPolar->m_QInfinite[j],8,'f',4)
					 .arg(pWPolar->m_XCP[j],9,'f',4);

			out << strong;

		}
	}
	out << "\n\n";
}

/**
 * @brief Returns a title for the plane's OpPoint. This title will typically be used  in the legend of the graphs.
 * @param pPOpp a pointer to the plane OpPoint instance.
 * @return the the plane OpPoint title.
 */
QString QMiarex::POppTitle(PlaneOpp *pPOpp)
{
	QString strong;

	if(pPOpp->isLLTMethod()) strong ="LLT - ";
	else if(pPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
	{
		if(pPOpp->m_bThinSurface)
		{
			pPOpp->m_bVLM1 ? strong = "VLM1 - " : strong = "VLM2 - ";
		}
		else strong = "Panels";
	}

	strong +=" ";

	if(pPOpp->polarType()==XFLR5::STABILITYPOLAR)
	{
		strong += QString("ctrl=%1-").arg(pPOpp->ctrl());
	}
	strong += QString::fromUtf8("%1°-").arg(pPOpp->alpha(), 7,'f',3);

	if(fabs(pPOpp->beta())>PRECISION) strong += QString::fromUtf8("%1°-").arg(pPOpp->beta(), 7,'f',3);

	strong += QString("%1").arg(pPOpp->QInf()*Units::mstoUnit());
	strong +=Units::speedUnitLabel();

	if(pPOpp->m_bTiltedGeom) strong += "-tilted";

	return strong;
}




/**
 * Returns the name of the variable referenced by iVar
 * @param iVar the index of the variable
 * @param Name the name of the variable as a QString object
 */
QString QMiarex::WPolarVariableName(int iVar)
{
	QString strLength  = Units::lengthUnitLabel();
	QString strSpeed   = Units::speedUnitLabel();
	QString strMoment  = Units::momentUnitLabel();
	QString strMass    = Units::weightUnitLabel();
	QString strForce   = Units::forceUnitLabel();

	switch (iVar)
	{
		case 0:
			return "Alpha";
			break;
		case 1:
			return "Beta";
			break;
		case 2:
			return "CL";
			break;
		case 3:
			return "CD";
			break;
		case 4:
			return "CD_viscous";
			break;
		case 5:
			return "CD_induced";
			break;
		case 6:
			return "CY";
			break;
		case 7:
			return "Cm";// Total Pitching moment coef.
			break;
		case 8:
			return "Cm_viscous";// Viscous Pitching moment coef.
			break;
		case 9:
			return "Cm_induced";// Induced Pitching moment coef.
			break;
		case 10:
			return "Cl";// Total Rolling moment coef.
			break;
		case 11:
			return "Cn";// Total Yawing moment coef.
			break;
		case 12:
			return "Cn_viscous";// Profile yawing moment
			break;
		case 13:
			return "Cn_induced";// Induced yawing moment
			break;
		case 14:
			return "CL/CD";
			break;
		case 15:
			return "CL^(3/2)/CD";
			break;
		case 16:
			return "1/Rt(CL)";
			break;
		case 17:
			return "Fx ("+strForce+")";
			break;
		case 18:
			return "Fy ("+strForce+")";
			break;
		case 19:
			return "Fz ("+strForce+")";
			break;
		case 20:
			return "Vx ("+strSpeed+")";
			break;
		case 21:
			return "Vz ("+strSpeed+")";
			break;
		case 22:
			return "V ("+strSpeed+")";
			break;
		case 23:
			return "Gamma";
			break;
		case 24:
			return "L ("+ strMoment+")";
			break;
		case 25:
			return "M ("+ strMoment+")";
			break;
		case 26:
			return "N ("+ strMoment+")";
			break;
		case 27:
			return "CPx ("+ strLength+")";
			break;
		case 28:
			return "CPy ("+ strLength+")";
			break;
		case 29:
			return "CPz ("+ strLength+")";
			break;
		case 30:
			return "BM ("+ strMoment+")";
			break;
		case 31:
			return "m.g.Vz (W)";
			break;
		case 32:
			return "Efficiency";
			break;
		case 33:
			return "XCp.Cl";
			break;
		case 34:
			return "(XCp-XCG)/MAC(%)";
			break;
		case 35:
			return "ctrl";
			break;
		case 36:
			return "XNP ("+ strLength+")";
			break;
		case 37:
			return "Phugoid Freq. (Hz)";
			break;
		case 38:
			return "Phugoid Damping";
			break;
		case 39:
			return "Short Period Freq. (Hz)";
			break;
		case 40:
			return "Short Period Damping Ratio";
			break;
		case 41:
			return "Dutch Roll Freq. (Hz)";
			break;
		case 42:
			return "Dutch Roll Damping";
			break;
		case 43:
			return "Roll mode t2 (s)";
			break;
		case 44:
			return "Spiral mode t2 (s)";
			break;
		case 45:
			return "Fx.Vx (W)";
			break;
		case 46:
			return "Extra drag ("+strForce+")";
			break;
		case 47:
			return "Mass ("+strMass+")";
			break;
		case 48:
			return "CoG_x ("+ strLength+")";
			break;
		case 49:
			return "CoG_z ("+ strLength+")";
			break;


	default:
			return "";
	}
	return "";
}
