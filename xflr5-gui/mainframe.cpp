/****************************************************************************

	MainFrame  Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <mainframe.h>
#include <globals.h>
#include <design/AFoil.h>
#include <miarex/Miarex.h>
#include <miarex/design/InertiaDlg.h>
#include <miarex/design/EditObjectDelegate.h>
#include <miarex/Objects3D.h>
#include <miarex/analysis/WPolarDlg.h>
#include <miarex/analysis/StabPolarDlg.h>
#include <miarex/analysis/PanelAnalysisDlg.h>
#include <miarex/design/BodyTransDlg.h>
#include <miarex/design/GL3dWingDlg.h>
#include <miarex/design/GL3dBodyDlg.h>
#include <miarex/view/GL3DScales.h>
#include <miarex/view/StabViewDlg.h>
#include <miarex/view/W3dPrefsDlg.h>
#include <miarex/design/PlaneDlg.h>
#include <miarex/mgt/PlaneTableDelegate.h>
#include <misc/AboutQ5.h>
#include <misc/ObjectPropsDlg.h>
#include <misc/LinePickerDlg.h>
#include <misc/Settings.h>
#include <misc/SaveOptionsDlg.h>
#include <misc/TranslatorDlg.h>
#include <misc/RenameDlg.h>
#include <misc/Units.h>
#include <misc/EditPlrDlg.h>
#include <graph/GraphDlg.h>
#include <xdirect/XDirect.h>
#include <xdirect/analysis/BatchDlg.h>
#include <xdirect/analysis/BatchThreadDlg.h>
#include <xdirect/geometry/NacaFoilDlg.h>
#include <xdirect/geometry/InterpolateFoilsDlg.h>
#include <xdirect/geometry/CAddDlg.h>
#include <xdirect/geometry/TwoDPanelDlg.h>
#include <xdirect/geometry/FoilCoordDlg.h>
#include <xdirect/geometry/FoilGeomDlg.h>
#include <xdirect/geometry/TEGapDlg.h>
#include <xdirect/geometry/LEDlg.h>
#include <xdirect/ManageFoilsDlg.h>
#include <xdirect/analysis/XFoilAnalysisDlg.h>
#include <xdirect/analysis/FoilPolarDlg.h>
#include <xinverse/XInverse.h>
#include <objects/Foil.h>
#include <objects/Polar.h>
#include <objects/Wing.h>
#include "openglinfodlg.h"

#include "inverseviewwidget.h"
#include "gl3widget.h"
#include "Direct2dDesign.h"
#include "xdirecttilewidget.h"
#include "miarextilewidget.h"

#include <script/xflscriptreader.h>
#include <script/xflscriptexec.h>


#include <QMessageBox>
#include <QtCore>
#include <QToolBar>
#include <QDockWidget>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QOpenGLContext>
#include <QSysInfo>


#ifdef Q_OS_MAC
	#include <CoreFoundation/CoreFoundation.h>
#endif


QPointer<MainFrame> MainFrame::_self = 0L;

QString MainFrame::s_ProjectName = "";
QString MainFrame::s_LanguageFilePath = "";
QDir MainFrame::s_StylesheetDir;
QDir MainFrame::s_TranslationDir;

bool MainFrame::s_bShowMousePos = true;
bool MainFrame::s_bSaved = true;
#ifdef QT_DEBUG
	bool MainFrame::s_bTrace = true;
#else
	bool MainFrame::s_bTrace = false;
#endif

QFile *MainFrame::s_pTraceFile = NULL;

QLabel *MainFrame::m_pctrlProjectName = NULL;
QList <QColor> MainFrame::s_ColorList;



MainFrame::MainFrame(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
	setAttribute(Qt::WA_DeleteOnClose);
    s_bTrace = false;

	if(s_bTrace)
	{
		QString FileName = QDir::tempPath() + "/Trace.log";
		s_pTraceFile = new QFile(FileName);

		if (!s_pTraceFile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) s_bTrace = false;
		s_pTraceFile->reset();

#if QT_VERSION >= 0x050500
		QSysInfo sysInfo;
		switch(sysInfo.windowsVersion())
		{
			case QSysInfo::WV_CE:          Trace("Windows CE"); break;
			case QSysInfo::WV_CENET:       Trace("Windows CE .NET"); break;
			case QSysInfo::WV_CE_5:        Trace("Windows CE 5.x"); break;
			case QSysInfo::WV_CE_6:        Trace("Windows CE 6.x"); break;
			case QSysInfo::WV_DOS_based:   Trace("MS-DOS-based version of Windows"); break;
			case QSysInfo::WV_NT_based:    Trace("NT-based version of Windows"); break;
			case QSysInfo::WV_CE_based:    Trace("CE-based version of Windows"); break;
			case QSysInfo::WV_None:        Trace("Operating system other than Windows."); break;
			case QSysInfo::WV_32s:         Trace("Windows 3.1 with Win 32s"); break;
			case QSysInfo::WV_95:          Trace("Windows 95"); break;
			case QSysInfo::WV_98:          Trace("Windows 98"); break;
			case QSysInfo::WV_Me:          Trace("Windows Me"); break;
			case QSysInfo::WV_NT:	       Trace("Windows NT (operating system version 4.0)"); break;
			case QSysInfo::WV_2000:	       Trace("Windows 2000 (operating system version 5.0)"); break;
			case QSysInfo::WV_XP:	       Trace("Windows XP (operating system version 5.1)"); break;
			case QSysInfo::WV_2003:	       Trace("Windows Server 2003, Windows Server 2003 R2, Windows Home Server, Windows XP Professional x64 Edition (operating system version 5.2)"); break;
			case QSysInfo::WV_VISTA:	   Trace("Windows Vista, Windows Server 2008 (operating system version 6.0)"); break;
			case QSysInfo::WV_WINDOWS7:    Trace("Windows 7, Windows Server 2008 R2 (operating system version 6.1)"); break;
			case QSysInfo::WV_WINDOWS8:    Trace("Windows 8 (operating system version 6.2)"); break;
			case QSysInfo::WV_WINDOWS8_1:  Trace("Windows 8.1 (operating system version 6.3), introduced in Qt 5.2"); break;
			case QSysInfo::WV_WINDOWS10:   Trace("Windows 10 (operating system version 10.0), introduced in Qt 5.5"); break;
		}
		switch(sysInfo.macVersion())
		{
			case QSysInfo::MV_9:        Trace("Mac OS 9 (unsupported)"); break;
			case QSysInfo::MV_Unknown:  Trace("An unknown and currently unsupported platform"); break;
			case QSysInfo::MV_CHEETAH:  Trace("CHEETAH MV_10_0"); break;
			case QSysInfo::MV_PUMA:     Trace("PUMA MV_10_1"); break;
			case QSysInfo::MV_JAGUAR:   Trace("JAGUAR MV_10_2"); break;
			case QSysInfo::MV_PANTHER:  Trace("PANTHER MV_10_3"); break;
			case QSysInfo::MV_TIGER:    Trace("TIGER MV_10_4"); break;
			case QSysInfo::MV_LEOPARD:  Trace("LEOPARD MV_10_5"); break;
			case QSysInfo::MV_SNOWLEOPARD:  Trace("SNOWLEOPARD MV_10_6"); break;
			case QSysInfo::MV_LION:         Trace("LION MV_10_7"); break;
			case QSysInfo::MV_MOUNTAINLION: Trace("MOUNTAINLION MV_10_8"); break;
			case QSysInfo::MV_MAVERICKS:    Trace("MAVERICKS MV_10_9"); break;
			case QSysInfo::MV_YOSEMITE:     Trace("YOSEMITE MV_10_10"); break;
			case QSysInfo::MV_ELCAPITAN:    Trace("ELCAPITAN MV_10_11"); break;
			case QSysInfo::MV_IOS:          Trace("iOS (any)"); break;
			case QSysInfo::MV_IOS_4_3:  Trace("iOS 4.3"); break;
			case QSysInfo::MV_IOS_5_0:  Trace("iOS 5.0"); break;
			case QSysInfo::MV_IOS_5_1:  Trace("iOS 5.1"); break;
			case QSysInfo::MV_IOS_6_0:  Trace("iOS 6.0"); break;
			case QSysInfo::MV_IOS_6_1:  Trace("iOS 6.1"); break;
			case QSysInfo::MV_IOS_7_0:  Trace("iOS 7.0"); break;
			case QSysInfo::MV_IOS_7_1:  Trace("iOS 7.1"); break;
			case QSysInfo::MV_IOS_8_0:  Trace("iOS 8.0"); break;
			case QSysInfo::MV_IOS_8_1:  Trace("iOS 8.1"); break;
			case QSysInfo::MV_IOS_8_2:  Trace("iOS 8.2"); break;
			case QSysInfo::MV_IOS_8_3:  Trace("iOS 8.3"); break;
			case QSysInfo::MV_IOS_8_4:  Trace("iOS 8.4"); break;
			case QSysInfo::MV_IOS_9_0:  Trace("iOS 9.0"); break;
			case QSysInfo::MV_None:  Trace("Not a Darwin operating system"); break;
		}

		Trace("build ABI: " + sysInfo.buildAbi());
		Trace("build CPU: " + sysInfo.buildCpuArchitecture());
		Trace("current CPU: " + sysInfo.currentCpuArchitecture());
		Trace("kernel type: "+sysInfo.kernelType());
		Trace("kernel version: "+sysInfo.kernelVersion());
		Trace("product name: "+sysInfo.prettyProductName());
		Trace("product type: " +sysInfo.productType());
		Trace("product version: " +sysInfo.productVersion());
#endif

		QString strange;
		strange.sprintf("Default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
		Trace(strange);
	}



	setWindowTitle(VERSIONNAME);
	setWindowIcon(QIcon(":/images/xflr5_64.png"));

	Settings sets(this);//to initialize the static variables
	//"Qt does not support style hints on X11 since this information is not provided by the window system."
	Settings::s_TextFont.setStyleHint(QFont::TypeWriter, QFont::OpenGLCompatible);
	Settings::s_TextFont.setStyleStrategy(QFont::OpenGLCompatible);
	Settings::s_TextFont.setFamily(Settings::s_TextFont.defaultFamily());
	Settings::s_TextFont.setPointSize(9);

	Settings::s_TableFont.setStyleHint(QFont::TypeWriter);
	Settings::s_TableFont.setStyleStrategy(QFont::PreferDevice);
	Settings::s_TableFont.setFamily(Settings::s_TableFont.defaultFamily());
	Settings::s_TableFont.setPointSize(8);

	Settings::s_RefGraph.setTitleFont(Settings::s_TextFont);
	Settings::s_RefGraph.setLabelFont(Settings::s_TextFont);

//	Settings::s_StyleSheetName = "xflr5_style";
	Settings::s_StyleSheetName = "";

	if(!QGLFormat::hasOpenGL())
	{
		QMessageBox::warning(this, tr("Warning"), tr("Your system does not provide support for OpenGL.\nXFLR5 will not operate correctly."));
	}
	
	createDockWindows();

	m_ImageFormat = XFLR5::PNG;
	Settings::s_ExportFileType = XFLR5::TXT;

	m_bAutoLoadLast = false;
	m_bAutoSave     = false;
	m_bSaveOpps     = false;
	m_bSaveWOpps    = true;
	m_bSaveSettings = true;

	m_SaveInterval = 10;
	m_GraphExportFilter = "Comma Separated Values (*.csv)";

	m_pSaveTimer = NULL;

	QAFoil *pAFoil       = (QAFoil*)m_pAFoil;
	QXDirect *pXDirect   = (QXDirect*)m_pXDirect;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	QMiarex *pMiarex     = (QMiarex*)m_pMiarex;

#ifdef Q_OS_MAC
		QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
	QString str;
	int kx=117, ky=57;
	bool bOK;
	settings.beginGroup("MainFrame");
	{
		str = settings.value("LanguageFilePath").toString();
		if(str.length()) s_LanguageFilePath = str;
		kx = settings.value("FrameGeometryx", 117).toInt(&bOK);
		ky = settings.value("FrameGeometryy",  57).toInt(&bOK);
	}
	settings.endGroup();

	XFoilAnalysisDlg::s_Position = QPoint(kx+31, ky+31);
	BatchThreadDlg::s_Position   = QPoint(kx+31, ky+31);
	BatchDlg::s_Position         = QPoint(kx+31, ky+31);
	LLTAnalysisDlg::s_Position   = QPoint(kx+31, ky+31);
	PanelAnalysisDlg::s_Position = QPoint(kx+31, ky+31);
	EditPlrDlg::s_Position       = QPoint(kx+31, ky+31);
	InertiaDlg::s_Position       = QPoint(kx+31, ky+31);


	if(s_LanguageFilePath.length())
	{
		qApp->removeTranslator(&m_Translator);
		if(m_Translator.load(s_LanguageFilePath))
		{
			qApp->installTranslator(&m_Translator);
		}
	}

	if(loadSettings())
	{
		Settings::loadSettings(&settings);

		pAFoil->loadSettings(&settings);
		pXDirect->loadSettings(&settings);
		pMiarex->loadSettings(&settings);
		pXInverse->loadSettings(&settings);

		GL3DScales::loadSettings(&settings);
		W3dPrefsDlg::loadSettings(&settings);
	}

	if(m_pSaveTimer)
	{
		m_pSaveTimer->stop();
		delete m_pSaveTimer;
	}
	m_pSaveTimer = new QTimer(this);
	m_pSaveTimer->setInterval(m_SaveInterval*60*1000);
	m_pSaveTimer->start();
	connect(m_pSaveTimer, SIGNAL(timeout()), this, SLOT(onSaveTimer()));

	setupDataDir();

	pXDirect->setAnalysisParams();
	createActions();
	createMenus();
	createToolbars();
	createStatusBar();

	m_pXDirectTileWidget->connectSignals();
	m_pMiarexTileWidget->connectSignals();

	s_ColorList.append(QColor(255,0,0));
	s_ColorList.append(QColor(0,255,0));
	s_ColorList.append(QColor(0,0,255));
	s_ColorList.append(QColor(255,255,0));
	s_ColorList.append(QColor(255,0,255));
	s_ColorList.append(QColor(0,255,255));

	s_bSaved     = true;

	m_iApp = XFLR5::NOAPP;
	m_pctrlAFoilToolBar->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();

	setMenus();

	QString styleSheet;
	if(Settings::s_bStyleSheets)
	{
	   readStyleSheet(Settings::s_StyleSheetName, styleSheet);
	}

    Objects3D::setStaticPointers();

	if(m_bAutoLoadLast)
	{
		loadLastProject();
	}
}




MainFrame::~MainFrame()
{
	if(s_pTraceFile) s_pTraceFile->close();

	for(int ioa=Foil::s_oaFoil.size()-1; ioa>=0; ioa--)
	{
		delete (Foil*)Foil::s_oaFoil.at(ioa);
		Foil::s_oaFoil.removeAt(ioa);
	}

	for(int ioa=Polar::s_oaPolar.size()-1; ioa>=0; ioa--)
	{
		delete (Polar*)Polar::s_oaPolar.at(ioa);
		Polar::s_oaPolar.removeAt(ioa);
	}

	for(int ioa=OpPoint::s_oaOpp.size()-1; ioa>=0; ioa--)
	{
		delete (OpPoint*)OpPoint::s_oaOpp.at(ioa);
		OpPoint::s_oaOpp.removeAt(ioa);
	}

	if(m_pSaveTimer)
	{
		m_pSaveTimer->stop();
		delete m_pSaveTimer;
	}
}


void MainFrame::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt(
#ifdef Q_OS_MAC
    this
#else
//            activeWindow()
	this
#endif // Q_OS_MAC
            );
#endif // QT_NO_MESSAGEBOX
}



void MainFrame::aboutXFLR5()
{
	AboutQ5 dlg(this);
	dlg.exec();
}


void MainFrame::addRecentFile(const QString &PathName)
{
	m_RecentFiles.removeAll(PathName);
	m_RecentFiles.prepend(PathName);
	while (m_RecentFiles.size() > MAXRECENTFILES)
		m_RecentFiles.removeLast();

	updateRecentFileActions();
}


void MainFrame::closeEvent (QCloseEvent * event)
{
	if(!s_bSaved)
	{
		int resp = QMessageBox::question(this, tr("Exit"), tr("Save the project before exit ?"),
										 QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
										 QMessageBox::Yes);
		if(resp == QMessageBox::Yes)
		{
			if(!saveProject(m_FileName))
			{
				event->ignore();
				return;
			}
			addRecentFile(m_FileName);
		}
		else if (resp==QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}
	deleteProject(true);

	saveSettings();
	event->accept();//continue closing
}



void MainFrame::createActions()
{
	m_pNewProjectAct = new QAction(QIcon(":/images/new.png"), tr("New Project"), this);
	m_pNewProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
	m_pNewProjectAct->setStatusTip(tr("Save and close the current project, create a new project"));
	connect(m_pNewProjectAct, SIGNAL(triggered()), this, SLOT(onNewProject()));

	m_pCloseProjectAct = new QAction(QIcon(":/images/new.png"), tr("Close the Project"), this);
	m_pCloseProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
	m_pCloseProjectAct->setStatusTip(tr("Save and close the current project"));
	connect(m_pCloseProjectAct, SIGNAL(triggered()), this, SLOT(onNewProject()));

	m_pOpenAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
	m_pOpenAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	m_pOpenAct->setStatusTip(tr("Open an existing file"));
	connect(m_pOpenAct, SIGNAL(triggered()), this, SLOT(onLoadFile()));

	m_pInsertAct = new QAction(tr("&Insert Project..."), this);
	m_pInsertAct->setStatusTip(tr("Insert an existing project in the current project"));
	m_pInsertAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
	connect(m_pInsertAct, SIGNAL(triggered()), this, SLOT(onInsertProject()));

	m_pOnAFoilAct = new QAction(tr("&Direct Foil Design"), this);
	m_pOnAFoilAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
	m_pOnAFoilAct->setStatusTip(tr("Open Foil Design application"));
	connect(m_pOnAFoilAct, SIGNAL(triggered()), this, SLOT(onAFoil()));

	m_pOnXInverseAct = new QAction(tr("&XFoil Inverse Design"), this);
	m_pOnXInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
	m_pOnXInverseAct->setStatusTip(tr("Open XFoil inverse analysis application"));
	connect(m_pOnXInverseAct, SIGNAL(triggered()), this, SLOT(onXInverse()));

	m_pOnMixedInverseAct = new QAction(tr("&XFoil Mixed Inverse Design"), this);
	m_pOnMixedInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
	m_pOnMixedInverseAct->setStatusTip(tr("Open XFoil Mixed Inverse analysis application"));
	connect(m_pOnMixedInverseAct, SIGNAL(triggered()), this, SLOT(onXInverseMixed()));

	m_pOnXDirectAct = new QAction(tr("&XFoil Direct Analysis"), this);
	m_pOnXDirectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));
	m_pOnXDirectAct->setStatusTip(tr("Open XFoil direct analysis application"));
	connect(m_pOnXDirectAct, SIGNAL(triggered()), this, SLOT(onXDirect()));

	m_pOnMiarexAct = new QAction(tr("&Wing and Plane Design"), this);
	m_pOnMiarexAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_6));
	m_pOnMiarexAct->setStatusTip(tr("Open Wing/plane design and analysis application"));
	connect(m_pOnMiarexAct, SIGNAL(triggered()), this, SLOT(onMiarex()));

	m_pSaveAct = new QAction(QIcon(":/images/save.png"), tr("Save"), this);
	m_pSaveAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	m_pSaveAct->setStatusTip(tr("Save the project to disk"));
	connect(m_pSaveAct, SIGNAL(triggered()), this, SLOT(onSaveProject()));

	m_pSaveProjectAsAct = new QAction(tr("Save Project As..."), this);
	m_pSaveProjectAsAct->setStatusTip(tr("Save the current project under a new name"));
	connect(m_pSaveProjectAsAct, SIGNAL(triggered()), this, SLOT(onSaveProjectAs()));

	m_pSaveOptionsAct = new QAction(tr("Load and Save Options"), this);
	connect(m_pSaveOptionsAct, SIGNAL(triggered()), this, SLOT(onSaveOptions()));

	m_pUnitsAct = new QAction(tr("Units..."), this);
	m_pUnitsAct->setStatusTip(tr("Define the units for this project"));
	connect(m_pUnitsAct, SIGNAL(triggered()), this, SLOT(onUnits()));

	m_pLanguageAct = new QAction(tr("Language..."), this);
	m_pLanguageAct->setStatusTip(tr("Define the default language for the application"));
	connect(m_pLanguageAct, SIGNAL(triggered()), this, SLOT(onLanguage()));

	m_pRestoreToolbarsAct	 = new QAction(tr("Restore toolbars"), this);
	m_pRestoreToolbarsAct->setStatusTip(tr("Restores the toolbars to their original state"));
	connect(m_pRestoreToolbarsAct, SIGNAL(triggered()), this, SLOT(onRestoreToolbars()));

	m_pSaveViewToImageFileAct = new QAction(tr("Save View to Image File"), this);
	m_pSaveViewToImageFileAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
	m_pSaveViewToImageFileAct->setStatusTip(tr("Saves the current view to a file on disk"));
	connect(m_pSaveViewToImageFileAct, SIGNAL(triggered()), this, SLOT(onSaveViewToImageFile()));

	m_pExecScript	 = new QAction(tr("Execute Script"), this);
	m_pExecScript->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
	m_pExecScript->setStatusTip(tr("Executes a set of foil and plane analysis defined in an xml file"));
	connect(m_pExecScript, SIGNAL(triggered()), this, SLOT(onExecuteScript()));

	m_pResetSettingsAct = new QAction(tr("Reset Default Settings"), this);
	m_pResetSettingsAct->setStatusTip(tr("will revert to default settings at the next session"));
	connect(m_pResetSettingsAct, SIGNAL(triggered()), this, SLOT(onResetSettings()));


	for (int i = 0; i < MAXRECENTFILES; ++i)
	{
		m_pRecentFileActs[i] = new QAction(this);
		m_pRecentFileActs[i]->setVisible(false);
		connect(m_pRecentFileActs[i], SIGNAL(triggered()), this, SLOT(onOpenRecentFile()));
	}

	m_pExportCurGraphAct = new QAction(tr("Export Graph"), this);
	m_pExportCurGraphAct->setStatusTip(tr("Export the current graph data to a text file"));
	connect(m_pExportCurGraphAct, SIGNAL(triggered()), this, SLOT(onExportCurGraph()));

	m_pResetCurGraphScales = new QAction(QIcon(":/images/OnResetGraphScale.png"), tr("Reset Graph Scales")+"\t(R)", this);
	m_pResetCurGraphScales->setStatusTip(tr("Restores the graph's x and y scales"));
	connect(m_pResetCurGraphScales, SIGNAL(triggered()), this, SLOT(onResetCurGraphScales()));

	m_pCurGraphDlgAct = new QAction(tr("Define Graph Settings")+"\t(G)", this);
	connect(m_pCurGraphDlgAct, SIGNAL(triggered()), this, SLOT(onCurGraphSettings()));

	m_pStyleAct = new QAction(tr("General Display Settings"), this);
	m_pStyleAct->setStatusTip(tr("Define the color and font options for all views and graphs"));
	connect(m_pStyleAct, SIGNAL(triggered()), this, SLOT(onStyleSettings()));

	m_pExitAct = new QAction(tr("E&xit"), this);
	m_pExitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	m_pExitAct->setStatusTip(tr("Exit the application"));
	connect(m_pExitAct, SIGNAL(triggered()), this, SLOT(close()));

	m_pOpenGLAct = new QAction(tr("OpenGL info"), this);
	connect(m_pOpenGLAct, SIGNAL(triggered()), this, SLOT(onOpenGLInfo()));

	m_pAboutAct = new QAction(tr("&About"), this);
	m_pAboutAct->setStatusTip(tr("More information about XFLR5"));
	connect(m_pAboutAct, SIGNAL(triggered()), this, SLOT(aboutXFLR5()));

	m_pAboutQtAct = new QAction(tr("About Qt"), this);
	connect(m_pAboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));

	createGraphActions();
	createAFoilActions();
	createXDirectActions();
	createXInverseActions();
	createMiarexActions();
}


void MainFrame::createAFoilActions()
{
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	m_pStoreSplineAct= new QAction(QIcon(":/images/OnStoreFoil.png"), tr("Store Splines as Foil"), this);
	m_pStoreSplineAct->setStatusTip(tr("Store the current splines in the foil database"));
	connect(m_pStoreSplineAct, SIGNAL(triggered()), pAFoil, SLOT(onStoreSplines()));

	m_pSplineControlsAct= new QAction(tr("Splines Params"), this);
	m_pSplineControlsAct->setStatusTip(tr("Define parameters for the splines : degree, number of out points"));
	connect(m_pSplineControlsAct, SIGNAL(triggered()), pAFoil, SLOT(onSplineControls()));

	m_pExportSplinesToFileAct= new QAction(tr("Export Splines To File"), this);
	m_pExportSplinesToFileAct->setStatusTip(tr("Define parameters for the splines : degree, number of out points"));
	connect(m_pExportSplinesToFileAct, SIGNAL(triggered()), pAFoil, SLOT(onExportSplinesToFile()));

	m_pNewSplinesAct= new QAction(tr("New Splines"), this);
	m_pNewSplinesAct->setStatusTip(tr("Reset the splines"));
	connect(m_pNewSplinesAct, SIGNAL(triggered()), pAFoil, SLOT(onNewSplines()));

	m_pUndoAFoilAct= new QAction(QIcon(":/images/OnUndo.png"), tr("Undo"), this);
	m_pUndoAFoilAct->setShortcut(Qt::CTRL + Qt::Key_Z);
	m_pUndoAFoilAct->setStatusTip(tr("Cancels the last modification"));
	connect(m_pUndoAFoilAct, SIGNAL(triggered()), pAFoil, SLOT(onUndo()));

	m_pRedoAFoilAct= new QAction(QIcon(":/images/OnRedo.png"), tr("Redo"), this);
	m_pRedoAFoilAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
	m_pRedoAFoilAct->setStatusTip(tr("Restores the last cancelled modification"));
	connect(m_pRedoAFoilAct, SIGNAL(triggered()), pAFoil, SLOT(onRedo()));

	m_pShowAllFoils= new QAction(tr("Show All Foils"), this);
	connect(m_pShowAllFoils, SIGNAL(triggered()), pAFoil, SLOT(onShowAllFoils()));
	m_pHideAllFoils= new QAction(tr("Hide All Foils"), this);
	connect(m_pHideAllFoils, SIGNAL(triggered()), pAFoil, SLOT(onHideAllFoils()));


	m_pAFoilDelete = new QAction(tr("Delete..."), this);
	connect(m_pAFoilDelete, SIGNAL(triggered()), pAFoil, SLOT(onDeleteCurFoil()));

	m_pAFoilRename = new QAction(tr("Rename..."), this);
	m_pAFoilRename->setShortcut(Qt::Key_F2);
	connect(m_pAFoilRename, SIGNAL(triggered()), pAFoil, SLOT(onRenameFoil()));

	m_pAFoilExport = new QAction(tr("Export..."), this);
	connect(m_pAFoilExport, SIGNAL(triggered()), pAFoil, SLOT(onExportCurFoil()));

	m_pAFoilDuplicateFoil = new QAction(tr("Duplicate..."), this);
	connect(m_pAFoilDuplicateFoil, SIGNAL(triggered()), pAFoil, SLOT(onDuplicate()));

	m_pShowCurrentFoil= new QAction(tr("Show Current Foil"), this);
	connect(m_pShowCurrentFoil, SIGNAL(triggered()), pAFoil, SLOT(onShowCurrentFoil()));

	m_pHideCurrentFoil= new QAction(tr("Hide Current Foil"), this);
	connect(m_pHideCurrentFoil, SIGNAL(triggered()), pAFoil, SLOT(onHideCurrentFoil()));

	m_pAFoilDerotateFoil = new QAction(tr("De-rotate the Foil"), this);
	connect(m_pAFoilDerotateFoil, SIGNAL(triggered()), pAFoil, SLOT(onAFoilDerotateFoil()));

	m_pAFoilNormalizeFoil = new QAction(tr("Normalize the Foil"), this);
	connect(m_pAFoilNormalizeFoil, SIGNAL(triggered()), pAFoil, SLOT(onAFoilNormalizeFoil()));

	m_pAFoilRefineGlobalFoil = new QAction(tr("Refine Globally")+"\t(F3)", this);
	connect(m_pAFoilRefineGlobalFoil, SIGNAL(triggered()), pAFoil, SLOT(onAFoilPanels()));

	m_pAFoilRefineLocalFoil = new QAction(tr("Refine Locally")+"\t(Shift+F3)", this);
	connect(m_pAFoilRefineLocalFoil, SIGNAL(triggered()), pAFoil, SLOT(onAFoilCadd()));

	m_pAFoilEditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
	connect(m_pAFoilEditCoordsFoil, SIGNAL(triggered()), pAFoil, SLOT(onAFoilFoilCoordinates()));

	m_pAFoilScaleFoil = new QAction(tr("Scale camber and thickness")+"\t(F9)", this);
	connect(m_pAFoilScaleFoil, SIGNAL(triggered()), pAFoil, SLOT(onAFoilFoilGeom()));

	m_pAFoilSetTEGap = new QAction(tr("Set T.E. Gap"), this);
	connect(m_pAFoilSetTEGap, SIGNAL(triggered()), pAFoil, SLOT(onAFoilSetTEGap()));

	m_pAFoilSetLERadius = new QAction(tr("Set L.E. Radius"), this);
	connect(m_pAFoilSetLERadius, SIGNAL(triggered()), pAFoil, SLOT(onAFoilSetLERadius()));

	m_pAFoilLECircle = new QAction(tr("Show LE Circle"), this);
	connect(m_pAFoilLECircle, SIGNAL(triggered()), pAFoil, SLOT(onAFoilLECircle()));

	m_pShowLegend = new QAction(tr("Show Legend"), this);
	m_pShowLegend->setCheckable(true);
	connect(m_pShowLegend, SIGNAL(triggered()), pAFoil, SLOT(onShowLegend()));

	m_pAFoilSetFlap = new QAction(tr("Set Flap")+"\t(F10)", this);
	connect(m_pAFoilSetFlap, SIGNAL(triggered()), pAFoil, SLOT(onAFoilSetFlap()));

	m_pAFoilInterpolateFoils = new QAction(tr("Interpolate Foils")+"\t(F11)", this);
	connect(m_pAFoilInterpolateFoils, SIGNAL(triggered()), pAFoil, SLOT(onAFoilInterpolateFoils()));

	m_pAFoilNacaFoils = new QAction(tr("Naca Foils"), this);
	connect(m_pAFoilNacaFoils, SIGNAL(triggered()), pAFoil, SLOT(onAFoilNacaFoils()));

	m_pAFoilTableColumns = new QAction(tr("Set Table Columns"), this);
	connect(m_pAFoilTableColumns, SIGNAL(triggered()), pAFoil, SLOT(onAFoilTableColumns()));

	m_pAFoilTableColumnWidths = new QAction(tr("Reset column widths"), this);
	connect(m_pAFoilTableColumnWidths, SIGNAL(triggered()), pAFoil, SLOT(onResetColumnWidths()));



	m_pAFoilGridAct= new QAction(tr("Grid Settings"), this);
	m_pAFoilGridAct->setStatusTip(tr("Define the grid settings for the view"));
	connect(m_pAFoilGridAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onGridSettings()));

	m_pInsertSplinePt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
	connect(m_pInsertSplinePt, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onInsertPt()));

	m_pRemoveSplinePt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
	connect(m_pRemoveSplinePt, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onRemovePt()));

	m_pResetXScaleAct= new QAction(QIcon(":/images/OnResetXScale.png"), tr("Reset X Scale"), this);
	m_pResetXScaleAct->setStatusTip(tr("Resets the scale to fit the current screen width"));
	connect(m_pResetXScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetXScale()));

	m_pResetXYScaleAct= new QAction(QIcon(":/images/OnResetFoilScale.png"), tr("Reset Scales")+"\t(R)", this);
	m_pResetXYScaleAct->setStatusTip(tr("Resets the x and y scales to screen size"));
	connect(m_pResetXYScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetScales()));


	m_pAFoilLoadImage = new QAction(tr("Load background image")   +"\tCtrl+Shift+I", this);
	connect(m_pAFoilLoadImage, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onLoadBackImage()));
	m_pAFoilClearImage = new QAction(tr("Clear background image") +"\tCtrl+Shift+I", this);

	connect(m_pAFoilClearImage, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onClearBackImage()));


	m_pResetYScaleAct= new QAction(tr("Reset Y Scale"), this);
	connect(m_pResetYScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetYScale()));

	m_pZoomInAct= new QAction(QIcon(":/images/OnZoomIn.png"), tr("Zoom in"), this);
	m_pZoomInAct->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
	connect(m_pZoomInAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomIn()));

	m_pZoomLessAct= new QAction(QIcon(":/images/OnZoomLess.png"), tr("Zoom Less"), this);
	m_pZoomLessAct->setStatusTip(tr("Zoom Less"));
	connect(m_pZoomLessAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomLess()));

	m_pZoomYAct= new QAction(QIcon(":/images/OnZoomYScale.png"), tr("Zoom Y Scale Only"), this);
	m_pZoomYAct->setStatusTip(tr("Zoom Y scale Only"));
	connect(m_pZoomYAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomYOnly()));
}



void MainFrame::createAFoilMenus()
{
	m_pAFoilViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pAFoilViewMenu->addAction(m_pShowCurrentFoil);
		m_pAFoilViewMenu->addAction(m_pHideCurrentFoil);
		m_pAFoilViewMenu->addAction(m_pShowAllFoils);
		m_pAFoilViewMenu->addAction(m_pHideAllFoils);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(m_pZoomInAct);
		m_pAFoilViewMenu->addAction(m_pZoomLessAct);
		m_pAFoilViewMenu->addAction(m_pResetXScaleAct);
		m_pAFoilViewMenu->addAction(m_pResetYScaleAct);
		m_pAFoilViewMenu->addAction(m_pResetXYScaleAct);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(m_pShowLegend);
		m_pAFoilViewMenu->addAction(m_pAFoilLECircle);
		m_pAFoilViewMenu->addAction(m_pAFoilGridAct);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(m_pAFoilLoadImage);
		m_pAFoilViewMenu->addAction(m_pAFoilClearImage);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(m_pSaveViewToImageFileAct);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(m_pStyleAct);
	}

	m_pAFoilDesignMenu = menuBar()->addMenu(tr("F&oil"));
	{
		m_pAFoilDesignMenu->addAction(m_pAFoilRename);
		m_pAFoilDesignMenu->addAction(m_pAFoilDelete);
		m_pAFoilDesignMenu->addAction(m_pAFoilExport);
		m_pAFoilDesignMenu->addAction(m_pAFoilDuplicateFoil);
		m_pAFoilDesignMenu->addSeparator();
		m_pAFoilDesignMenu->addAction(m_pHideAllFoils);
		m_pAFoilDesignMenu->addAction(m_pShowAllFoils);
		m_pAFoilDesignMenu->addSeparator();
		m_pAFoilDesignMenu->addAction(m_pAFoilNormalizeFoil);
		m_pAFoilDesignMenu->addAction(m_pAFoilDerotateFoil);
		m_pAFoilDesignMenu->addAction(m_pAFoilRefineLocalFoil);
		m_pAFoilDesignMenu->addAction(m_pAFoilRefineGlobalFoil);
		m_pAFoilDesignMenu->addAction(m_pAFoilEditCoordsFoil);
		m_pAFoilDesignMenu->addAction(m_pAFoilScaleFoil);
		m_pAFoilDesignMenu->addAction(m_pAFoilSetTEGap);
		m_pAFoilDesignMenu->addAction(m_pAFoilSetLERadius);
		m_pAFoilDesignMenu->addAction(m_pAFoilSetFlap);
		m_pAFoilDesignMenu->addSeparator();
		m_pAFoilDesignMenu->addAction(m_pAFoilInterpolateFoils);
		m_pAFoilDesignMenu->addAction(m_pAFoilNacaFoils);
		m_pAFoilDesignMenu->addAction(m_pManageFoilsAct);
	}

	m_pAFoilSplineMenu = menuBar()->addMenu(tr("&Splines"));
	{
		m_pAFoilSplineMenu->addAction(m_pInsertSplinePt);
		m_pAFoilSplineMenu->addAction(m_pRemoveSplinePt);
		m_pAFoilSplineMenu->addSeparator();
		m_pAFoilSplineMenu->addAction(m_pUndoAFoilAct);
		m_pAFoilSplineMenu->addAction(m_pRedoAFoilAct);
		m_pAFoilSplineMenu->addSeparator();
		m_pAFoilSplineMenu->addAction(m_pNewSplinesAct);
		m_pAFoilSplineMenu->addAction(m_pSplineControlsAct);
		m_pAFoilSplineMenu->addAction(m_pStoreSplineAct);
		m_pAFoilSplineMenu->addAction(m_pExportSplinesToFileAct);
	}

	//AFoil Context Menu
	m_pAFoilCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pAFoilDesignMenu_AFoilCtxMenu = m_pAFoilCtxMenu->addMenu(tr("F&oil"));
        {
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilRename);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilDelete);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilExport);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilDuplicateFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pHideAllFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pShowAllFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilNormalizeFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilDerotateFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilRefineLocalFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilRefineGlobalFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilEditCoordsFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilScaleFoil);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilSetTEGap);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilSetLERadius);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilSetFlap);
            m_pAFoilDesignMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilInterpolateFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pAFoilNacaFoils);
            m_pAFoilDesignMenu_AFoilCtxMenu->addAction(m_pManageFoilsAct);
        }
        //m_pAFoilCtxMenu->addMenu(m_pAFoilDesignMenu);
		m_pAFoilCtxMenu->addSeparator();
        m_pAFoilSplineMenu_AFoilCtxMenu = m_pAFoilCtxMenu->addMenu(tr("&Splines"));
        {
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pInsertSplinePt);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pRemoveSplinePt);
            m_pAFoilSplineMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pUndoAFoilAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pRedoAFoilAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addSeparator();
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pNewSplinesAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pSplineControlsAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pStoreSplineAct);
            m_pAFoilSplineMenu_AFoilCtxMenu->addAction(m_pExportSplinesToFileAct);
        }
        //m_pAFoilCtxMenu->addMenu(m_pAFoilSplineMenu);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pShowAllFoils);
		m_pAFoilCtxMenu->addAction(m_pHideAllFoils);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pResetXScaleAct);
		m_pAFoilCtxMenu->addAction(m_pResetYScaleAct);
		m_pAFoilCtxMenu->addAction(m_pResetXYScaleAct);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pShowLegend);
		m_pAFoilCtxMenu->addAction(m_pAFoilLECircle);
		m_pAFoilCtxMenu->addAction(m_pAFoilGridAct);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pAFoilLoadImage);
		m_pAFoilCtxMenu->addAction(m_pAFoilClearImage);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pSaveViewToImageFileAct);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pAFoilTableColumns);
		m_pAFoilCtxMenu->addAction(m_pAFoilTableColumnWidths);
	}
	m_pDirect2dWidget->setContextMenu(m_pAFoilCtxMenu);

	//Context menu to be displayed when user right clicks on a foil in the table
	m_pAFoilTableCtxMenu = new QMenu(tr("Foil Actions"),this);
	{
		m_pAFoilTableCtxMenu->addAction(m_pAFoilRename);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilDelete);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilExport);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilDuplicateFoil);
		m_pAFoilTableCtxMenu->addSeparator();
		m_pAFoilTableCtxMenu->addAction(m_pAFoilNormalizeFoil);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilDerotateFoil);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilRefineLocalFoil);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilRefineGlobalFoil);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilEditCoordsFoil);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilScaleFoil);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilSetTEGap);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilSetLERadius);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilSetFlap);
		m_pAFoilTableCtxMenu->addSeparator();
		m_pAFoilTableCtxMenu->addAction(m_pAFoilTableColumns);
		m_pAFoilTableCtxMenu->addAction(m_pAFoilTableColumnWidths);
	}
}


void MainFrame::createAFoilToolbar()
{
	m_pctrlAFoilToolBar = addToolBar(tr("Foil"));
	m_pctrlAFoilToolBar->addAction(m_pNewProjectAct);
	m_pctrlAFoilToolBar->addAction(m_pOpenAct);
	m_pctrlAFoilToolBar->addAction(m_pSaveAct);
	m_pctrlAFoilToolBar->addSeparator();
	m_pctrlAFoilToolBar->addAction(m_pZoomInAct);
	m_pctrlAFoilToolBar->addAction(m_pZoomLessAct);
	m_pctrlAFoilToolBar->addAction(m_pResetXYScaleAct);
	m_pctrlAFoilToolBar->addAction(m_pResetXScaleAct);
	m_pctrlAFoilToolBar->addAction(m_pZoomYAct);
	m_pctrlAFoilToolBar->addSeparator();
	m_pctrlAFoilToolBar->addAction(m_pUndoAFoilAct);
	m_pctrlAFoilToolBar->addAction(m_pRedoAFoilAct);

	m_pctrlAFoilToolBar->addSeparator();
	m_pctrlAFoilToolBar->addAction(m_pStoreSplineAct);
}



void MainFrame::createDockWindows()
{
	QAFoil::s_pMainFrame           = this;
	QXDirect::s_pMainFrame         = this;
	QXInverse::s_pMainFrame        = this;
	QMiarex::s_pMainFrame          = this;
	GL3Widget::s_pMainFrame        = this;
	Section2dWidget::s_pMainFrame  = this;
	GraphWidget::s_pMainFrame      = this;
	OpPointWidget::s_pMainFrame    = this;
	WingWidget::s_pMainFrame       = this;
	QGraph::s_pMainFrame           = this;


	m_pctrlXDirectWidget = new QDockWidget(tr("Direct foil analysis"), this);
	m_pctrlXDirectWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_pctrlXDirectWidget);

	m_pctrlXInverseWidget = new QDockWidget(tr("Inverse foil design"), this);
	m_pctrlXInverseWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_pctrlXInverseWidget);

	m_pctrlMiarexWidget = new QDockWidget(tr("Plane analysis"), this);
	m_pctrlMiarexWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_pctrlMiarexWidget);

	m_pctrlAFoilWidget = new QDockWidget(tr("Foil direct design"), this);
	m_pctrlAFoilWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, m_pctrlAFoilWidget);

	m_p2dWidget = new InverseViewWidget(this);
	m_pgl3Widget = new GL3Widget(this);

	m_pDirect2dWidget = new Direct2dDesign(this);
	m_pXDirectTileWidget = new XDirectTileWidget(this);
	m_pMiarexTileWidget  = new MiarexTileWidget(this);

	m_pXDirect = new QXDirect(this);
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	m_pctrlXDirectWidget->setWidget(pXDirect);
	m_pctrlXDirectWidget->setVisible(false);
	m_pctrlXDirectWidget->setFloating(true);
	m_pctrlXDirectWidget->move(960,60);

	m_pXInverse = new QXInverse(this);
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	m_pctrlXInverseWidget->setWidget(pXInverse);
	m_pctrlXInverseWidget->setVisible(false);
	m_pctrlXInverseWidget->setFloating(true);
	m_pctrlXInverseWidget->move(960,60);

	m_pMiarex = new QMiarex;
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	m_pctrlMiarexWidget->setWidget(pMiarex);
	m_pctrlMiarexWidget->setVisible(false);
	m_pctrlMiarexWidget->setFloating(true);
	m_pctrlMiarexWidget->move(960,60);


	m_pGL3DScales = new GL3DScales(this);
	GL3DScales * pGL3DScales = (GL3DScales*)m_pGL3DScales;
	GL3DScales::s_pMiarex      = m_pMiarex;
	m_pctrl3DScalesWidget = new QDockWidget(tr("3D Scales"), this);
	m_pctrl3DScalesWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_pctrl3DScalesWidget);
	m_pctrl3DScalesWidget->setWidget(pGL3DScales);
	pGL3DScales->setParent(m_pctrl3DScalesWidget);
	m_pctrl3DScalesWidget->setVisible(false);
	m_pctrl3DScalesWidget->setFloating(true);
	m_pctrl3DScalesWidget->move(60,60);

	StabViewDlg::s_pMiarex = m_pMiarex;
	m_pStabView = new StabViewDlg(this);
	StabViewDlg * pStabView = (StabViewDlg*)m_pStabView;
	m_pctrlStabViewWidget = new QDockWidget(tr("Stability"), this);
	m_pctrlStabViewWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_pctrlStabViewWidget);
	m_pctrlStabViewWidget->setWidget(pStabView);
	m_pctrlStabViewWidget->setVisible(false);
	m_pctrlStabViewWidget->setFloating(true);
	m_pctrlStabViewWidget->move(60,60);

	m_pctrlCentralWidget = new QStackedWidget;
	m_pctrlCentralWidget->addWidget(m_p2dWidget);
	m_pctrlCentralWidget->addWidget(m_pgl3Widget);
	m_pctrlCentralWidget->addWidget(m_pDirect2dWidget);
	m_pctrlCentralWidget->addWidget(m_pXDirectTileWidget);
	m_pctrlCentralWidget->addWidget(m_pMiarexTileWidget);


	setCentralWidget(m_pctrlCentralWidget);

	m_pAFoil  = new QAFoil(this);
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	pAFoil->m_p2DWidget = m_pDirect2dWidget;
	m_pctrlAFoilWidget->setWidget(pAFoil);
	m_pctrlAFoilWidget->setVisible(false);

	m_p2dWidget->m_pXInverse  = pXInverse;
	m_p2dWidget->m_pMainFrame = this;

	pMiarex->m_pgl3Widget = m_pgl3Widget;

	pXDirect->m_CpGraph.m_pParent    = m_p2dWidget;
	pXDirect->m_poaFoil  = &Foil::s_oaFoil;
	pXDirect->m_poaPolar = &Polar::s_oaPolar;
	pXDirect->m_poaOpp   = &OpPoint::s_oaOpp;

	pAFoil->initDialog(m_pDirect2dWidget, &Foil::s_oaFoil, pXDirect->m_pXFoil);

	QXInverse::s_p2DWidget        = m_p2dWidget;
	pXInverse->s_pMainFrame       = this;
	pXInverse->m_pXFoil           = pXDirect->m_pXFoil;
	pXInverse->m_poaFoil          = &Foil::s_oaFoil;

	GL3dWingDlg::s_poaFoil = &Foil::s_oaFoil;
	EditObjectDelegate::s_poaFoil = &Foil::s_oaFoil;

	LLTAnalysis::s_poaPolar = &Polar::s_oaPolar;

	GL3Widget::s_pMiarex = m_pMiarex;

	WingWidget::s_pMiarex         = m_pMiarex;
	XFoilAnalysisDlg::s_pXDirect  = m_pXDirect;
	NacaFoilDlg::s_pXFoil         = pXDirect->m_pXFoil;
	InterpolateFoilsDlg::s_pXFoil = pXDirect->m_pXFoil;
	CAddDlg::s_pXFoil             = pXDirect->m_pXFoil;
	TwoDPanelDlg::s_pXFoil        = pXDirect->m_pXFoil;
	FoilGeomDlg::s_pXFoil         = pXDirect->m_pXFoil;
	TEGapDlg::s_pXFoil            = pXDirect->m_pXFoil;
	LEDlg::s_pXFoil               = pXDirect->m_pXFoil;
	BatchDlg::s_pXDirect          = m_pXDirect;
	BatchThreadDlg::s_pXDirect    = m_pXDirect;

	GraphTileWidget::s_pMainFrame = this;
	GraphTileWidget::s_pMiarex    = m_pMiarex;
	GraphTileWidget::s_pXDirect   = m_pXDirect;

	LegendWidget::s_pMainFrame = this;
	LegendWidget::s_pMiarex    = m_pMiarex;
	LegendWidget::s_pXDirect   = m_pXDirect;


	pMiarex->connectSignals();
}



void MainFrame::createMenus()
{
	m_pFileMenu = menuBar()->addMenu(tr("&File"));
	{
		m_pFileMenu->addAction(m_pNewProjectAct);
		m_pFileMenu->addAction(m_pOpenAct);
		m_pFileMenu->addAction(m_pInsertAct);
		m_pFileMenu->addAction(m_pCloseProjectAct);
		m_pFileMenu->addSeparator();
		m_pFileMenu->addAction(m_pSaveAct);
		m_pFileMenu->addAction(m_pSaveProjectAsAct);
		m_pFileMenu->addSeparator();
//		m_pFileMenu->addAction(m_pExecScript);
//		m_pFileMenu->addSeparator();
		m_pFileMenu->addAction(m_pOnAFoilAct);
		m_pFileMenu->addAction(m_pOnXInverseAct);
		m_pFileMenu->addAction(m_pOnXDirectAct);
		m_pFileMenu->addAction(m_pOnMiarexAct);
		m_pSeparatorAct = m_pFileMenu->addSeparator();
		for (int i = 0; i < MAXRECENTFILES; ++i)
			m_pFileMenu->addAction(m_pRecentFileActs[i]);
		m_pFileMenu->addSeparator();
		m_pFileMenu->addAction(m_pExitAct);
		updateRecentFileActions();
	}

	m_pOptionsMenu = menuBar()->addMenu(tr("O&ptions"));
	{
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(m_pLanguageAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(m_pSaveOptionsAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(m_pUnitsAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(m_pRestoreToolbarsAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(m_pResetSettingsAct);
	}

	m_pGraphMenu = menuBar()->addMenu(tr("&Graphs"));
	{
		for(int ig=0; ig<MAXGRAPHS; ig++)
			m_pGraphMenu->addAction(m_pSingleGraph[ig]);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pTwoGraphs);
		m_pGraphMenu->addAction(m_pFourGraphs);
		m_pGraphMenu->addAction(m_pAllGraphs);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pAllGraphsSettings);
		m_pGraphMenu->addAction(m_pAllGraphsScalesAct);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pHighlightOppAct);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pShowMousePosAct);

	}

	m_pHelpMenu = menuBar()->addMenu(tr("&?"));
	{
		m_pHelpMenu->addAction(m_pOpenGLAct);
		m_pHelpMenu->addAction(m_pAboutQtAct);
		m_pHelpMenu->addAction(m_pAboutAct);
	}

	//Create Application-Specific Menus
	createXDirectMenus();
	createXInverseMenus();
	createMiarexMenus();
	createAFoilMenus();
}
 


void MainFrame::createGraphActions()
{
	for (int ig=0; ig<MAXGRAPHS; ++ig)
	{
		m_pSingleGraph[ig] = new QAction(tr("Graph")+QString(" %1\t(%2)").arg(ig+1).arg(ig+1), this);
		m_pSingleGraph[ig]->setData(ig);
		m_pSingleGraph[ig]->setCheckable(true);
		connect(m_pSingleGraph[ig], SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onSingleGraph()));
		connect(m_pSingleGraph[ig], SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onSingleGraph()));
	}
	m_pTwoGraphs = new QAction(tr("Two Graphs")+"\t(T)", this);
	m_pTwoGraphs->setStatusTip(tr("Display the first two graphs"));
	m_pTwoGraphs->setCheckable(true);
	connect(m_pTwoGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onTwoGraphs()));
	connect(m_pTwoGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onTwoGraphs()));

	m_pFourGraphs = new QAction(tr("Four Graphs")+"\t(F)", this);
	m_pFourGraphs->setStatusTip(tr("Display four graphs"));
	m_pFourGraphs->setCheckable(true);
	connect(m_pFourGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onFourGraphs()));
	connect(m_pFourGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onFourGraphs()));

	m_pAllGraphs = new QAction(tr("All Graphs")+"\t(A)", this);
	m_pAllGraphs->setStatusTip(tr("Display four graphs"));
	m_pAllGraphs->setCheckable(true);
	connect(m_pAllGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphs()));
	connect(m_pAllGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphs()));

	m_pGraphDlgAct = new QAction(tr("Define Graph Settings")+"\t(G)", this);
	m_pGraphDlgAct->setStatusTip(tr("Define the settings for the selected graph"));
	connect(m_pGraphDlgAct, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onCurGraphSettings()));
	connect(m_pGraphDlgAct, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onCurGraphSettings()));

	m_pAllGraphsScalesAct = new QAction(tr("Reset All Graph Scales"), this);
	m_pAllGraphsScalesAct->setStatusTip(tr("Reset the scales of all four polar graphs"));
	connect(m_pAllGraphsScalesAct, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphScales()));
	connect(m_pAllGraphsScalesAct, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphScales()));

	m_pAllGraphsSettings = new QAction(tr("All Graph Settings"), this);
	m_pAllGraphsSettings->setStatusTip(tr("Define the settings of all graphs"));
	connect(m_pAllGraphsSettings, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphSettings()));
	connect(m_pAllGraphsSettings, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphSettings()));

	m_pShowMousePosAct	 = new QAction(tr("Display mouse coordinates"), this);
	m_pShowMousePosAct->setCheckable(true);
	m_pShowMousePosAct->setChecked(s_bShowMousePos);
	m_pShowMousePosAct->setStatusTip(tr("Display the coordinates of the mouse on the top right corner of the graph"));
	connect(m_pShowMousePosAct, SIGNAL(triggered()), this, SLOT(onShowMousePos()));

}


void MainFrame::createMiarexActions()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	m_pWOppAct = new QAction(QIcon(":/images/OnWOppView.png"), tr("OpPoint View")+"\tF5", this);
	m_pWOppAct->setCheckable(true);
	m_pWOppAct->setStatusTip(tr("Switch to the Operating point view"));
//	WOppAct->setShortcut(Qt::Key_F5);
	connect(m_pWOppAct, SIGNAL(triggered()), pMiarex, SLOT(onWOppView()));

	m_pWPolarAct = new QAction(QIcon(":/images/OnPolarView.png"), tr("Polar View")+"\tF8", this);
	m_pWPolarAct->setCheckable(true);
	m_pWPolarAct->setStatusTip(tr("Switch to the Polar view"));
//	WPolarAct->setShortcut(Qt::Key_F8);
	connect(m_pWPolarAct, SIGNAL(triggered()), pMiarex, SLOT(onWPolarView()));

	m_pStabTimeAct = new QAction(QIcon(":/images/OnStabView.png"),tr("Time Response Vew")+"\tShift+F8", this);
	m_pStabTimeAct->setCheckable(true);
	m_pStabTimeAct->setStatusTip(tr("Switch to stability analysis post-processing"));
//	StabTimeAct->setShortcut(tr("Shift+F8"));
	connect(m_pStabTimeAct, SIGNAL(triggered()), pMiarex, SLOT(onStabTimeView()));

	m_pRootLocusAct = new QAction(QIcon(":/images/OnRootLocus.png"),tr("Root Locus View")+"\tCtrl+F8", this);
	m_pRootLocusAct->setCheckable(true);
	m_pRootLocusAct->setStatusTip(tr("Switch to root locus view"));
	connect(m_pRootLocusAct, SIGNAL(triggered()), pMiarex, SLOT(onRootLocusView()));

	m_pW3DAct = new QAction(QIcon(":/images/On3DView.png"), tr("3D View")+"\tF4", this);
	m_pW3DAct->setCheckable(true);
	m_pW3DAct->setStatusTip(tr("Switch to the 3D view"));
	connect(m_pW3DAct, SIGNAL(triggered()), pMiarex, SLOT(on3DView()));

	m_pCpViewAct = new QAction(QIcon(":/images/OnCpView.png"), tr("Cp View")+"\tF9", this);
	m_pCpViewAct->setCheckable(true);
	m_pCpViewAct->setStatusTip(tr("Switch to the Cp view"));
	connect(m_pCpViewAct, SIGNAL(triggered()), pMiarex, SLOT(onCpView()));

	m_pW3DPrefsAct = new QAction(tr("3D View Preferences"), this);
	m_pW3DPrefsAct->setStatusTip(tr("Define the preferences for the 3D view"));
	connect(m_pW3DPrefsAct, SIGNAL(triggered()), pMiarex, SLOT(on3DPrefs()));

	m_pMiarexPolarFilter = new QAction(tr("Polar Filter"), this);
	m_pMiarexPolarFilter->setStatusTip(tr("Define which type of polars should be shown or hidden"));
	connect(m_pMiarexPolarFilter, SIGNAL(triggered()), pMiarex, SLOT(onPolarFilter()));

	m_pReset3DScale = new QAction(tr("Reset scale")+"\t(R)", this);
	m_pReset3DScale->setStatusTip(tr("Resets the display scale so that the plane fits in the window"));
	connect(m_pReset3DScale, SIGNAL(triggered()), pMiarex, SLOT(on3DResetScale()));

	m_pShowFlapMoments = new QAction(tr("Show flap moments"), this);
	m_pShowFlapMoments->setCheckable(true);
	m_pShowFlapMoments->setStatusTip(tr("Display the flap moment values together with the other operating point results"));
	connect(m_pShowFlapMoments, SIGNAL(triggered()), pMiarex, SLOT(onShowFlapMoments()));

	m_pW3DScalesAct = new QAction(tr("3D Scales"), this);
	m_pW3DScalesAct->setStatusTip(tr("Define the scales for the 3D display of lift, moment, drag, and downwash"));
	m_pW3DScalesAct->setCheckable(true);
	connect(m_pW3DScalesAct, SIGNAL(triggered()), pMiarex, SLOT(onGL3DScale()));

	m_pW3DLightAct = new QAction(tr("3D Light Options"), this);
	m_pW3DLightAct->setStatusTip(tr("Define the light options in 3D view"));
	connect(m_pW3DLightAct, SIGNAL(triggered()), pMiarex, SLOT(onSetupLight()));

	m_pDefinePlaneAct = new QAction(tr("Define a New Plane")+"\tF3", this);
	m_pDefinePlaneAct->setStatusTip(tr("Shows a dialogbox to create a new plane definition"));
	connect(m_pDefinePlaneAct, SIGNAL(triggered()), pMiarex, SLOT(onNewPlane()));

	m_pDefinePlaneObjectAct = new QAction(tr("Define... (Advanced users)")+"\tF3", this);
	m_pDefinePlaneObjectAct->setStatusTip(tr("Shows a dialogbox to create a new plane definition"));
	connect(m_pDefinePlaneObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onNewPlaneObject()));

	m_pEditPlaneAct = new QAction(tr("Edit..."), this);
	m_pEditPlaneAct->setStatusTip(tr("Shows a form to edit the currently selected plane"));
	m_pEditPlaneAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	connect(m_pEditPlaneAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurPlane()));

	m_pEditObjectAct = new QAction(tr("Edit... (advanced users)"), this);
	m_pEditObjectAct->setStatusTip(tr("Shows a form to edit the currently selected plane"));
	m_pEditObjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT+Qt::Key_P));
	connect(m_pEditObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurObject()));

	m_pEditWingAct = new QAction(tr("Edit wing..."), this);
	m_pEditWingAct->setStatusTip(tr("Shows a form to edit the wing of the currently selected plane"));
	m_pEditWingAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	m_pEditWingAct->setData(0);
	connect(m_pEditWingAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWing()));

	m_pEditStabAct = new QAction(tr("Edit elevator..."), this);
	m_pEditStabAct->setData(2);
	m_pEditStabAct->setShortcut(Qt::CTRL + Qt::Key_E);
	connect(m_pEditStabAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWing()));

	m_pEditFinAct = new QAction(tr("Edit fin..."), this);
	m_pEditFinAct->setData(3);
	m_pEditFinAct->setShortcut(Qt::CTRL + Qt::Key_F);
	connect(m_pEditFinAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWing()));

	m_pEditBodyAct = new QAction(tr("Edit body..."), this);
	m_pEditBodyAct->setStatusTip(tr("Shows a form to edit the body of the currently selected plane"));
	m_pEditBodyAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
	connect(m_pEditBodyAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurBody()));

	m_pEditBodyObjectAct= new QAction(tr("Edit body (advanced users)"), this);
	m_pEditBodyObjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_B));
	connect(m_pEditBodyObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurBodyObject()));

	m_pRenameCurPlaneAct = new QAction(tr("Rename...")+"\tF2", this);
	m_pRenameCurPlaneAct->setStatusTip(tr("Rename the currently selected object"));
	connect(m_pRenameCurPlaneAct, SIGNAL(triggered()), pMiarex, SLOT(onRenameCurPlane()));

	m_pExporttoAVL = new QAction(tr("Export to AVL..."), this);
	m_pExporttoAVL->setStatusTip(tr("Export the current plane or wing to a text file in the format required by AVL"));
	connect(m_pExporttoAVL, SIGNAL(triggered()), pMiarex, SLOT(onExporttoAVL()));

	m_pExporttoSTL = new QAction(tr("Export to STL..."), this);
	m_pExporttoSTL->setStatusTip(tr("Export the current wing to a file in the STL format"));
	connect(m_pExporttoSTL, SIGNAL(triggered()), pMiarex, SLOT(onExporttoSTL()));

	m_pExportCurWOpp = new QAction(tr("Export..."), this);
	m_pExportCurWOpp->setStatusTip(tr("Export the current operating point to a text or csv file"));
	connect(m_pExportCurWOpp, SIGNAL(triggered()), pMiarex, SLOT(onExportCurPOpp()));

	m_pScaleWingAct = new QAction(tr("Scale Wing"), this);
	m_pScaleWingAct->setStatusTip(tr("Scale the dimensions of the currently selected wing"));
	connect(m_pScaleWingAct, SIGNAL(triggered()), pMiarex, SLOT(onScaleWing()));

	m_pManagePlanesAct = new QAction(tr("Manage objects"), this);
	m_pManagePlanesAct->setStatusTip(tr("Rename or delete the planes and wings stored in the database"));
	m_pManagePlanesAct->setShortcut(Qt::Key_F7);
	connect(m_pManagePlanesAct, SIGNAL(triggered()), pMiarex, SLOT(onManagePlanes()));

	m_pImportWPolar = new QAction(tr("Import Polar"), this);
	m_pImportWPolar->setStatusTip(tr("Import a polar from a text file"));
	connect(m_pImportWPolar, SIGNAL(triggered()), pMiarex, SLOT(onImportWPolar()));

	m_pPlaneInertia = new QAction(tr("Define Inertia")+"\tF12", this);
	m_pPlaneInertia->setStatusTip(tr("Define the inertia for the current plane or wing"));
	connect(m_pPlaneInertia, SIGNAL(triggered()), pMiarex, SLOT(onPlaneInertia()));

	m_pShowCurWOppOnly = new QAction(tr("Show Current OpPoint Only"), this);
	m_pShowCurWOppOnly->setStatusTip(tr("Hide all the curves except for the one corresponding to the currently selected operating point"));
	m_pShowCurWOppOnly->setCheckable(true);
	connect(m_pShowCurWOppOnly, SIGNAL(triggered()), pMiarex, SLOT(onCurWOppOnly()));

	m_pShowAllWOpps = new QAction(tr("Show All OpPoints"), this);
	m_pShowAllWOpps->setStatusTip(tr("Show the graph curves of all operating points"));
	connect(m_pShowAllWOpps, SIGNAL(triggered()), pMiarex, SLOT(onShowAllWOpps()));

	m_pHideAllWOpps = new QAction(tr("Hide All OpPoints"), this);
	m_pHideAllWOpps->setStatusTip(tr("Hide the graph curves of all operating points"));
	connect(m_pHideAllWOpps, SIGNAL(triggered()), pMiarex, SLOT(onHideAllWOpps()));

	m_pDeleteAllWOpps = new QAction(tr("Delete All OpPoints"), this);
	m_pDeleteAllWOpps->setStatusTip(tr("Delete all the operating points of all planes and polars"));
	connect(m_pDeleteAllWOpps, SIGNAL(triggered()), pMiarex, SLOT(onDeleteAllWOpps()));

	m_pShowAllWPlrOpps = new QAction(tr("Show Associated OpPoints"), this);
	m_pShowAllWPlrOpps->setStatusTip(tr("Show the curves of all the operating points of the currently selected polar"));
	connect(m_pShowAllWPlrOpps, SIGNAL(triggered()), pMiarex, SLOT(onShowAllWPlrOpps()));
	
	m_pHideAllWPlrOpps = new QAction(tr("Hide Associated OpPoints"), this);
	m_pHideAllWPlrOpps->setStatusTip(tr("Hide the curves of all the operating points of the currently selected polar"));
	connect(m_pHideAllWPlrOpps, SIGNAL(triggered()), pMiarex, SLOT(onHideAllWPlrOpps()));
	
	m_pDeleteAllWPlrOpps = new QAction(tr("Delete Associated OpPoints"), this);
	m_pDeleteAllWPlrOpps->setStatusTip(tr("Delete all the operating points of the currently selected polar"));
	connect(m_pDeleteAllWPlrOpps, SIGNAL(triggered()), pMiarex, SLOT(onDeleteAllWPlrOpps()));
	m_pShowTargetCurve = new QAction(tr("Show Target Curve"), this);
	m_pShowTargetCurve->setCheckable(false);
	connect(m_pShowTargetCurve, SIGNAL(triggered()), pMiarex, SLOT(onShowTargetCurve()));

	m_pShowXCmRefLocation = new QAction(tr("Show XCG location"), this);
	m_pShowXCmRefLocation->setStatusTip(tr("Show the position of the center of gravity defined in the analysis"));
	m_pShowXCmRefLocation->setCheckable(true);
	connect(m_pShowXCmRefLocation, SIGNAL(triggered()), pMiarex, SLOT(onShowXCmRef()));

	m_pShowStabCurve = new QAction(tr("Show Elevator Curve"), this);
	m_pShowStabCurve->setStatusTip(tr("Show the graph curves for the elevator"));
	m_pShowStabCurve->setCheckable(true);
	connect(m_pShowStabCurve, SIGNAL(triggered()), pMiarex, SLOT(onStabCurve()));

	m_pShowFinCurve = new QAction(tr("Show Fin Curve"), this);
	m_pShowFinCurve->setStatusTip(tr("Show the graph curves for the fin"));
	m_pShowFinCurve->setCheckable(true);
	connect(m_pShowFinCurve, SIGNAL(triggered()), pMiarex, SLOT(onFinCurve()));

	m_pShowWing2Curve = new QAction(tr("Show Second Wing Curve"), this);
	m_pShowWing2Curve->setStatusTip(tr("Show the graph curves for the second wing"));
	m_pShowWing2Curve->setCheckable(true);
	connect(m_pShowWing2Curve, SIGNAL(triggered()), pMiarex, SLOT(onWing2Curve()));

	m_pDefineWPolar = new QAction(tr("Define an Analysis")+" \t(F6)", this);
	m_pDefineWPolar->setStatusTip(tr("Define an analysis for the current wing or plane"));
	connect(m_pDefineWPolar, SIGNAL(triggered()), pMiarex, SLOT(onDefineWPolar()));

	m_pDefineWPolarObjectAct = new QAction(tr("Define an Analysis (advanced users)")+" \t(Ctrl+F6)", this);
	m_pDefineWPolarObjectAct->setStatusTip(tr("Shows a form to edit a new polar object"));
	connect(m_pDefineWPolarObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onDefineWPolarObject()));

	m_pEditWPolarAct = new QAction(tr("Edit..."), this);
	m_pEditWPolarAct->setStatusTip(tr("Modify the analysis parameters of this polar"));
	connect(m_pEditWPolarAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWPolar()));

	m_pEditWPolarObjectAct = new QAction(tr("Edit object (advanced users)"), this);
	m_pEditWPolarObjectAct->setStatusTip(tr("Shows a form to edit the currently selected polar"));
	connect(m_pEditWPolarObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWPolarObject()));

	m_pEditWPolarPts = new QAction(tr("Edit data points..."), this);
	m_pEditWPolarPts->setStatusTip(tr("Modify the data points of this polar"));
	connect(m_pEditWPolarPts, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWPolarPts()));

	m_pDefineStabPolar = new QAction(tr("Define a Stability Analysis")+"\t(Shift+F6)", this);
	m_pDefineStabPolar->setStatusTip(tr("Define a stability analysis for the current wing or plane"));
	connect(m_pDefineStabPolar, SIGNAL(triggered()), pMiarex, SLOT(onDefineStabPolar()));

	m_pHidePlaneWPlrs = new QAction(tr("Hide Associated Polars"), this);
	m_pHidePlaneWPlrs->setStatusTip(tr("Hide all the polar curves associated to the currently selected wing or plane"));
	connect(m_pHidePlaneWPlrs, SIGNAL(triggered()), pMiarex, SLOT(onHidePlaneWPolars()));
	
	m_pShowPlaneWPlrsOnly = new QAction(tr("Show Only Associated Polars"), this);
	connect(m_pShowPlaneWPlrsOnly, SIGNAL(triggered()), pMiarex, SLOT(onShowPlaneWPolarsOnly()));

	m_pShowPlaneWPlrs = new QAction(tr("Show Associated Polars"), this);
	m_pShowPlaneWPlrs->setStatusTip(tr("Show all the polar curves associated to the currently selected wing or plane"));
	connect(m_pShowPlaneWPlrs, SIGNAL(triggered()), pMiarex, SLOT(onShowPlaneWPolars()));

	m_pDeletePlaneWPlrs = new QAction(tr("Delete Associated Polars"), this);
	m_pDeletePlaneWPlrs->setStatusTip(tr("Delete all the polars associated to the currently selected wing or plane"));
	connect(m_pDeletePlaneWPlrs, SIGNAL(triggered()), pMiarex, SLOT(onDeletePlaneWPolars()));

	m_pHideAllWPlrs = new QAction(tr("Hide All Polars"), this);
	m_pHideAllWPlrs->setStatusTip(tr("Hide all the polar curves of all wings and planes"));
	connect(m_pHideAllWPlrs, SIGNAL(triggered()), pMiarex, SLOT(onHideAllWPolars()));
	
	m_pShowAllWPlrs = new QAction(tr("Show All Polars"), this);
	m_pShowAllWPlrs->setStatusTip(tr("Show all the polar curves of all wings and planes"));
	connect(m_pShowAllWPlrs, SIGNAL(triggered()), pMiarex, SLOT(onShowAllWPolars()));

	m_pHidePlaneWOpps = new QAction(tr("Hide Associated OpPoints"), this);
	m_pHidePlaneWOpps->setStatusTip(tr("Hide all the operating point curves of the currently selected wing or plane"));
	connect(m_pHidePlaneWOpps, SIGNAL(triggered()), pMiarex, SLOT(onHidePlaneOpps()));
	
	m_pShowPlaneWOpps = new QAction(tr("Show Associated OpPoints"), this);
	m_pShowPlaneWOpps->setStatusTip(tr("Show all the operating point curves of the currently selected wing or plane"));
	connect(m_pShowPlaneWOpps, SIGNAL(triggered()), pMiarex, SLOT(onShowPlaneOpps()));
	
	m_pDeletePlaneWOpps = new QAction(tr("Delete Associated OpPoints"), this);
	m_pDeletePlaneWOpps->setStatusTip(tr("Delete all the operating points of the currently selected wing or plane"));
	connect(m_pDeletePlaneWOpps, SIGNAL(triggered()), pMiarex, SLOT(onDeletePlanePOpps()));

	m_pDeleteCurPlane = new QAction(tr("Delete..."), this);
	m_pDeleteCurPlane->setStatusTip(tr("Delete the currently selected wing or plane"));
	connect(m_pDeleteCurPlane, SIGNAL(triggered()), pMiarex, SLOT(onDeleteCurPlane()));

	m_pDuplicateCurPlane = new QAction(tr("Duplicate..."), this);
	m_pDuplicateCurPlane->setStatusTip(tr("Duplicate the currently selected wing or plane"));
	connect(m_pDuplicateCurPlane, SIGNAL(triggered()), pMiarex, SLOT(onDuplicateCurPlane()));

	m_pSavePlaneAsProjectAct = new QAction(tr("Save as Project..."), this);
	m_pSavePlaneAsProjectAct->setStatusTip(tr("Save the currently selected wing or plane as a new separate project"));
	connect(m_pSavePlaneAsProjectAct, SIGNAL(triggered()), this, SLOT(onSavePlaneAsProject()));

	m_pRrenameCurWPolar = new QAction(tr("Rename..."), this);
	m_pRrenameCurWPolar->setStatusTip(tr("Rename the currently selected polar"));
	connect(m_pRrenameCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(onRenameCurWPolar()));

	m_pExportCurWPolar = new QAction(tr("Export results"), this);
	m_pExportCurWPolar->setStatusTip(tr("Export the currently selected polar to a text or csv file"));
	connect(m_pExportCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(onExportCurWPolar()));

	m_pResetCurWPolar = new QAction(tr("Reset ..."), this);
	m_pResetCurWPolar->setStatusTip(tr("Delete all the points of the currently selected polar, but keep the analysis settings"));
	connect(m_pResetCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(onResetCurWPolar()));

	m_pDeleteCurWPolar = new QAction(tr("Delete ..."), this);
	m_pDeleteCurWPolar->setStatusTip(tr("Delete the currently selected polar"));
	connect(m_pDeleteCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(onDeleteCurWPolar()));

	m_pDeleteCurWOpp = new QAction(tr("Delete..."), this);
	m_pDeleteCurWOpp->setStatusTip(tr("Delete the currently selected operating point"));
	connect(m_pDeleteCurWOpp, SIGNAL(triggered()), pMiarex, SLOT(onDeleteCurWOpp()));

	m_pAadvancedSettings = new QAction(tr("Advanced Settings..."), this);
	m_pAadvancedSettings->setStatusTip(tr("Define the settings for LLT, VLM and Panel analysis"));
	connect(m_pAadvancedSettings, SIGNAL(triggered()), pMiarex, SLOT(onAdvancedSettings()));

	m_pShowPolarProps = new QAction(tr("Properties"), this);
	m_pShowPolarProps->setStatusTip(tr("Show the properties of the currently selected polar"));
	m_pShowPolarProps->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
	connect(m_pShowPolarProps, SIGNAL(triggered()), pMiarex, SLOT(onWPolarProperties()));

	m_pShowWOppProps = new QAction(tr("Properties"), this);
	m_pShowWOppProps->setStatusTip(tr("Show the properties of the currently selected operating point"));
	m_pShowWOppProps->setShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Return));
	connect(m_pShowWOppProps, SIGNAL(triggered()), pMiarex, SLOT(onPlaneOppProperties()));

	m_pExportPlaneToXML = new QAction(tr("Export to xml file"), this);
	connect(m_pExportPlaneToXML, SIGNAL(triggered()), pMiarex, SLOT(onExportPlanetoXML()));

	m_pImportPlaneFromXml= new QAction(tr("Import plane from xml file"), this);
	connect(m_pImportPlaneFromXml, SIGNAL(triggered()), pMiarex, SLOT(onImportPlanefromXML()));

	m_pExportWPolarToXML = new QAction(tr("Export analysis to xml file"), this);
	connect(m_pExportWPolarToXML, SIGNAL(triggered()), pMiarex, SLOT(onExportWPolarToXML()));

	m_pImportWPolarFromXml= new QAction(tr("Import analysis from xml file"), this);
	connect(m_pImportWPolarFromXml, SIGNAL(triggered()), pMiarex, SLOT(onImportWPolarFromXML()));

}


void MainFrame::createMiarexMenus()
{
	//MainMenu for Miarex Application
	m_pMiarexViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pMiarexViewMenu->addAction(m_pWOppAct);
		m_pMiarexViewMenu->addAction(m_pWPolarAct);
		m_pMiarexViewMenu->addAction(m_pW3DAct);
		m_pMiarexViewMenu->addAction(m_pCpViewAct);
		m_pMiarexViewMenu->addAction(m_pStabTimeAct);
		m_pMiarexViewMenu->addAction(m_pRootLocusAct);
		m_pMiarexViewMenu->addSeparator();
		m_pMiarexViewMenu->addAction(m_pW3DPrefsAct);
		m_pMiarexViewMenu->addAction(m_pW3DLightAct);
		m_pMiarexViewMenu->addAction(m_pW3DScalesAct);
		m_pMiarexViewMenu->addSeparator();
		m_pMiarexViewMenu->addAction(m_pSaveViewToImageFileAct);
		m_pMiarexViewMenu->addSeparator();
		m_pMiarexViewMenu->addAction(m_pStyleAct);
	}


	m_pPlaneMenu = menuBar()->addMenu(tr("&Plane"));
	{
		m_pPlaneMenu->addAction(m_pDefinePlaneAct);
		m_pPlaneMenu->addAction(m_pDefinePlaneObjectAct);
		m_pPlaneMenu->addAction(m_pManagePlanesAct);
		m_pCurrentPlaneMenu = m_pPlaneMenu->addMenu(tr("Current Plane"));
		{
			QMenu *pAnalysisMenu = m_pCurrentPlaneMenu->addMenu(tr("Analysis"));
			{
				pAnalysisMenu->addAction(m_pDefineWPolar);
				pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
				pAnalysisMenu->addAction(m_pDefineStabPolar);
			}
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pEditPlaneAct);
			m_pCurrentPlaneMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu->addAction(m_pEditBodyAct);
			m_pCurrentPlaneMenu->addAction(m_pEditBodyObjectAct);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pRenameCurPlaneAct);
			m_pCurrentPlaneMenu->addAction(m_pDuplicateCurPlane);
			m_pCurrentPlaneMenu->addAction(m_pDeleteCurPlane);
			m_pCurrentPlaneMenu->addAction(m_pSavePlaneAsProjectAct);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pScaleWingAct);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pPlaneInertia);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pExporttoAVL);
			m_pCurrentPlaneMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu->addAction(m_pExportPlaneToXML);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pShowPlaneWPlrsOnly);
			m_pCurrentPlaneMenu->addAction(m_pShowPlaneWPlrs);
			m_pCurrentPlaneMenu->addAction(m_pHidePlaneWPlrs);
			m_pCurrentPlaneMenu->addAction(m_pDeletePlaneWPlrs);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pHidePlaneWOpps);
			m_pCurrentPlaneMenu->addAction(m_pShowPlaneWOpps);
			m_pCurrentPlaneMenu->addAction(m_pDeletePlaneWOpps);
		}
		m_pPlaneMenu->addAction(m_pImportPlaneFromXml);
	}

	m_pMiarexWPlrMenu = menuBar()->addMenu(tr("&Polars"));
	{
		m_pCurWPlrMenu = m_pMiarexWPlrMenu->addMenu(tr("Current Polar"));
		{
			m_pCurWPlrMenu->addAction(m_pShowPolarProps);
			m_pCurWPlrMenu->addAction(m_pEditWPolarAct);
			m_pCurWPlrMenu->addAction(m_pEditWPolarObjectAct);
			m_pCurWPlrMenu->addAction(m_pEditWPolarPts);
			m_pCurWPlrMenu->addAction(m_pRrenameCurWPolar);
			m_pCurWPlrMenu->addAction(m_pDeleteCurWPolar);
			m_pCurWPlrMenu->addAction(m_pResetCurWPolar);
			m_pCurWPlrMenu->addSeparator();
			m_pCurWPlrMenu->addAction(m_pShowAllWPlrOpps);
			m_pCurWPlrMenu->addAction(m_pHideAllWPlrOpps);
			m_pCurWPlrMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu->addSeparator();
			m_pCurWPlrMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu->addAction(m_pExportWPolarToXML);
		}

		m_pMiarexWPlrMenu->addSeparator();
		m_pMiarexWPlrMenu->addAction(m_pImportWPolar);
		m_pMiarexWPlrMenu->addSeparator();
		m_pMiarexWPlrMenu->addAction(m_pMiarexPolarFilter);
		m_pMiarexWPlrMenu->addSeparator();
		m_pMiarexWPlrMenu->addAction(m_pHideAllWPlrs);
		m_pMiarexWPlrMenu->addAction(m_pShowAllWPlrs);
		m_pMiarexWPlrMenu->addSeparator();
	}

	m_pMiarexWOppMenu = menuBar()->addMenu(tr("&OpPoint"));
	{
		m_pCurWOppMenu = m_pMiarexWOppMenu->addMenu(tr("Current OpPoint"));
		{
			m_pCurWOppMenu->addAction(m_pShowWOppProps);
			m_pCurWOppMenu->addAction(m_pExportCurWOpp);
			m_pCurWOppMenu->addAction(m_pDeleteCurWOpp);
		}
		m_pMiarexWOppMenu->addSeparator();
		m_pMiarexWOppMenu->addAction(m_pShowCurWOppOnly);
		m_pMiarexWOppMenu->addAction(m_pShowAllWOpps);
		m_pMiarexWOppMenu->addAction(m_pHideAllWOpps);
		m_pMiarexWOppMenu->addAction(m_pDeleteAllWOpps);
		m_pMiarexWOppMenu->addSeparator();
		m_pMiarexWOppMenu->addAction(m_pShowTargetCurve);
		m_pMiarexWOppMenu->addAction(m_pShowXCmRefLocation);
		m_pMiarexWOppMenu->addAction(m_pShowWing2Curve);
		m_pMiarexWOppMenu->addAction(m_pShowStabCurve);
		m_pMiarexWOppMenu->addAction(m_pShowFinCurve);
		m_pMiarexWOppMenu->addAction(m_pShowFlapMoments);
	}

	//Miarex Analysis Menu
	m_pMiarexAnalysisMenu  = menuBar()->addMenu(tr("&Analysis"));
	{
		m_pMiarexAnalysisMenu->addAction(m_pDefineWPolar);
		m_pMiarexAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
		m_pMiarexAnalysisMenu->addAction(m_pDefineStabPolar);
		m_pMiarexAnalysisMenu->addAction(m_pImportWPolarFromXml);
		m_pMiarexAnalysisMenu->addSeparator();
		m_pMiarexAnalysisMenu->addAction(m_pViewLogFile);
		m_pMiarexAnalysisMenu->addAction(m_pAadvancedSettings);
	}


	//WOpp View Context Menu
	m_pWOppCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentPlaneMenu_WOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WOppCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WOppCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WOppCtxMenu->addAction(m_pDeletePlaneWOpps);
        }
        //m_pWOppCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWOppCtxMenu->addSeparator();
        m_pCurWPlrMenu_WOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current Polar"));
        {
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pShowPolarProps);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pEditWPolarAct);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pEditWPolarObjectAct);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pEditWPolarPts);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pRrenameCurWPolar);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pDeleteCurWPolar);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pResetCurWPolar);
			m_pCurWPlrMenu_WOppCtxMenu->addSeparator();
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pShowAllWPlrOpps);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pHideAllWPlrOpps);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu_WOppCtxMenu->addSeparator();
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu_WOppCtxMenu->addAction(m_pExportWPolarToXML);
		}

        //m_pWOppCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWOppCtxMenu->addSeparator();
        m_pCurWOppMenu_WOppCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_WOppCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_WOppCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_WOppCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pWOppCtxMenu->addMenu(m_pCurWOppMenu);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addAction(m_pShowCurWOppOnly);
		m_pWOppCtxMenu->addAction(m_pShowAllWOpps);
		m_pWOppCtxMenu->addAction(m_pHideAllWOpps);
		m_pWOppCtxMenu->addAction(m_pDeleteAllWOpps);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addAction(m_pShowTargetCurve);
		m_pWOppCtxMenu->addAction(m_pShowXCmRefLocation);
		m_pWOppCtxMenu->addAction(m_pShowWing2Curve);
		m_pWOppCtxMenu->addAction(m_pShowStabCurve);
		m_pWOppCtxMenu->addAction(m_pShowFinCurve);
		m_pWOppCtxMenu->addAction(m_pShowFlapMoments);
		m_pWOppCtxMenu->addSeparator();
		QMenu *pCurGraphCtxMenu = m_pWOppCtxMenu->addMenu(tr("Current Graph"));
		{
			pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
			pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
			pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
		}
		m_pWOppCtxMenu->addAction(m_pShowMousePosAct);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addAction(m_pViewLogFile);
		m_pWOppCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}

	//WOpp View Context Menu
	m_pWCpCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentPlaneMenu_WCpCtxMenu = m_pWCpCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WCpCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WCpCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WCpCtxMenu->addAction(m_pDeletePlaneWOpps);
        }
        //m_pWCpCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWCpCtxMenu->addSeparator();
        m_pCurWPlrMenu_WCpCtxMenu = m_pWCpCtxMenu->addMenu(tr("Current Polar"));
        {
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pShowPolarProps);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pEditWPolarAct);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pEditWPolarObjectAct);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pEditWPolarPts);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pRrenameCurWPolar);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pDeleteCurWPolar);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pResetCurWPolar);
			m_pCurWPlrMenu_WCpCtxMenu->addSeparator();
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pShowAllWPlrOpps);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pHideAllWPlrOpps);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu_WCpCtxMenu->addSeparator();
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu_WCpCtxMenu->addAction(m_pExportWPolarToXML);
		}

        //m_pWCpCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWCpCtxMenu->addSeparator();
        m_pCurWOppMenu_WCpCtxMenu = m_pWCpCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_WCpCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_WCpCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_WCpCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pWCpCtxMenu->addMenu(m_pCurWOppMenu);
		m_pWCpCtxMenu->addSeparator();
		m_pWCpCtxMenu->addAction(m_pShowWing2Curve);
		m_pWCpCtxMenu->addAction(m_pShowStabCurve);
		m_pWCpCtxMenu->addAction(m_pShowFinCurve);
		m_pWCpCtxMenu->addSeparator();
		m_pWCpCtxMenu->addAction(m_pViewLogFile);
		m_pWCpCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}

	//WTime View Context Menu
	m_pWTimeCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentPlaneMenu_WTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WTimeCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WTimeCtxMenu->addAction(m_pDeletePlaneWOpps);
        }

        //m_pWTimeCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWTimeCtxMenu->addSeparator();
        m_pCurWPlrMenu_WTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current Polar"));
        {
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pShowPolarProps);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pEditWPolarAct);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pEditWPolarObjectAct);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pEditWPolarPts);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pRrenameCurWPolar);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pDeleteCurWPolar);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pResetCurWPolar);
			m_pCurWPlrMenu_WTimeCtxMenu->addSeparator();
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pShowAllWPlrOpps);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pHideAllWPlrOpps);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu_WTimeCtxMenu->addSeparator();
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu_WTimeCtxMenu->addAction(m_pExportWPolarToXML);
		}
        //m_pWTimeCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWTimeCtxMenu->addSeparator();
        m_pCurWOppMenu_WTimeCtxMenu = m_pWTimeCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_WTimeCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_WTimeCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_WTimeCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pWTimeCtxMenu->addMenu(m_pCurWOppMenu);
		m_pWTimeCtxMenu->addSeparator();
		m_pWTimeCtxMenu->addAction(m_pShowCurWOppOnly);
		m_pWTimeCtxMenu->addAction(m_pShowAllWOpps);
		m_pWTimeCtxMenu->addAction(m_pHideAllWOpps);
		m_pWTimeCtxMenu->addAction(m_pDeleteAllWOpps);
		m_pWTimeCtxMenu->addSeparator();
		m_pWTimeCtxMenu->addAction(m_pViewLogFile);
		m_pWTimeCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}

	//Polar View Context Menu
	m_pWPlrCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentPlaneMenu_WPlrCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_WPlrCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_WPlrCtxMenu->addAction(m_pDeletePlaneWOpps);
        }

        //m_pWPlrCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWPlrCtxMenu->addSeparator();
        m_pCurWPlrMenu_WPlrCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pRrenameCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_WPlrCtxMenu->addSeparator();
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu_WPlrCtxMenu->addSeparator();
			m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu_WPlrCtxMenu->addAction(m_pExportWPolarToXML);
		}
        //m_pWPlrCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWPlrCtxMenu->addSeparator();
		m_pWPlrCtxMenu->addAction(m_pHideAllWPlrs);
		m_pWPlrCtxMenu->addAction(m_pShowAllWPlrs);
		m_pWPlrCtxMenu->addSeparator();
		QMenu *pCurGraphCtxMenu = m_pWPlrCtxMenu->addMenu(tr("Current Graph"));
		{
			pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
			pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
			pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
			pCurGraphCtxMenu->addAction(m_pHighlightOppAct);
		}
		m_pWPlrCtxMenu->addAction(m_pShowMousePosAct);
		m_pWPlrCtxMenu->addSeparator();
		m_pWPlrCtxMenu->addAction(m_pViewLogFile);
		m_pWPlrCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}

	//W3D View Context Menu
	m_pW3DCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentPlaneMenu_W3DCtxMenu = m_pW3DCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_W3DCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_W3DCtxMenu->addAction(m_pDeletePlaneWOpps);
        }

        //m_pW3DCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pW3DCtxMenu->addSeparator();
        m_pCurWPlrMenu_W3DCtxMenu = m_pW3DCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pRrenameCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_W3DCtxMenu->addSeparator();
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu_W3DCtxMenu->addSeparator();
			m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu_W3DCtxMenu->addAction(m_pExportWPolarToXML);
		}
        //m_pW3DCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pW3DCtxMenu->addSeparator();
        m_pCurWOppMenu_W3DCtxMenu = m_pW3DCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_W3DCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_W3DCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_W3DCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pW3DCtxMenu->addMenu(m_pCurWOppMenu);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addAction(m_pDeleteAllWOpps);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addAction(m_pReset3DScale);
		m_pW3DCtxMenu->addAction(m_pW3DScalesAct);
		m_pW3DCtxMenu->addAction(m_pW3DLightAct);
		m_pW3DCtxMenu->addAction(m_pShowFlapMoments);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addAction(m_pViewLogFile);
		m_pW3DCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}

	//W3D Stab View Context Menu
	m_pW3DStabCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentPlaneMenu_W3DStabCtxMenu = m_pW3DStabCtxMenu->addMenu(tr("Current Plane"));
        {
            QMenu *pAnalysisMenu = m_pCurrentPlaneMenu_W3DStabCtxMenu->addMenu(tr("Analysis"));
            {
                pAnalysisMenu->addAction(m_pDefineWPolar);
                pAnalysisMenu->addAction(m_pDefineWPolarObjectAct);
                pAnalysisMenu->addAction(m_pDefineStabPolar);
            }
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditPlaneAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditObjectAct);
			m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditWingAct);
			m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditStabAct);
			m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditFinAct);
			m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditBodyAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pEditBodyObjectAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pRenameCurPlaneAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDuplicateCurPlane);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDeleteCurPlane);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pSavePlaneAsProjectAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pScaleWingAct);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pPlaneInertia);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pExporttoAVL);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pExporttoSTL);
			m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pExportPlaneToXML);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pShowPlaneWPlrsOnly);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pShowPlaneWPlrs);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pHidePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDeletePlaneWPlrs);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addSeparator();
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pHidePlaneWOpps);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pShowPlaneWOpps);
            m_pCurrentPlaneMenu_W3DStabCtxMenu->addAction(m_pDeletePlaneWOpps);
        }
        //m_pW3DStabCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pW3DStabCtxMenu->addSeparator();
        m_pCurWPlrMenu_W3DStabCtxMenu = m_pW3DStabCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pShowPolarProps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pEditWPolarAct);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pEditWPolarObjectAct);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pEditWPolarPts);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pRrenameCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pDeleteCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pExportCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pResetCurWPolar);
            m_pCurWPlrMenu_W3DStabCtxMenu->addSeparator();
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pShowAllWPlrOpps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pHideAllWPlrOpps);
            m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pDeleteAllWPlrOpps);
			m_pCurWPlrMenu_W3DStabCtxMenu->addSeparator();
			m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pExportCurWPolar);
			m_pCurWPlrMenu_W3DStabCtxMenu->addAction(m_pExportWPolarToXML);
		}

        //m_pW3DStabCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pW3DStabCtxMenu->addSeparator();
        m_pCurWOppMenu_W3DStabCtxMenu = m_pW3DStabCtxMenu->addMenu(tr("Current OpPoint"));
        {
            m_pCurWOppMenu_W3DStabCtxMenu->addAction(m_pShowWOppProps);
            m_pCurWOppMenu_W3DStabCtxMenu->addAction(m_pExportCurWOpp);
            m_pCurWOppMenu_W3DStabCtxMenu->addAction(m_pDeleteCurWOpp);
        }
        //m_pW3DStabCtxMenu->addMenu(m_pCurWOppMenu);
		m_pW3DStabCtxMenu->addSeparator();
		m_pW3DStabCtxMenu->addAction(m_pReset3DScale);
		m_pW3DStabCtxMenu->addAction(m_pW3DScalesAct);
		m_pW3DStabCtxMenu->addAction(m_pW3DLightAct);
		m_pW3DStabCtxMenu->addAction(m_pShowFlapMoments);
		m_pW3DStabCtxMenu->addSeparator();
		m_pW3DStabCtxMenu->addAction(m_pViewLogFile);
		m_pW3DStabCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}
}


void MainFrame::createMiarexToolbar()
{
	m_pctrlPlane  = new QComboBox();
	QStyledItemDelegate *pPlaneDelegate = new QStyledItemDelegate(this);
	m_pctrlPlane->setItemDelegate(pPlaneDelegate); //necessary to support stylesheets
	m_pctrlPlanePolar = new QComboBox;
	QStyledItemDelegate *pPPolarDelegate = new QStyledItemDelegate(this);
	m_pctrlPlanePolar->setItemDelegate(pPPolarDelegate); //necessary to support stylesheets
	m_pctrlPlaneOpp   = new QComboBox;
	QStyledItemDelegate *pPOppDelegate = new QStyledItemDelegate(this);
	m_pctrlPlaneOpp->setItemDelegate(pPOppDelegate); //necessary to support stylesheets

	m_pctrlPlane->setFocusPolicy(Qt::NoFocus);// to override keypressevent and key_F4
	m_pctrlPlanePolar->setFocusPolicy(Qt::NoFocus);
	m_pctrlPlaneOpp->setFocusPolicy(Qt::NoFocus);


	m_pctrlPlane->setMinimumWidth(150);
	m_pctrlPlanePolar->setMinimumWidth(150);
	m_pctrlPlaneOpp->setMinimumWidth(80);
	m_pctrlPlane->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPlanePolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPlaneOpp->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPlane->setMaxVisibleItems(23);
	m_pctrlPlanePolar->setMaxVisibleItems(23);
	m_pctrlPlaneOpp->setMaxVisibleItems(23);

	m_pctrlMiarexToolBar = addToolBar(tr("Plane"));
	m_pctrlMiarexToolBar->addAction(m_pNewProjectAct);
	m_pctrlMiarexToolBar->addAction(m_pOpenAct);
	m_pctrlMiarexToolBar->addAction(m_pSaveAct);
	m_pctrlMiarexToolBar->addSeparator();
	m_pctrlMiarexToolBar->addAction(m_pWOppAct);
	m_pctrlMiarexToolBar->addAction(m_pWPolarAct);
	m_pctrlMiarexToolBar->addAction(m_pW3DAct);
	m_pctrlMiarexToolBar->addAction(m_pCpViewAct);
	m_pctrlMiarexToolBar->addAction(m_pRootLocusAct);
	m_pctrlMiarexToolBar->addAction(m_pStabTimeAct);

	m_pctrlMiarexToolBar->addSeparator();
	m_pctrlMiarexToolBar->addWidget(m_pctrlPlane);
	m_pctrlMiarexToolBar->addWidget(m_pctrlPlanePolar);
	m_pctrlMiarexToolBar->addWidget(m_pctrlPlaneOpp);

	connect(m_pctrlPlane,      SIGNAL(activated(int)), this, SLOT(onSelChangePlane(int)));
	connect(m_pctrlPlanePolar, SIGNAL(activated(int)), this, SLOT(onSelChangeWPolar(int)));
	connect(m_pctrlPlaneOpp,   SIGNAL(activated(int)), this, SLOT(onSelChangePlaneOpp(int)));
}


void MainFrame::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
	m_pctrlProjectName = new QLabel(" ");
	m_pctrlProjectName->setMinimumWidth(200);
	statusBar()->addPermanentWidget(m_pctrlProjectName);
}


void MainFrame::createToolbars()
{
	createXDirectToolbar();
	createXInverseToolbar();
	createMiarexToolbar();
	createAFoilToolbar();
}


void MainFrame::createXDirectToolbar()
{
	m_pctrlFoil    = new QComboBox;
	m_pctrlPolar   = new QComboBox;
	m_pctrlOpPoint = new QComboBox;

	QStyledItemDelegate *pFoilDelegate = new QStyledItemDelegate(this);
	m_pctrlFoil->setItemDelegate(pFoilDelegate); //necessary to support stylesheets

	QStyledItemDelegate *pPolarDelegate = new QStyledItemDelegate(this);
	m_pctrlPolar->setItemDelegate(pPolarDelegate); //necessary to support stylesheets

	QStyledItemDelegate *pOppDelegate = new QStyledItemDelegate(this);
	m_pctrlOpPoint->setItemDelegate(pOppDelegate); //necessary to support stylesheets

	m_pctrlFoil->setMaxVisibleItems(23);
	m_pctrlPolar->setMaxVisibleItems(23);
	m_pctrlOpPoint->setMaxVisibleItems(23);
	m_pctrlFoil->setMinimumWidth(150);
	m_pctrlPolar->setMinimumWidth(150);
	m_pctrlOpPoint->setMinimumWidth(80);
	m_pctrlFoil->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlOpPoint->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	m_pctrlXDirectToolBar = addToolBar(tr("Foil"));
	m_pctrlXDirectToolBar->addAction(m_pNewProjectAct);
	m_pctrlXDirectToolBar->addAction(m_pOpenAct);
	m_pctrlXDirectToolBar->addAction(m_pSaveAct);
	m_pctrlXDirectToolBar->addSeparator();
	m_pctrlXDirectToolBar->addAction(m_pOpPointsAct);
	m_pctrlXDirectToolBar->addAction(m_pPolarsAct);
	m_pctrlXDirectToolBar->addSeparator();
	m_pctrlXDirectToolBar->addWidget(m_pctrlFoil);
	m_pctrlXDirectToolBar->addWidget(m_pctrlPolar);
	m_pctrlXDirectToolBar->addWidget(m_pctrlOpPoint);

	connect(m_pctrlFoil,    SIGNAL(activated(int)), this, SLOT(onSelChangeFoil(int)));
	connect(m_pctrlPolar,   SIGNAL(activated(int)), this, SLOT(onSelChangePolar(int)));
	connect(m_pctrlOpPoint, SIGNAL(activated(int)), this, SLOT(onSelChangeOpp(int)));
}




void MainFrame::createXDirectActions()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;

	m_pOpPointsAct = new QAction(QIcon(":/images/OnCpView.png"), tr("OpPoint view")+"\tF5", this);
	m_pOpPointsAct->setCheckable(true);
	m_pOpPointsAct->setStatusTip(tr("Show Operating point view"));
	connect(m_pOpPointsAct, SIGNAL(triggered()), pXDirect, SLOT(onOpPointView()));

	m_pPolarsAct = new QAction(QIcon(":/images/OnPolarView.png"), tr("Polar view")+"\tF8", this);
	m_pPolarsAct->setCheckable(true);
	m_pPolarsAct->setStatusTip(tr("Show Polar view"));
	connect(m_pPolarsAct, SIGNAL(triggered()), pXDirect, SLOT(onPolarView()));

	m_pXDirectPolarFilter = new QAction(tr("Polar Filter"), this);
	connect(m_pXDirectPolarFilter, SIGNAL(triggered()), pXDirect, SLOT(onPolarFilter()));

	m_pHighlightOppAct	 = new QAction(tr("Highlight Current OpPoint")+"\t(Ctrl+H)", this);
	m_pHighlightOppAct->setCheckable(true);
	m_pHighlightOppAct->setStatusTip(tr("Highlights on the polar curve the currently selected operating point"));
	connect(m_pHighlightOppAct, SIGNAL(triggered()), this, SLOT(onHighlightOperatingPoint()));

	m_pDeleteCurFoil = new QAction(tr("Delete..."), this);
	connect(m_pDeleteCurFoil, SIGNAL(triggered()), pXDirect, SLOT(onDeleteCurFoil()));

	m_pRenameCurFoil = new QAction(tr("Rename...")+"\tF2", this);
	connect(m_pRenameCurFoil, SIGNAL(triggered()), pXDirect, SLOT(onRenameCurFoil()));

	m_pExportCurFoil = new QAction(tr("Export..."), this);
	connect(m_pExportCurFoil, SIGNAL(triggered()), pXDirect, SLOT(onExportCurFoil()));

	m_pDirectDuplicateCurFoil = new QAction(tr("Duplicate..."), this);
	connect(m_pDirectDuplicateCurFoil, SIGNAL(triggered()), pXDirect, SLOT(onDuplicateFoil()));

	m_pSetCurFoilStyle = new QAction(tr("Set Style..."), this);
	connect(m_pSetCurFoilStyle, SIGNAL(triggered()), this, SLOT(onCurFoilStyle()));

	m_pDeleteFoilPolars = new QAction(tr("Delete associated polars"), this);
	m_pDeleteFoilPolars->setStatusTip(tr("Delete all the polars associated to this foil"));
	connect(m_pDeleteFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(onDeleteFoilPolars()));

	m_pShowFoilPolarsOnly = new QAction(tr("Show only associated polars"), this);
	connect(m_pShowFoilPolarsOnly, SIGNAL(triggered()), pXDirect, SLOT(onShowFoilPolarsOnly()));

	m_pShowFoilPolars = new QAction(tr("Show associated polars"), this);
	connect(m_pShowFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(onShowFoilPolars()));

	m_pHideFoilPolars = new QAction(tr("Hide associated polars"), this);
	connect(m_pHideFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(onHideFoilPolars()));

	m_pSaveFoilPolars = new QAction(tr("Save associated polars"), this);
	connect(m_pSaveFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(onSavePolars()));

	m_pHidePolarOpps = new QAction(tr("Hide associated OpPoints"), this);
	connect(m_pHidePolarOpps, SIGNAL(triggered()), pXDirect, SLOT(onHidePolarOpps()));

	m_pShowPolarOpps = new QAction(tr("Show associated OpPoints"), this);
	connect(m_pShowPolarOpps, SIGNAL(triggered()), pXDirect, SLOT(onShowPolarOpps()));

	m_pDeletePolarOpps = new QAction(tr("Delete associated OpPoints"), this);
	connect(m_pDeletePolarOpps, SIGNAL(triggered()), pXDirect, SLOT(onDeletePolarOpps()));

	m_pExportPolarOpps = new QAction(tr("Export associated OpPoints"), this);
	connect(m_pExportPolarOpps, SIGNAL(triggered()), pXDirect, SLOT(onExportPolarOpps()));

	m_pHideFoilOpps = new QAction(tr("Hide associated OpPoints"), this);
	connect(m_pHideFoilOpps, SIGNAL(triggered()), pXDirect, SLOT(onHideFoilOpps()));

	m_pShowFoilOpps = new QAction(tr("Show associated OpPoints"), this);
	connect(m_pShowFoilOpps, SIGNAL(triggered()), pXDirect, SLOT(onShowFoilOpps()));

	m_pDeleteFoilOpps = new QAction(tr("Delete associated OpPoints"), this);
	connect(m_pDeleteFoilOpps, SIGNAL(triggered()), pXDirect, SLOT(onDeleteFoilOpps()));

	m_pDefinePolarAct = new QAction(tr("Define an Analysis")+"\tF6", this);
	m_pDefinePolarAct->setStatusTip(tr("Defines a single analysis/polar"));
	connect(m_pDefinePolarAct, SIGNAL(triggered()), pXDirect, SLOT(onDefinePolar()));

	m_pBatchAnalysisAct = new QAction(tr("Batch Analysis")+"\tShift+F6", this);
	m_pBatchAnalysisAct->setStatusTip(tr("Launches a batch of analysis calculation for a specified range or list of Reynolds numbers"));
	connect(m_pBatchAnalysisAct, SIGNAL(triggered()), pXDirect, SLOT(onBatchAnalysis()));

	m_pMultiThreadedBatchAct = new QAction(tr("Multi-threaded Batch Analysis")+"\tCtrl+F6", this);
	m_pMultiThreadedBatchAct->setStatusTip(tr("Launches a batch of analysis calculation using all available computer CPU cores"));
	connect(m_pMultiThreadedBatchAct, SIGNAL(triggered()), pXDirect, SLOT(onMultiThreadedBatchAnalysis()));

	m_pDeletePolar = new QAction(tr("Delete"), this);
	m_pDeletePolar->setStatusTip(tr("Deletes the currently selected polar"));
	connect(m_pDeletePolar, SIGNAL(triggered()), pXDirect, SLOT(onDeleteCurPolar()));

	m_pResetCurPolar = new QAction(tr("Reset"), this);
	m_pResetCurPolar->setStatusTip(tr("Deletes the contents of the currently selected polar"));
	connect(m_pResetCurPolar, SIGNAL(triggered()), pXDirect, SLOT(onResetCurPolar()));

	m_pEditCurPolar = new QAction(tr("Edit"), this);
	m_pEditCurPolar->setStatusTip(tr("Remove the unconverged or erroneaous points of the currently selected polar"));
	connect(m_pEditCurPolar, SIGNAL(triggered()), pXDirect, SLOT(onEditCurPolar()));

	m_pExportCurPolar = new QAction(tr("Export"), this);
	connect(m_pExportCurPolar, SIGNAL(triggered()), pXDirect, SLOT(onExportCurPolar()));

	m_pExportAllPolars = new QAction(tr("Export all polars"), this);
	connect(m_pExportAllPolars, SIGNAL(triggered()), pXDirect, SLOT(onExportAllPolars()));

	m_pXDirectStyleAct = new QAction(tr("Define Styles"), this);
	m_pXDirectStyleAct->setStatusTip(tr("Define the style for the boundary layer and the pressure arrows"));

	m_pShowNeutralLine = new QAction(tr("Neutral Line"), this);
	m_pShowNeutralLine->setCheckable(true);

	m_pShowPanels = new QAction(tr("Show Panels"), this);
	m_pShowPanels->setCheckable(true);
	m_pShowPanels->setStatusTip(tr("Show the foil's panels"));

	m_pResetFoilScale = new QAction(tr("Reset Foil Scale"), this);
	m_pResetFoilScale->setStatusTip(tr("Resets the foil's scale to original size"));

	m_pManageFoilsAct = new QAction(tr("Manage Foils"), this);
	m_pManageFoilsAct->setShortcut(Qt::Key_F7);
	connect(m_pManageFoilsAct, SIGNAL(triggered()), this, SLOT(onManageFoils()));

	m_pRenamePolarAct = new QAction(tr("Rename"), this);
	connect(m_pRenamePolarAct, SIGNAL(triggered()), pXDirect, SLOT(onRenamePolar()));


	m_pShowInviscidCurve = new QAction(tr("Show Inviscid Curve"), this);
	m_pShowInviscidCurve->setCheckable(true);
	m_pShowInviscidCurve->setStatusTip(tr("Display the Opp's inviscid curve"));
	connect(m_pShowInviscidCurve, SIGNAL(triggered()), pXDirect, SLOT(onCpi()));


	m_pShowAllPolars = new QAction(tr("Show All Polars"), this);
	connect(m_pShowAllPolars, SIGNAL(triggered()), pXDirect, SLOT(onShowAllPolars()));

	m_pHideAllPolars = new QAction(tr("Hide All Polars"), this);
	connect(m_pHideAllPolars, SIGNAL(triggered()), pXDirect, SLOT(onHideAllPolars()));

	m_pShowCurOppOnly = new QAction(tr("Show Current Opp Only"), this);
	m_pShowCurOppOnly->setCheckable(true);
	connect(m_pShowCurOppOnly, SIGNAL(triggered()), pXDirect, SLOT(onCurOppOnly()));

	m_pShowAllOpPoints = new QAction(tr("Show All Opps"), this);
	connect(m_pShowAllOpPoints, SIGNAL(triggered()), pXDirect, SLOT(onShowAllOpps()));

	m_pHideAllOpPoints = new QAction(tr("Hide All Opps"), this);
	connect(m_pHideAllOpPoints, SIGNAL(triggered()), pXDirect, SLOT(onHideAllOpps()));

	m_pExportCurOpp = new QAction(tr("Export"), this);
	connect(m_pExportCurOpp, SIGNAL(triggered()), pXDirect, SLOT(onExportCurOpp()));

	m_pDeleteCurOpp = new QAction(tr("Delete"), this);
	connect(m_pDeleteCurOpp, SIGNAL(triggered()), pXDirect, SLOT(onDelCurOpp()));

	m_pGetOppProps = new QAction(tr("Properties"), this);
	connect(m_pGetOppProps, SIGNAL(triggered()), pXDirect, SLOT(onOpPointProps()));

	m_pGetPolarProps = new QAction(tr("Properties"), this);
	connect(m_pGetPolarProps, SIGNAL(triggered()), pXDirect, SLOT(onPolarProps()));

	m_pViewXFoilAdvanced = new QAction(tr("XFoil Advanced Settings"), this);
	m_pBatchAnalysisAct->setStatusTip(tr("Tip : you don't want to use that option..."));
	connect(m_pViewXFoilAdvanced, SIGNAL(triggered()), pXDirect, SLOT(onXFoilAdvanced()));
 
	m_pViewLogFile = new QAction(tr("View Log File")+"\t(L)", this);
	connect(m_pViewLogFile, SIGNAL(triggered()), this, SLOT(onLogFile()));

	m_pDerotateFoil = new QAction(tr("De-rotate the Foil"), this);
	connect(m_pDerotateFoil, SIGNAL(triggered()), pXDirect, SLOT(onDerotateFoil()));

	m_pNormalizeFoil = new QAction(tr("Normalize the Foil"), this);
	connect(m_pNormalizeFoil, SIGNAL(triggered()), pXDirect, SLOT(onNormalizeFoil()));

	m_pRefineLocalFoil = new QAction(tr("Refine Locally")+"\t(Shift+F3)", this);
	connect(m_pRefineLocalFoil, SIGNAL(triggered()), pXDirect, SLOT(onCadd()));

	m_pRefineGlobalFoil = new QAction(tr("Refine Globally")+"\t(F3)", this);
	connect(m_pRefineGlobalFoil, SIGNAL(triggered()), pXDirect, SLOT(onRefinePanelsGlobally()));

	m_pEditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
	connect(m_pEditCoordsFoil, SIGNAL(triggered()), pXDirect, SLOT(onFoilCoordinates()));

	m_pScaleFoil = new QAction(tr("Scale camber and thickness")+"\t(F9)", this);
	connect(m_pScaleFoil, SIGNAL(triggered()), pXDirect, SLOT(onFoilGeom()));

	m_pSetTEGap = new QAction(tr("Set T.E. Gap"), this);
	connect(m_pSetTEGap, SIGNAL(triggered()), pXDirect, SLOT(onSetTEGap()));

	m_pSetLERadius = new QAction(tr("Set L.E. Radius"), this);
	connect(m_pSetLERadius, SIGNAL(triggered()), pXDirect, SLOT(onSetLERadius()));

	m_pSetFlap = new QAction(tr("Set Flap")+"\t(F10)", this);
	connect(m_pSetFlap, SIGNAL(triggered()), pXDirect, SLOT(onSetFlap()));

	m_pInterpolateFoils = new QAction(tr("Interpolate Foils")+"\t(F11)", this);
	connect(m_pInterpolateFoils, SIGNAL(triggered()), pXDirect, SLOT(onInterpolateFoils()));

	m_pNacaFoils = new QAction(tr("Naca Foils"), this);
	connect(m_pNacaFoils, SIGNAL(triggered()), pXDirect, SLOT(onNacaFoils()));

	m_psetCpVarGraph = new QAction(tr("Cp Variable"), this);
	m_psetCpVarGraph->setCheckable(true);
	m_psetCpVarGraph->setStatusTip(tr("Sets Cp vs. chord graph"));
	connect(m_psetCpVarGraph, SIGNAL(triggered()), pXDirect, SLOT(onCpGraph()));

	m_psetQVarGraph = new QAction(tr("Q Variable"), this);
	m_psetQVarGraph->setCheckable(true);
	m_psetQVarGraph->setStatusTip(tr("Sets Speed vs. chord graph"));
	connect(m_psetQVarGraph, SIGNAL(triggered()), pXDirect, SLOT(onQGraph()));

	m_pExportCurXFoilRes = new QAction(tr("Export Cur. XFoil Results"), this);
	m_pExportCurXFoilRes->setStatusTip(tr("Sets Speed vs. chord graph"));
	connect(m_pExportCurXFoilRes, SIGNAL(triggered()), pXDirect, SLOT(onExportCurXFoilResults()));

	m_pCurXFoilCtPlot = new QAction(tr("Max. Shear Coefficient"), this);
	m_pCurXFoilCtPlot->setCheckable(true);
	connect(m_pCurXFoilCtPlot, SIGNAL(triggered()), pXDirect, SLOT(onCtPlot()));

	m_CurXFoilDbPlot = new QAction(tr("Bottom Side D* and Theta"), this);
	m_CurXFoilDbPlot->setCheckable(true);
	connect(m_CurXFoilDbPlot, SIGNAL(triggered()), pXDirect, SLOT(onDbPlot()));

	m_pCurXFoilDtPlot = new QAction(tr("Top Side D* and Theta"), this);
	m_pCurXFoilDtPlot->setCheckable(true);
	connect(m_pCurXFoilDtPlot, SIGNAL(triggered()), pXDirect, SLOT(onDtPlot()));

	m_pCurXFoilRtLPlot = new QAction(tr("Log(Re_Theta)"), this);
	m_pCurXFoilRtLPlot->setCheckable(true);
	connect(m_pCurXFoilRtLPlot, SIGNAL(triggered()), pXDirect, SLOT(onRtLPlot()));

	m_pCurXFoilRtPlot = new QAction(tr("Re_Theta"), this);
	m_pCurXFoilRtPlot->setCheckable(true);
	connect(m_pCurXFoilRtPlot, SIGNAL(triggered()), pXDirect, SLOT(onRtPlot()));

	m_pCurXFoilNPlot = new QAction(tr("Amplification Ratio"), this);
	m_pCurXFoilNPlot->setCheckable(true);
	connect(m_pCurXFoilNPlot, SIGNAL(triggered()), pXDirect, SLOT(onNPlot()));

	m_pCurXFoilCdPlot = new QAction(tr("Dissipation Coefficient"), this);
	m_pCurXFoilCdPlot->setCheckable(true);
	connect(m_pCurXFoilCdPlot, SIGNAL(triggered()), pXDirect, SLOT(onCdPlot()));

	m_pCurXFoilCfPlot = new QAction(tr("Skin Friction Coefficient"), this);
	m_pCurXFoilCfPlot->setCheckable(true);
	connect(m_pCurXFoilCfPlot, SIGNAL(triggered()), pXDirect, SLOT(onCfPlot()));

	m_pCurXFoilUePlot = new QAction(tr("Edge Velocity"), this);
	m_pCurXFoilUePlot->setCheckable(true);
	connect(m_pCurXFoilUePlot, SIGNAL(triggered()), pXDirect, SLOT(onUePlot()));

	m_pCurXFoilHPlot = new QAction(tr("Kinematic Shape Parameter"), this);
	m_pCurXFoilHPlot->setCheckable(true);
	connect(m_pCurXFoilHPlot, SIGNAL(triggered()), pXDirect, SLOT(onHPlot()));

//	m_pImportJavaFoilPolar = new QAction(tr("Import JavaFoil Polar"), this);
//	connect(m_pImportJavaFoilPolar, SIGNAL(triggered()), pXDirect, SLOT(onImportJavaFoilPolar()));

	m_pImportXFoilPolar = new QAction(tr("Import XFoil Polar"), this);
	connect(m_pImportXFoilPolar, SIGNAL(triggered()), pXDirect, SLOT(onImportXFoilPolar()));

	m_pImportXMLFoilAnalysis = new QAction(tr("Import Analysis from XML"), this);
	connect(m_pImportXMLFoilAnalysis, SIGNAL(triggered()), pXDirect, SLOT(onImportXMLAnalysis()));

	m_pExportXMLFoilAnalysis = new QAction(tr("Export Analysis to XML"), this);
	connect(m_pExportXMLFoilAnalysis, SIGNAL(triggered()), pXDirect, SLOT(onExportXMLAnalysis()));
}


void MainFrame::createXDirectMenus()
{
	//MainMenu for XDirect Application
	m_pXDirectViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pXDirectViewMenu->addAction(m_pOpPointsAct);
		m_pXDirectViewMenu->addAction(m_pPolarsAct);
		m_pXDirectViewMenu->addSeparator();
		m_pXDirectViewMenu->addAction(m_pSaveViewToImageFileAct);
		m_pXDirectViewMenu->addSeparator();
		m_pXDirectViewMenu->addAction(m_pStyleAct);
	}

	m_pXDirectFoilMenu = menuBar()->addMenu(tr("&Foil"));
	{
		m_pXDirectFoilMenu->addAction(m_pManageFoilsAct);
		m_pXDirectFoilMenu->addSeparator();
		m_pCurrentFoilMenu = m_pXDirectFoilMenu->addMenu(tr("Current Foil"));
		{
			m_pCurrentFoilMenu->addAction(m_pSetCurFoilStyle);
			m_pCurrentFoilMenu->addSeparator();
			m_pCurrentFoilMenu->addAction(m_pExportCurFoil);
			m_pCurrentFoilMenu->addAction(m_pRenameCurFoil);
			m_pCurrentFoilMenu->addAction(m_pDeleteCurFoil);
			m_pCurrentFoilMenu->addAction(m_pDirectDuplicateCurFoil);
			m_pCurrentFoilMenu->addSeparator();
			m_pCurrentFoilMenu->addAction(m_pShowFoilPolarsOnly);
			m_pCurrentFoilMenu->addAction(m_pShowFoilPolars);
			m_pCurrentFoilMenu->addAction(m_pHideFoilPolars);
			m_pCurrentFoilMenu->addAction(m_pDeleteFoilPolars);
			m_pCurrentFoilMenu->addAction(m_pSaveFoilPolars);
			m_pCurrentFoilMenu->addSeparator();
			m_pCurrentFoilMenu->addAction(m_pShowFoilOpps);
			m_pCurrentFoilMenu->addAction(m_pHideFoilOpps);
			m_pCurrentFoilMenu->addAction(m_pDeleteFoilOpps);
		}
		m_pXDirectFoilMenu->addSeparator();
		m_pXDirectFoilMenu->addAction(m_pResetFoilScale);
		m_pXDirectFoilMenu->addAction(m_pShowPanels);
		m_pXDirectFoilMenu->addAction(m_pShowNeutralLine);
		m_pXDirectFoilMenu->addAction(m_pXDirectStyleAct);
	}

	m_pDesignMenu = menuBar()->addMenu(tr("&Design"));
	{
		m_pDesignMenu->addAction(m_pNormalizeFoil);
		m_pDesignMenu->addAction(m_pDerotateFoil);
		m_pDesignMenu->addAction(m_pRefineGlobalFoil);
		m_pDesignMenu->addAction(m_pRefineLocalFoil);
		m_pDesignMenu->addAction(m_pEditCoordsFoil);
		m_pDesignMenu->addAction(m_pScaleFoil);
		m_pDesignMenu->addAction(m_pSetTEGap);
		m_pDesignMenu->addAction(m_pSetLERadius);
		m_pDesignMenu->addAction(m_pSetFlap);
		m_pDesignMenu->addSeparator();
		m_pDesignMenu->addAction(m_pInterpolateFoils);
		m_pDesignMenu->addAction(m_pNacaFoils);
	}

	m_pXFoilAnalysisMenu = menuBar()->addMenu(tr("Analysis"));
	{
		m_pXFoilAnalysisMenu->addAction(m_pDefinePolarAct);
		m_pXFoilAnalysisMenu->addAction(m_pBatchAnalysisAct);
		m_pXFoilAnalysisMenu->addAction(m_pMultiThreadedBatchAct);
		m_pXFoilAnalysisMenu->addSeparator();
		m_pXFoilAnalysisMenu->addAction(m_pImportXMLFoilAnalysis);
		m_pXFoilAnalysisMenu->addSeparator();
		m_pXFoilAnalysisMenu->addAction(m_pViewXFoilAdvanced);
		m_pXFoilAnalysisMenu->addAction(m_pViewLogFile);
		m_pXFoilAnalysisMenu->addSeparator();
		m_pXFoilAnalysisMenu->addAction(m_pExportCurXFoilRes);
	}

	m_pPolarMenu = menuBar()->addMenu(tr("&Polars"));
	{
		m_pCurrentPolarMenu = m_pPolarMenu->addMenu(tr("Current Polar"));
		{
			m_pCurrentPolarMenu->addAction(m_pGetPolarProps);
			m_pCurrentPolarMenu->addAction(m_pEditCurPolar);
			m_pCurrentPolarMenu->addAction(m_pResetCurPolar);
			m_pCurrentPolarMenu->addAction(m_pDeletePolar);
			m_pCurrentPolarMenu->addAction(m_pRenamePolarAct);
			m_pCurrentPolarMenu->addAction(m_pExportCurPolar);
			m_pCurrentPolarMenu->addSeparator();
			m_pCurrentPolarMenu->addAction(m_pExportXMLFoilAnalysis);
			m_pCurrentPolarMenu->addSeparator();
			m_pCurrentPolarMenu->addAction(m_pShowPolarOpps);
			m_pCurrentPolarMenu->addAction(m_pHidePolarOpps);
			m_pCurrentPolarMenu->addAction(m_pDeletePolarOpps);
			m_pCurrentPolarMenu->addAction(m_pExportPolarOpps);
		}
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(m_pImportXFoilPolar);
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(m_pExportAllPolars);
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(m_pXDirectPolarFilter);
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(m_pShowAllPolars);
		m_pPolarMenu->addAction(m_pHideAllPolars);
		m_pPolarMenu->addSeparator();
	}

	m_pOpPointMenu = menuBar()->addMenu(tr("Operating Points"));
	{
		m_pCurrentOppMenu = m_pOpPointMenu->addMenu(tr("Current OpPoint"));
		{
			m_pCurrentOppMenu->addAction(m_pExportCurOpp);
			m_pCurrentOppMenu->addAction(m_pDeleteCurOpp);
			m_pCurrentOppMenu->addAction(m_pGetOppProps);
		}
		m_pOpPointMenu->addSeparator();
		m_pXDirectCpGraphMenu = m_pOpPointMenu->addMenu(tr("Cp Graph"));
		{
			m_pXDirectCpGraphMenu->addAction(m_psetCpVarGraph);
			m_pXDirectCpGraphMenu->addAction(m_psetQVarGraph);
			m_pXDirectCpGraphMenu->addSeparator();
			m_pXDirectCpGraphMenu->addAction(m_pShowInviscidCurve);
			m_pXDirectCpGraphMenu->addSeparator();
			m_pCurXFoilResults = m_pXDirectCpGraphMenu->addMenu(tr("Current XFoil Results"));
			{
				m_pCurXFoilResults->addSeparator();
				m_pCurXFoilResults->addAction(m_pCurXFoilCtPlot);
				m_pCurXFoilResults->addAction(m_CurXFoilDbPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilDtPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilRtLPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilRtPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilNPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilCdPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilCfPlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilUePlot);
				m_pCurXFoilResults->addAction(m_pCurXFoilHPlot);
			}
			m_pXDirectCpGraphMenu->addSeparator();
			m_pXDirectCpGraphMenu->addAction(m_pResetCurGraphScales);
//			m_pXDirectCpGraphMenu->addAction(exportCurGraphAct);
		}
		m_pOpPointMenu->addAction(m_pShowMousePosAct);
		m_pOpPointMenu->addSeparator();
		m_pOpPointMenu->addAction(m_pShowCurOppOnly);
		m_pOpPointMenu->addAction(m_pHideAllOpPoints);
		m_pOpPointMenu->addAction(m_pShowAllOpPoints);
	}

	//XDirect foil Context Menu
	m_pOperFoilCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentFoilMenu_OperFoilCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current Foil"));
        {
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pSetCurFoilStyle);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pExportCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pRenameCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDeleteCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDirectDuplicateCurFoil);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pShowFoilPolarsOnly);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pShowFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pHideFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDeleteFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pSaveFoilPolars);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pShowFoilOpps);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pHideFoilOpps);
            m_pCurrentFoilMenu_OperFoilCtxMenu->addAction(m_pDeleteFoilOpps);
        }
        //m_pOperFoilCtxMenu->addMenu(m_pCurrentFoilMenu);
		m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pCurrentPolarMenu_OperFoilCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pGetPolarProps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pEditCurPolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pResetCurPolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pDeletePolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pRenamePolarAct);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pExportCurPolar);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addSeparator();
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pShowPolarOpps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pHidePolarOpps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pDeletePolarOpps);
            m_pCurrentPolarMenu_OperFoilCtxMenu->addAction(m_pExportPolarOpps);
        }
        //m_pOperFoilCtxMenu->addMenu(m_pCurrentPolarMenu);
		m_pOperFoilCtxMenu->addSeparator();//_______________
        m_pDesignMenu_OperPolarCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("&Design"));
        {
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pNormalizeFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pDerotateFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pRefineGlobalFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pRefineLocalFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pEditCoordsFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pScaleFoil);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pSetTEGap);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pSetLERadius);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pSetFlap);
            m_pDesignMenu_OperPolarCtxMenu->addSeparator();
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pInterpolateFoils);
            m_pDesignMenu_OperPolarCtxMenu->addAction(m_pNacaFoils);
        }

        //m_pOperFoilCtxMenu->addMenu(m_pDesignMenu);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pCurOppCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current OpPoint"));
		{
			m_pCurOppCtxMenu->addAction(m_pExportCurOpp);
			m_pCurOppCtxMenu->addAction(m_pDeleteCurOpp);
			m_pCurOppCtxMenu->addAction(m_pGetOppProps);
		}
		m_pOperFoilCtxMenu->addAction(m_pShowMousePosAct);

		m_pOperFoilCtxMenu->addSeparator();//_______________
	//	CurGraphCtxMenu = OperFoilCtxMenu->addMenu(tr("Cp graph"));
        m_pXDirectCpGraphMenu_OperPolarCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Cp Graph"));
        {
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_psetCpVarGraph);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_psetQVarGraph);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addSeparator();
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_pShowInviscidCurve);
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addSeparator();
            m_pCurXFoilResults_OperPolarCtxMenu = m_pXDirectCpGraphMenu_OperPolarCtxMenu->addMenu(tr("Current XFoil Results"));
            {
                m_pCurXFoilResults_OperPolarCtxMenu->addSeparator();
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilCtPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_CurXFoilDbPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilDtPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilRtLPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilRtPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilNPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilCdPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilCfPlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilUePlot);
                m_pCurXFoilResults_OperPolarCtxMenu->addAction(m_pCurXFoilHPlot);
            }
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addSeparator();
            m_pXDirectCpGraphMenu_OperPolarCtxMenu->addAction(m_pResetCurGraphScales);
//			m_pXDirectCpGraphMenu->addAction(exportCurGraphAct);
        }
        //m_pOperFoilCtxMenu->addMenu(m_pXDirectCpGraphMenu);

		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pDefinePolarAct);
		m_pOperFoilCtxMenu->addAction(m_pBatchAnalysisAct);
		m_pOperFoilCtxMenu->addAction(m_pMultiThreadedBatchAct);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pShowAllPolars);
		m_pOperFoilCtxMenu->addAction(m_pHideAllPolars);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pShowCurOppOnly);
		m_pOperFoilCtxMenu->addAction(m_pShowAllOpPoints);
		m_pOperFoilCtxMenu->addAction(m_pHideAllOpPoints);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pResetFoilScale);
		m_pOperFoilCtxMenu->addAction(m_pShowPanels);
		m_pOperFoilCtxMenu->addAction(m_pShowNeutralLine);
		m_pOperFoilCtxMenu->addAction(m_pXDirectStyleAct);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}
	//End XDirect foil Context Menu


	//XDirect polar Context Menu
	m_pOperPolarCtxMenu = new QMenu(tr("Context Menu"),this);
	{
        m_pCurrentFoilMenu_OperPolarCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Foil"));
        {
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pSetCurFoilStyle);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pExportCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pRenameCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDeleteCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDirectDuplicateCurFoil);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pShowFoilPolarsOnly);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pShowFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pHideFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDeleteFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pSaveFoilPolars);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pShowFoilOpps);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pHideFoilOpps);
            m_pCurrentFoilMenu_OperPolarCtxMenu->addAction(m_pDeleteFoilOpps);
        }

        //m_pOperPolarCtxMenu->addMenu(m_pCurrentFoilMenu);
        m_pCurrentPolarMenu_OperPolarCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Polar"));
        {
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pGetPolarProps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pEditCurPolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pResetCurPolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pDeletePolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pRenamePolarAct);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pExportCurPolar);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addSeparator();
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pShowPolarOpps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pHidePolarOpps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pDeletePolarOpps);
            m_pCurrentPolarMenu_OperPolarCtxMenu->addAction(m_pExportPolarOpps);
        }
        //m_pOperPolarCtxMenu->addMenu(m_pCurrentPolarMenu);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		QMenu *pCurGraphCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Graph"));
		{
			pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
			pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
			pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
		}
		m_pOperPolarCtxMenu->addAction(m_pAllGraphsSettings);
		m_pOperPolarCtxMenu->addAction(m_pAllGraphsScalesAct);
		m_pOperPolarCtxMenu->addAction(m_pShowMousePosAct);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pOperPolarCtxMenu->addAction(m_pDefinePolarAct);
		m_pOperPolarCtxMenu->addAction(m_pBatchAnalysisAct);
		m_pOperPolarCtxMenu->addAction(m_pMultiThreadedBatchAct);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pOperPolarCtxMenu->addAction(m_pShowAllPolars);
		m_pOperPolarCtxMenu->addAction(m_pHideAllPolars);
		m_pOperPolarCtxMenu->addAction(m_pShowAllOpPoints);
		m_pOperPolarCtxMenu->addAction(m_pHideAllOpPoints);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pOperPolarCtxMenu->addAction(m_pSaveViewToImageFileAct);
	}

	//End XDirect polar Context Menu
}


void MainFrame::createXInverseActions()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	m_pStoreFoil = new QAction(QIcon(":/images/OnStoreFoil.png"), tr("Store Foil"), this);
	m_pStoreFoil->setStatusTip(tr("Store Foil in database"));
	connect(m_pStoreFoil, SIGNAL(triggered()), pXInverse, SLOT(onStoreFoil()));

	m_pExtractFoil = new QAction(QIcon(":/images/OnExtractFoil.png"),tr("Extract Foil"), this);
	m_pExtractFoil->setStatusTip(tr("Extract a Foil from the database for modification"));
	connect(m_pExtractFoil, SIGNAL(triggered()), pXInverse, SLOT(onExtractFoil()));

	m_pXInverseStyles = new QAction(tr("Define Styles"), this);
	m_pXInverseStyles->setStatusTip(tr("Define the styles for this view"));
	connect(m_pXInverseStyles, SIGNAL(triggered()), pXInverse, SLOT(onInverseStyles()));

	m_pXInverseResetFoilScale = new QAction(QIcon(":/images/OnResetFoilScale.png"), tr("Reset foil scale")+"\t(R)", this);
	m_pXInverseResetFoilScale->setStatusTip(tr("Resets the scale to fit the screen size"));
	connect(m_pXInverseResetFoilScale, SIGNAL(triggered()), pXInverse, SLOT(onResetFoilScale()));

	m_pInverseInsertCtrlPt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
	connect(m_pInverseInsertCtrlPt, SIGNAL(triggered()), pXInverse, SLOT(onInsertCtrlPt()));

	m_pInverseRemoveCtrlPt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
	connect(m_pInverseRemoveCtrlPt, SIGNAL(triggered()), pXInverse, SLOT(onRemoveCtrlPt()));

	m_pInvQInitial = new QAction(tr("Show Q-Initial"), this);
	m_pInvQInitial->setCheckable(true);
	connect(m_pInvQInitial, SIGNAL(triggered()), pXInverse, SLOT(onQInitial()));

	m_pInvQSpec = new QAction(tr("Show Q-Spec"), this);
	m_pInvQSpec->setCheckable(true);
	connect(m_pInvQSpec, SIGNAL(triggered()), pXInverse, SLOT(onQSpec()));

	m_pInvQViscous = new QAction(tr("Show Q-Viscous"), this);
	m_pInvQViscous->setCheckable(true);
	connect(m_pInvQViscous, SIGNAL(triggered()), pXInverse, SLOT(onQViscous()));

	m_pInvQPoints = new QAction(tr("Show Points"), this);
	m_pInvQPoints->setCheckable(true);
	connect(m_pInvQPoints, SIGNAL(triggered()), pXInverse, SLOT(onQPoints()));

	m_pInvQReflected = new QAction(tr("Show Reflected"), this);
	m_pInvQReflected->setCheckable(true);
	connect(m_pInvQReflected, SIGNAL(triggered()), pXInverse, SLOT(onQReflected()));

	m_pInverseZoomIn = new QAction(QIcon(":/images/OnZoomIn.png"), tr("Zoom in"), this);
	m_pInverseZoomIn->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
	connect(m_pInverseZoomIn, SIGNAL(triggered()), pXInverse, SLOT(onZoomIn()));
}


void MainFrame::createXInverseMenus()
{
	//MainMenu for XInverse Application
	m_pXInverseViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pXInverseViewMenu->addAction(m_pXInverseStyles);
		m_pXInverseViewMenu->addSeparator();
		m_pXInverseViewMenu->addAction(m_pSaveViewToImageFileAct);
		m_pXInverseViewMenu->addSeparator();
		m_pXInverseViewMenu->addAction(m_pStyleAct);
	}

	m_pXInverseGraphMenu = menuBar()->addMenu(tr("&Graph"));
	{
		m_pXInverseGraphMenu->addAction(m_pCurGraphDlgAct);
		m_pXInverseGraphMenu->addAction(m_pResetCurGraphScales);
		m_pXInverseGraphMenu->addAction(m_pExportCurGraphAct);
	}

	m_pXInverseFoilMenu = menuBar()->addMenu(tr("&Foil"));
	{
		m_pXInverseFoilMenu->addAction(m_pStoreFoil);
		m_pXInverseFoilMenu->addAction(m_pExtractFoil);
		m_pXInverseFoilMenu->addAction(m_pXInverseResetFoilScale);
		m_pXInverseFoilMenu->addSeparator();
		m_pXInverseFoilMenu->addAction(m_pInvQInitial);
		m_pXInverseFoilMenu->addAction(m_pInvQSpec);
		m_pXInverseFoilMenu->addAction(m_pInvQViscous);
		m_pXInverseFoilMenu->addAction(m_pInvQPoints);
		m_pXInverseFoilMenu->addAction(m_pInvQReflected);
	}

	//Context Menu for XInverse Application
	m_pInverseContextMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pInverseContextMenu->addAction(m_pXInverseStyles);
		m_pInverseContextMenu->addAction(m_pXInverseResetFoilScale);
		m_pInverseContextMenu->addSeparator();
		m_pInverseContextMenu->addAction(m_pCurGraphDlgAct);
		m_pInverseContextMenu->addAction(m_pResetCurGraphScales);
		m_pInverseContextMenu->addAction(m_pShowMousePosAct);
		m_pInverseContextMenu->addSeparator();
		m_pInverseContextMenu->addAction(m_pInverseInsertCtrlPt);
		m_pInverseContextMenu->addAction(m_pInverseRemoveCtrlPt);
		m_pInverseContextMenu->addSeparator();
		m_pInverseContextMenu->addAction(m_pInvQInitial);
		m_pInverseContextMenu->addAction(m_pInvQSpec);
		m_pInverseContextMenu->addAction(m_pInvQViscous);
		m_pInverseContextMenu->addAction(m_pInvQPoints);
		m_pInverseContextMenu->addAction(m_pInvQReflected);
		m_pInverseContextMenu->addSeparator();
		m_pInverseContextMenu->addAction(m_pStoreFoil);
		m_pInverseContextMenu->addAction(m_pExtractFoil);
		m_pInverseContextMenu->addSeparator();

		m_pInverseContextMenu->addSeparator();
		m_pInverseContextMenu->addAction(m_pXInverseResetFoilScale);
	}
}



void MainFrame::createXInverseToolbar()
{
	m_pctrlFullInverse  = new QRadioButton(tr("Full Inverse"));
	m_pctrlMixedInverse = new QRadioButton(tr("Mixed Inverse"));
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	connect(m_pctrlFullInverse,  SIGNAL(clicked()), pXInverse, SLOT(onInverseApp()));
	connect(m_pctrlMixedInverse, SIGNAL(clicked()), pXInverse, SLOT(onInverseApp()));

	m_pctrlXInverseToolBar = addToolBar(tr("XInverse"));
	m_pctrlXInverseToolBar->addAction(m_pNewProjectAct);
	m_pctrlXInverseToolBar->addAction(m_pOpenAct);
	m_pctrlXInverseToolBar->addAction(m_pSaveAct);
	m_pctrlXInverseToolBar->addSeparator();
	m_pctrlXInverseToolBar->addWidget(m_pctrlFullInverse);
	m_pctrlXInverseToolBar->addWidget(m_pctrlMixedInverse);
	m_pctrlXInverseToolBar->addSeparator();
	m_pctrlXInverseToolBar->addAction(m_pExtractFoil);
	m_pctrlXInverseToolBar->addAction(m_pStoreFoil);
	m_pctrlXInverseToolBar->addSeparator();
	m_pctrlXInverseToolBar->addAction(m_pInverseZoomIn);
	m_pctrlXInverseToolBar->addAction(m_pResetCurGraphScales);
	m_pctrlXInverseToolBar->addAction(m_pXInverseResetFoilScale);
}



void MainFrame::deleteProject(bool bClosing)
{
	// clear everything
	int i;
	void *pObj;

	Objects3D::deleteObjects();

	for (i=Foil::s_oaFoil.size()-1; i>=0; i--)
	{
		pObj = Foil::s_oaFoil.at(i);
		Foil::s_oaFoil.removeAt(i);
		delete (Foil*)pObj;
	}
	for (i=Polar::s_oaPolar.size()-1; i>=0; i--)
	{
		pObj = Polar::s_oaPolar.at(i);
		Polar::s_oaPolar.removeAt(i);
		delete (Polar*)pObj;
	}
	for (i=OpPoint::s_oaOpp.size()-1; i>=0; i--)
	{
		pObj = OpPoint::s_oaOpp.at(i);
		OpPoint::s_oaOpp.removeAt(i);
		delete (OpPoint*)pObj;
	}

	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_pCurPlane  = NULL;
	pMiarex->m_pCurPOpp   = NULL;
	pMiarex->m_pCurWPolar = NULL;
	pMiarex->m_bStream = false;

	if(!bClosing)
	{
		m_pctrlPlane->clear();
		m_pctrlPlanePolar->clear();
		m_pctrlPlaneOpp->clear();
		m_pctrlFoil->clear();
		m_pctrlPolar->clear();
		m_pctrlOpPoint->clear();

		m_pctrlPlane->setEnabled(false);
		m_pctrlPlanePolar->setEnabled(false);
		m_pctrlPlaneOpp->setEnabled(false);
		m_pctrlFoil->setEnabled(false);
		m_pctrlPolar->setEnabled(false);
		m_pctrlOpPoint->setEnabled(false);


		pMiarex->setPlane();
/*		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
		switch(pMiarex->m_iView)
		{
			case XFLR5::WPOLARVIEW:    pMiarex->createWPolarCurves();    break;
			case XFLR5::STABPOLARVIEW: pMiarex->createStabRLCurves();    break;
			case XFLR5::STABTIMEVIEW:  pMiarex->createStabilityCurves(); break;
			case XFLR5::WOPPVIEW:      pMiarex->createWOppCurves();      break;
			case XFLR5::WCPVIEW:       pMiarex->createCpCurves();        break;
			default: break;
		}*/
		if(m_iApp==XFLR5::MIAREX) pMiarex->setControls();

		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		Foil::setCurFoil(NULL);
		Polar::setCurPolar(NULL);
		OpPoint::setCurOpp(NULL);
		pXDirect->setFoil();

		updateFoilListBox();
		if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
		else                       pXDirect->createOppCurves();

		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->fillFoilTable();
		pAFoil->selectFoil();


		QXInverse *pXInverse =(QXInverse*)m_pXInverse;
		pXInverse->clear();

		setProjectName("");
		setSaveState(true);
	}
}



QColor MainFrame::getColor(int type)
{
	//type
	// 0=Foil
	// 1=Polar
	// 2=Opp
	// 3=Wing (unused)
	// 4=WPolar
	// 5=WOpp
	// 6=POpp
	int i,j;
	bool bFound = false;
	switch (type)
	{
		case 0:
		{
			Foil *pFoil;
			for (j=0; j<s_ColorList.size(); j++)
			{
				for (i=0; i<Foil::s_oaFoil.size(); i++)
				{
					pFoil = (Foil*)Foil::s_oaFoil.at(i);
					bFound = false;
					if(colour(pFoil) == s_ColorList.at(j))
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return s_ColorList.at(j);
			}
			break;
		}
		case 1:
		{
			Polar *pPolar;
			for (j=0; j<s_ColorList.size(); j++)
			{
				for (i=0; i<Polar::s_oaPolar.size(); i++)
				{
					pPolar = (Polar*)Polar::s_oaPolar.at(i);
					bFound = false;
					if(colour(pPolar) == s_ColorList.at(j))
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
					return s_ColorList.at(j);
			}
			break;
		}
		case 2:
		{
			OpPoint *pOpPoint;
			for (j=0; j<s_ColorList.size(); j++){
				for (i=0; i<OpPoint::s_oaOpp.size(); i++)
				{
					pOpPoint = (OpPoint*)OpPoint::s_oaOpp.at(i);
					bFound = false;
					int r,g,b,a;
					pOpPoint->getColor(r,g,b,a);
					QColor clr(r,g,b,a);
					if(clr == s_ColorList.at(j))
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return s_ColorList.at(j);
			}
			break;
		}

		default:
		{
		}
	}
	return randomColor();
}



void MainFrame::keyPressEvent(QKeyEvent *event)
{
	bool bCtrl = (event->modifiers() & Qt::ControlModifier);

	if(m_iApp == XFLR5::XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->keyPressEvent(event);
	}
	else if(m_iApp == XFLR5::MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->keyPressEvent(event);
	}
	else if(m_iApp == XFLR5::DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->keyPressEvent(event);
	}
	else if(m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyPressEvent(event);
	}
    else
	{
		switch (event->key())
		{
		    case Qt::Key_1:
		    {
				if(bCtrl) onAFoil();
				break;
			}
		    case Qt::Key_2:
		    {
				if(bCtrl) onAFoil();
				break;
			}
		    case Qt::Key_3:
		    {
				if(bCtrl) onXInverse();
				break;
			}
		    case Qt::Key_4:
		    {
				if(bCtrl) onXInverseMixed();
				break;
			}
		    case Qt::Key_5:
		    {
				if(bCtrl) onXDirect();
				break;
		    }
			case Qt::Key_6:
			{
				if(bCtrl) onMiarex();
				break;
			}
			case Qt::Key_7:
			{
				if(bCtrl)
				{
					loadLastProject();
				}
				break;
			}
			case Qt::Key_8:
			{
				if(bCtrl) onOpenGLInfo();
				break;
			}

			case Qt::Key_L:
			{
				onLogFile();
				break;
			}


			default:
				event->ignore();
		}
	}
	event->accept();
}


void MainFrame::keyReleaseEvent(QKeyEvent *event)
{
	if(m_iApp == XFLR5::XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->keyReleaseEvent(event);
	}
	else if(m_iApp == XFLR5::MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;

		if (event->key()==Qt::Key_Control)
		{
			updateView();
		}
		else pMiarex->keyReleaseEvent(event);
	}
	else if(m_iApp == XFLR5::DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->keyReleaseEvent(event);
	}
	else if(m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyReleaseEvent(event);
	}
	event->accept();
}


bool MainFrame::loadPolarFileV3(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
	Foil *pFoil;
	Polar *pPolar = NULL;
	Polar *pOldPlr;
	OpPoint *pOpp, *pOldOpp;

	//first read all available foils
	int i,l,n;
	ar >> n;
//	if(n<100003) return false;

	ar>>n;

	for (i=0;i<n; i++)
	{
		pFoil = new Foil();

		if (!pFoil->serialize(ar, bIsStoring))
		{
			delete pFoil;
			return false;
		}

		pFoil->insertThisFoil();
	}


	//next read all available polars
	ar>>n;
	for (i=0; i<n; i++)
	{
		pPolar = new Polar();
		QColor clr = getColor(1);
		pPolar->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
		if (!pPolar->serialize(ar, bIsStoring))
		{
			delete pPolar;
			return false;
		}
		for (l=0; l<Polar::s_oaPolar.size(); l++)
		{
			pOldPlr = (Polar*)Polar::s_oaPolar[l];
			if (pOldPlr->foilName() == pPolar->foilName() &&
				pOldPlr->polarName()  == pPolar->polarName())
			{
				//just overwrite...
				Polar::s_oaPolar.removeAt(l);
				delete pOldPlr;
				//... and continue to add
			}
		}
		Polar::addPolar(pPolar);
	}

	//Last read all available operating points
	ar>>n;
	for (i=0; i<n; i++)
	{
		pOpp = new OpPoint();
		if(!pOpp)
		{
			delete pOpp;
			return false;
		}

		QColor clr = s_ColorList[OpPoint::s_oaOpp.size()%24];
		pOpp->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
		if(ArchiveFormat>=100002)
		{
			if (!pOpp->serializeOppWPA(ar, bIsStoring, 100002))
			{
				delete pOpp;
				return false;
			}
			else
			{
				pFoil = Foil::foil(pOpp->foilName());
				if(pFoil)
				{
//					memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
//					memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
				}
				else
				{
					delete pOpp;
				}
			}
		}
		else
		{
			if (!pOpp->serializeOppWPA(ar, bIsStoring))
			{
				delete pOpp;
				return false;
			}
			else
			{
				pFoil = Foil::foil(pOpp->foilName());
				if(pFoil)
				{
//					memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
//					memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
				}
				else
				{
					delete pOpp;
				}
			}
		}
		if(pOpp)
		{
			for (int l=OpPoint::s_oaOpp.size()-1;l>=0; l--)
			{
				pOldOpp = (OpPoint*)OpPoint::s_oaOpp.at(l);
				if (pOldOpp->foilName() == pOpp->foilName() &&
					pOldOpp->polarName()  == pOpp->polarName() &&
					qAbs(pOldOpp->aoa()-pOpp->aoa())<0.001)
				{
					//just overwrite...
					OpPoint::s_oaOpp.removeAt(l);
					delete pOldOpp;
					//... and continue to add
				}
			}
		}
		OpPoint::insertOpPoint(pOpp);
	}

	return true;
}


bool MainFrame::loadSettings()
{
	QPoint pt;
	bool bFloat;
	int SettingsFormat;
	QSize size;
	

#ifdef Q_OS_MAC
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

	settings.beginGroup("MainFrame");
	{
		SettingsFormat = settings.value("SettingsFormat").toInt();
		if(SettingsFormat != SETTINGSFORMAT) return false;

		Settings::s_StyleName = settings.value("StyleName","").toString();
		int k = settings.value("ExportFileType", 0).toInt();
		if (k==0) Settings::s_ExportFileType = XFLR5::TXT;
		else      Settings::s_ExportFileType = XFLR5::CSV;

		s_LanguageFilePath = settings.value("LanguageFilePath").toString();

		m_GraphExportFilter = settings.value("GraphExportFilter",".csv").toString();

		bFloat  = settings.value("Miarex_Float").toBool();
		pt.rx() = settings.value("Miarex_x").toInt();
		pt.ry() = settings.value("Miarex_y").toInt();
		size    = settings.value("MiarexSize").toSize();
		m_pctrlMiarexWidget->setFloating(bFloat);
		if(bFloat) m_pctrlMiarexWidget->move(pt);
		m_pctrlMiarexWidget->resize(size);

		bFloat  = settings.value("XDirect_Float").toBool();
		pt.rx() = settings.value("XDirect_x").toInt();
		pt.ry() = settings.value("XDirect_y").toInt();
		size    = settings.value("XDirectSize").toSize();
		m_pctrlXDirectWidget->setFloating(bFloat);
		if(bFloat) m_pctrlXDirectWidget->move(pt);
		m_pctrlXDirectWidget->resize(size);

		bFloat  = settings.value("AFoil_Float").toBool();
		pt.rx() = settings.value("AFoil_x").toInt();
		pt.ry() = settings.value("AFoil_y").toInt();
		size    = settings.value("AFoilSize").toSize();
		m_pctrlAFoilWidget->setFloating(bFloat);
		if(bFloat) m_pctrlAFoilWidget->move(pt);
		m_pctrlAFoilWidget->resize(size);

		bFloat  = settings.value("XInverse_Float").toBool();
		pt.rx() = settings.value("XInverse_x").toInt();
		pt.ry() = settings.value("XInverse_y").toInt();
		size    = settings.value("XInverseSize").toSize();
		m_pctrlXInverseWidget->setFloating(bFloat);
		if(bFloat) m_pctrlXInverseWidget->move(pt);
		m_pctrlXInverseWidget->resize(size);

		bFloat  = settings.value("StabView_Float").toBool();
		pt.rx() = settings.value("StabView_x").toInt();
		pt.ry() = settings.value("StabView_y").toInt();
		size    = settings.value("StabSize").toSize();
		m_pctrlStabViewWidget->setFloating(bFloat);
		if(bFloat) m_pctrlStabViewWidget->move(pt);
		m_pctrlStabViewWidget->resize(size);

		m_ImageDirName = settings.value("ImageDirName").toString();
		m_ExportLastDirName = settings.value("ExportLastDirName").toString();

		Units::setLengthUnitIndex(settings.value("LengthUnit").toInt());
		Units::setAreaUnitIndex(settings.value("AreaUnit").toInt());
		Units::setWeightUnitIndex(settings.value("WeightUnit").toInt());
		Units::setSpeedUnitIndex(settings.value("SpeedUnit").toInt());
		Units::setForceUnitIndex(settings.value("ForceUnit").toInt());
		Units::setMomentUnitIndex(settings.value("MomentUnit").toInt());
		Units::setPressureUnitIndex(settings.value("PressureUnit").toInt());
		Units::setInertiaUnitIndex(settings.value("InertiaUnit").toInt());
		Units::setUnitConversionFactors();

		QGraph::setOppHighlighting(settings.value("HighlightOpp").toBool());

		switch(settings.value("ImageFormat").toInt())
		{
			case 0:
				m_ImageFormat = XFLR5::PNG;
				break;
			case 1:
				m_ImageFormat = XFLR5::JPEG;
				break;
			case 2:
				m_ImageFormat = XFLR5::BMP;
				break;
			default:
				m_ImageFormat = XFLR5::PNG;
				break;
		}

		m_bAutoLoadLast = settings.value("AutoLoadLastProject").toBool();
		m_bSaveOpps   = settings.value("SaveOpps").toBool();
		m_bSaveWOpps  = settings.value("SaveWOpps").toBool();

		m_bAutoSave = settings.value("AutoSaveProject", false).toBool();
		m_SaveInterval = settings.value("AutoSaveInterval", 10).toInt();

//		a = settings.value("RecentFileSize").toInt();
		QString RecentF,strange;
		m_RecentFiles.clear();
		int n=0;
		do
		{
			RecentF = QString("RecentFile_%1").arg(n);
			strange = settings.value(RecentF).toString();
			if(strange.length())
			{
				m_RecentFiles.append(strange);
				n++;
			}
			else break;
		}while(n<MAXRECENTFILES);


		Settings::s_bStyleSheets  = settings.value("ShowStyleSheets", false).toBool();
		Settings::s_StyleSheetName = settings.value("StyleSheetName", "xflr5_style").toString();
		s_bShowMousePos = settings.value("ShowMousePosition", true).toBool();	
	}

	return true;
}


MainFrame* MainFrame::self() {
    if (!_self) {
        _self = new MainFrame(0L, 0L);
    }
    return _self;
}


XFLR5::enumApp MainFrame::loadXFLR5File(QString pathName)
{
	QFile XFile(pathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not open the file\n")+pathName;
		QMessageBox::information(window(), tr("Info"), strange);
		return XFLR5::NOAPP;
	}

	QAFoil *pAFoil= (QAFoil*)m_pAFoil;
	QXDirect * pXDirect = (QXDirect*)m_pXDirect;

	QString end = pathName.right(4).toLower();

	pathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

	int pos = pathName.lastIndexOf("/");
	if(pos>0) 	Settings::s_LastDirName = pathName.left(pos);

	if(end==".plr")
	{
		QDataStream ar(&XFile);
		ar.setVersion(QDataStream::Qt_4_5);
		ar.setByteOrder(QDataStream::LittleEndian);

		readPolarFile(ar);

		pXDirect->m_bPolarView = true;
		Polar::setCurPolar(NULL);
		OpPoint::setCurOpp(NULL);

		pXDirect->setFoil();

		pXDirect->setPolar();

		XFile.close();

		addRecentFile(pathName);
		setSaveState(false);
		pXDirect->setControls();
		return XFLR5::XFOILANALYSIS;
	}
	else if(end==".dat")
	{
		QString fileName = pathName;
		fileName.replace(".dat","");
		int pos1 = fileName.lastIndexOf("hn");
		fileName = fileName.right(fileName.length()-pos1);

		QTextStream ar(&XFile);
		Foil *pFoil = (Foil*)readFoilFile(ar, fileName);
		XFile.close();

		if(pFoil)
		{
			pFoil->insertThisFoil();
			Foil::setCurFoil(pFoil);
			Polar::setCurPolar(NULL);
			OpPoint::setCurOpp(NULL);

            XFile.close();

            setSaveState(false);
			addRecentFile(pathName);

			if(m_iApp==XFLR5::XFOILANALYSIS)
			{
				pXDirect->setControls();
				pXDirect->setFoil(pFoil);

                return XFLR5::XFOILANALYSIS;
            }
			else if(m_iApp==XFLR5::DIRECTDESIGN)  pAFoil->selectFoil(pFoil);


			return XFLR5::DIRECTDESIGN;
		}
	}
	else if(end==".wpa")
	{
		if(!s_bSaved)
		{
			QString strong = tr("Save the current project ?");
			int resp =  QMessageBox::question(this ,tr("Save"), strong,  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
			if(resp==QMessageBox::Cancel)
			{
				XFile.close();
				return XFLR5::NOAPP;
			}
			else if (resp==QMessageBox::Yes)
			{
				if(!saveProject(m_FileName))
				{
					XFile.close();
					return XFLR5::NOAPP;
				}
			}
		}

		deleteProject();

		QDataStream ar(&XFile);
		ar.setVersion(QDataStream::Qt_4_5);
		ar.setByteOrder(QDataStream::LittleEndian);
		if(!serializeProjectWPA(ar, false))
		{
			QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
		}

		addRecentFile(pathName);
		setSaveState(true);
		pathName.replace(".wpa", ".xfl", Qt::CaseInsensitive);
		setProjectName(pathName);

		XFile.close();

		if(Objects3D::s_oaPlane.size()) return XFLR5::MIAREX;
		else                            return XFLR5::XFOILANALYSIS;
	}
	else if(end==".xfl")
	{
		if(!s_bSaved)
		{
			QString strong = tr("Save the current project ?");
			int resp =  QMessageBox::question(this ,tr("Save"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
			if(resp==QMessageBox::Cancel)
			{
				XFile.close();
				return XFLR5::NOAPP;
			}
			else if (resp==QMessageBox::Yes)
			{
				if(!saveProject(m_FileName))
				{
					XFile.close();
					return XFLR5::NOAPP;
				}
			}
		}

		deleteProject();

		QDataStream ar(&XFile);
		if(!serializeProjectXFL(ar, false))
		{
			QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
		}

		addRecentFile(pathName);
		setSaveState(true);
		setProjectName(pathName);

		XFile.close();

		if(Objects3D::s_oaPlane.size()) return XFLR5::MIAREX;
		else                            return XFLR5::XFOILANALYSIS;
	}


	XFile.close();

	return XFLR5::NOAPP;
}



void MainFrame::onAFoil()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();

	m_iApp = XFLR5::DIRECTDESIGN;
	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlAFoilToolBar->show();

	m_pctrlMiarexWidget->hide();
	m_pctrlXDirectWidget->hide();
	m_pctrlXInverseWidget->hide();
	m_pctrlAFoilWidget->show();

	setMainFrameCentralWidget();
	setMenus();
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	pAFoil->setAFoilParams();
	updateView();
}


void MainFrame::onCurFoilStyle()
{
	if(!Foil::curFoil()) return;

	LinePickerDlg dlg(this);
	dlg.initDialog(Foil::curFoil()->foilPointStyle(), Foil::curFoil()->foilLineStyle(), Foil::curFoil()->foilLineWidth(), colour(Foil::curFoil()));

	if(QDialog::Accepted==dlg.exec())
	{
		Foil::curFoil()->setColor(dlg.lineColor().red(), dlg.lineColor().green(), dlg.lineColor().blue(), dlg.lineColor().alpha());
		Foil::curFoil()->foilLineStyle() = dlg.lineStyle();
		Foil::curFoil()->foilLineWidth() = dlg.width();
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->m_BufferFoil.setColor(Foil::curFoil()->red(), Foil::curFoil()->green(), Foil::curFoil()->blue(), Foil::curFoil()->alphaChannel());
		pXDirect->m_BufferFoil.foilLineStyle() = Foil::curFoil()->foilLineStyle();
		pXDirect->m_BufferFoil.foilLineWidth() = Foil::curFoil()->foilLineWidth();
		setSaveState(false);
	}

	updateView();
}


void MainFrame::onInsertProject()
{
	QString PathName;
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	PathName = QFileDialog::getOpenFileName(this, tr("Open File"),
											Settings::s_LastDirName,
											"Project file (*.wpa *.xfl)");
	if(!PathName.length()) return;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(this,tr("Warning"), tr("Could not read the file\n")+ PathName);
		return;
	}

	QString end = PathName.right(4).toLower();

	if(end==".wpa")
	{
		QDataStream ar(&XFile);
		ar.setVersion(QDataStream::Qt_4_5);
		ar.setByteOrder(QDataStream::LittleEndian);
		if(!serializeProjectWPA(ar, false))
		{
			QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+PathName+"\n");
		}

	}
	else if(end==".xfl")
	{
		QDataStream ar(&XFile);
		if(!serializeProjectXFL(ar, false))
		{
			QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+PathName+"\n");
		}
	}

	XFile.close();
	setSaveState(false);

	if(m_iApp == XFLR5::MIAREX)
	{
		updatePlaneListBox();
		pMiarex->setPlane();
		QMiarex::s_bResetCurves = true;
	}
	else if(m_iApp == XFLR5::XFOILANALYSIS)
	{
		if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
		else                       pXDirect->createOppCurves();
		updateFoilListBox();
	}
	else if(m_iApp == XFLR5::DIRECTDESIGN)
	{
		pAFoil->fillFoilTable();
		pAFoil->selectFoil();
	}
	updateView();
}


void MainFrame::onHighlightOperatingPoint()
{
	QGraph::setOppHighlighting(!QGraph::isHighLighting());
	m_pHighlightOppAct->setChecked(QGraph::isHighLighting());

	if(m_iApp == XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
		QMiarex::s_bResetCurves = true;
		pMiarex->updateView();
	}
	else if(m_iApp == XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->m_bResetCurves = true;
		pXDirect->updateView();
	}
}


void MainFrame::onLanguage()
{
    TranslatorDlg tDlg(this);
    tDlg.InitDialog();
    if(tDlg.exec()==QDialog::Accepted)
	{
	}
}


void MainFrame::onLoadFile()
{
	QStringList PathNames;
	QString PathName;
	XFLR5::enumApp App  = XFLR5::NOAPP;
	bool warn_non_airfoil_multiload = false;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	PathNames = QFileDialog::getOpenFileNames(this, tr("Open File"),
												Settings::s_LastDirName,
												"XFLR5 file (*.dat *.plr *.wpa *.xfl)");
	if(!PathNames.size()) return;
	if(PathNames.size() > 1)
	{
		for (int i=0; i<PathNames.size(); i++)
		{
			PathName = PathNames.at(i);
			if (PathName.endsWith(".dat"))
			{
				App = loadXFLR5File(PathName);
			} else {
				warn_non_airfoil_multiload = true;
			}
		}
		if (warn_non_airfoil_multiload) {
			QMessageBox::warning(0, QObject::tr("Warning"), QObject::tr("Multiple file loading only available for airfoil files.\nNon *.dat files will be ignored."));
		}
    }
    else
    {
		PathName = PathNames.at(0);
		if(!PathName.length()) return;


        PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

		int pos = PathName.lastIndexOf("/");
		if(pos>0) Settings::s_LastDirName = PathName.left(pos);

		App = loadXFLR5File(PathName);
	}

	if(m_iApp==XFLR5::NOAPP)
	{
		m_iApp = App;

		if(m_iApp==XFLR5::MIAREX) onMiarex();
                else                      onXDirect();
	}

	if(App==0)
	{
	}
	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		if(Polar::s_oaPolar.size())
		{
			if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
			else                       pXDirect->createOppCurves();
		}
		updateFoilListBox();
		updateView();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		updatePlaneListBox();
		pMiarex->setPlane();
		pMiarex->m_bIs2DScaleSet = false;
		pMiarex->setControls();
		updateView();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->setAFoilParams();
		pAFoil->selectFoil(Foil::curFoil());
		updateView();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		onXInverse();
		updateView();
	}
}


void MainFrame::onLogFile()
{
	QString FileName = QDir::tempPath() + "/XFLR5.log";
	// 20090605 Francesco Meschia
	QDesktopServices::openUrl(QUrl::fromLocalFile(FileName));
}




void MainFrame::onNewProject()
{
	if(!s_bSaved)
	{
		int resp = QMessageBox::question(this, tr("Question"), tr("Save the current project ?"),
										  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

		if (QMessageBox::Cancel == resp)
		{
			return;
		}
		else if (QMessageBox::Yes == resp)
		{
			if(saveProject(m_FileName))
			{
				deleteProject();
				statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
			}
			else return; //save failed, don't close
		}
		else if (QMessageBox::No == resp)
		{
			deleteProject();
		}
	}
	else
	{
		deleteProject();
	}

	m_pgl3Widget->m_bArcball = false;
	updateView();
}


void MainFrame::onOpenGLInfo()
{
	OpenGLInfoDlg w;
	w.resize(700, 800);
	w.exec();
}




void MainFrame::onResetSettings()
{
	int resp = QMessageBox::question(this, tr("Default Settings"), tr("Are you sure you want to reset the default settings ?"),
									 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
	if(resp == QMessageBox::Yes)
	{
		QMessageBox::warning(this,tr("Default Settings"), tr("The settings will be reset at the next session"));
#ifdef Q_OS_MAC
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
		settings.clear();
		Settings::s_LastDirName = QDir::homePath();
		// do not save on exit
		m_bSaveSettings = false;
	}
}


void MainFrame::onRestoreToolbars()
{
	if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		m_pctrlXInverseToolBar->hide();
		m_pctrlAFoilToolBar->hide();
		m_pctrlMiarexToolBar->hide();
		m_pctrlStabViewWidget->hide();
		m_pctrl3DScalesWidget->hide();

		m_pctrlAFoilWidget->hide();
		m_pctrlXInverseWidget->hide();
		m_pctrlMiarexWidget->hide();

		m_pctrlXDirectToolBar->show();
		m_pctrlXDirectWidget->show();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		m_pctrlXInverseToolBar->hide();
		m_pctrlMiarexToolBar->hide();
		m_pctrlXDirectToolBar->hide();
		m_pctrl3DScalesWidget->hide();

		m_pctrlXDirectWidget->hide();
		m_pctrlXInverseWidget->hide();
		m_pctrlMiarexWidget->hide();
		m_pctrlStabViewWidget->hide();

		m_pctrlAFoilToolBar->show();
		m_pctrlAFoilWidget->show();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		m_pctrlAFoilToolBar->hide();
		m_pctrlMiarexToolBar->hide();
		m_pctrlXDirectToolBar->hide();
		m_pctrl3DScalesWidget->hide();
		m_pctrlStabViewWidget->hide();

		m_pctrlAFoilWidget->hide();
		m_pctrlXDirectWidget->hide();
		m_pctrlMiarexWidget->hide();

		m_pctrlXInverseToolBar->show();
		m_pctrlXInverseWidget->show();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		m_pctrlXInverseToolBar->hide();
		m_pctrlAFoilToolBar->hide();
		m_pctrlXDirectToolBar->hide();
		m_pctrl3DScalesWidget->hide();

		m_pctrlAFoilWidget->hide();
		m_pctrlXDirectWidget->hide();
		m_pctrlXInverseWidget->hide();
		m_pctrlMiarexWidget->show();
		m_pctrlMiarexToolBar->show();
//		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
//		if(pMiarex->m_iView==WSTABVIEW) m_pctrlStabViewWidget->show();
//		else                            m_pctrlStabViewWidget->hide();
	}
}


void MainFrame::onSaveOptions()
{
    SaveOptionsDlg soDlg(this);
	soDlg.initDialog(m_bAutoLoadLast, m_bSaveOpps, m_bSaveWOpps, m_bAutoSave, m_SaveInterval);
    if(soDlg.exec()==QDialog::Accepted)
	{
		m_bAutoLoadLast = soDlg.m_bAutoLoadLast;
		m_bAutoSave     = soDlg.m_bAutoSave;
		m_SaveInterval  = soDlg.m_SaveInterval;
		m_bSaveOpps     = soDlg.m_bOpps;
		m_bSaveWOpps    = soDlg.m_bWOpps;

		if(m_bAutoSave)
		{
			if(m_pSaveTimer)
			{
				m_pSaveTimer->stop();
				delete m_pSaveTimer;
			}
			m_pSaveTimer = new QTimer(this);
			m_pSaveTimer->setInterval(m_SaveInterval*60*1000);
			m_pSaveTimer->start();
			connect(m_pSaveTimer, SIGNAL(timeout()), this, SLOT(onSaveTimer()));
		}
	}
}


void MainFrame::onSaveTimer()
{
	if (!s_ProjectName.length()) return;
	if(saveProject(m_FileName))
	{
		statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
	}
}


void MainFrame::onSaveProject()
{
/*	QString Filter = "XFLR5 v6 Project File (*.xfl)";

	QString saveFileName = "/home/techwinder/download/save.bin";
	QFile fp(saveFileName);
	fp.open(QIODevice::WriteOnly);
	QDataStream ar(&fp);

	ar<<1025;
	ar << QColor(159,158,157,156);

	ar << QColor(7,6,5,4);
	fp.close();
	return;*/

	QString FileName = s_ProjectName;

	if (!s_ProjectName.length())
	{
		onSaveProjectAs();
		return;
	}
	if(saveProject(m_FileName))
	{
		addRecentFile(m_FileName);
		statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
	}
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	pMiarex->updateView();
}



bool MainFrame::onSaveProjectAs()
{
	if(saveProject())
	{
		setProjectName(m_FileName);
		addRecentFile(m_FileName);
		statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
	}

	return true;
}




void MainFrame::onSaveViewToImageFile()
{
	QString FileName, Filter;

	switch(m_ImageFormat)
	{
		case XFLR5::PNG :
		{
			Filter = "Portable Network Graphics (*.png)";
			break;
		}
		case XFLR5::JPEG :
		{
			Filter = "JPEG (*.jpg)";
			break;
		}
		case XFLR5::BMP :
		{
			Filter = "Windows Bitmap (*.bmp)";
			break;
		}
	}

	FileName = QFileDialog::getSaveFileName(this, tr("Save Image"),
											m_ImageDirName,
											"Portable Network Graphics (*.png);;JPEG (*.jpg);;Windows Bitmap (*.bmp)",
											&Filter);

	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) m_ImageDirName = FileName.left(pos);

	if(Filter == "Portable Network Graphics (*.png)")
	{
		if(FileName.right(4)!=".png") FileName+= ".png";
		m_ImageFormat = XFLR5::PNG;
	}
	else if(Filter == "JPEG (*.jpg)")
	{
		if(FileName.right(4)!=".jpg") FileName+= ".jpg";
		m_ImageFormat = XFLR5::JPEG;
	}
	else if(Filter == "Windows Bitmap (*.bmp)")
	{
		if(FileName.right(4)!=".bmp") FileName+= ".bmp";
		m_ImageFormat = XFLR5::BMP;
	}

	switch(m_iApp)
	{
		case XFLR5::XFOILANALYSIS:
		{
			QPixmap pix = m_pXDirectTileWidget->grab();
			pix.save(FileName, "PNG");
			return;
		}
		case XFLR5::DIRECTDESIGN:
		{
			QPixmap pix = m_pDirect2dWidget->grab();
			pix.save(FileName, "PNG");
			return;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QPixmap pix = m_p2dWidget->grab();
			pix.save(FileName, "PNG");
			return;
		}
		case XFLR5::MIAREX:
		{
			QMiarex *pMiarex = (QMiarex*)m_pMiarex;

			if(pMiarex->m_iView==XFLR5::W3DVIEW)
			{
				QMessageBox::StandardButton reply = QMessageBox::question(this, "3D save option", tr("Set a transparent background ?"), QMessageBox::Yes|QMessageBox::No);
				if (reply == QMessageBox::Yes)
				{
					QPixmap outPix = m_pgl3Widget->grab();
					QPainter painter(&outPix);
					painter.drawPixmap(0,0, pMiarex->m_PixText);
					painter.drawPixmap(0,0, m_pgl3Widget->m_PixTextOverlay);

					outPix.save(FileName);
				}
				else
				{
					QImage outImg = m_pgl3Widget->grabFramebuffer();
					QPainter painter(&outImg);
					painter.drawPixmap(0,0, pMiarex->m_PixText);
					painter.drawPixmap(0,0, m_pgl3Widget->m_PixTextOverlay);

					outImg.save(FileName);
				}

				return;
			}
			else
			{
				QPixmap pix = m_pMiarexTileWidget->grab();
				pix.save(FileName, "PNG");
				return;
			}
		}
		default:
			break;
	}
}


void MainFrame::onSelChangePlane(int sel)
{
	// Gets the new selected wing name and notifies Miarex
	// then updates WPolar combobox
	// and selects either the current WPolar
	// or the first one in the list, if any
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();
	QString strong;
//	int sel = m_pctrlPlane->currentIndex();
	if (sel >=0) strong = m_pctrlPlane->itemText(sel);
	pMiarex->setPlane(strong);
	pMiarex->updateView();
}


void MainFrame::onSelChangeWPolar(int sel)
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QString strong;
//	int sel = m_pctrlPlanePolar->currentIndex();
	if (sel>=0) strong = m_pctrlPlanePolar->itemText(sel);
	m_iApp = XFLR5::MIAREX;
	pMiarex->setWPolar(false, strong);
	pMiarex->setControls();
	pMiarex->updateView();
}


void MainFrame::onSelChangePlaneOpp(int sel)
{
	QString strong;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	// Gets the new selected WOpp name and notifies Miarex
	if(!m_pctrlPlaneOpp->count())
	{
		QMiarex::s_bResetCurves = true;
		pMiarex->updateView();
		return;
	}

	if (sel>=0) strong = m_pctrlPlaneOpp->itemText(sel);
	else
	{
		m_pctrlPlaneOpp->setCurrentIndex(0);
		strong = m_pctrlPlaneOpp->itemText(0).trimmed();
	}
	if(strong.length())
	{
		bool bOK;
		double x = locale().toDouble(strong, &bOK);
		if(bOK)
		{
			m_iApp = XFLR5::MIAREX;
			pMiarex->setPlaneOpp(false, x);
			pMiarex->updateView();
		}
		else
		{
			QMessageBox::warning(window(), tr("Warning"), tr("Unidentified Operating Point"));
			pMiarex->setPlaneOpp(true);
		}
	}
}


void MainFrame::onSelChangeFoil(int sel)
{
	// Gets the new selected foil name and notifies XDirect
	// sets a polar, if any
	// then updates the polar combobox

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;

	QString strong;
//	int sel = m_pctrlFoil->currentIndex();
	if (sel >=0) strong = m_pctrlFoil->itemText(sel);

	Foil::setCurFoil(Foil::foil(strong));
	pXDirect->setFoil(Foil::curFoil());
	pXDirect->setPolar();
	m_iApp = XFLR5::XFOILANALYSIS;
	updatePolarListBox();
	pXDirect->setControls();
	updateView();
}


void MainFrame::onSelChangePolar(int sel)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;

	// Gets the new selected polar name and notifies XDirect
	QString strong;
//	int selNew = m_pctrlPolar->currentIndex();
	if (sel>=0) strong = m_pctrlPolar->itemText(sel);
	m_iApp = XFLR5::XFOILANALYSIS;


	pXDirect->setPolar(Polar::getPolar(Foil::curFoil(), strong));
	updateOppListBox();
	pXDirect->setControls();
	updateView();
}


void MainFrame::onSelChangeOpp(int sel)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;

	// Gets the new selected Opp name and notifies XDirect
	QString strong;

	if (sel>=0) strong = m_pctrlOpPoint->itemText(sel);
	m_iApp = XFLR5::XFOILANALYSIS;

	double Alpha;
	bool bOK;
	Alpha = locale().toDouble(strong, &bOK);

	if(bOK)
	{
		pXDirect->setOpp(Alpha);
	}
	else
	{
		QMessageBox::warning(window(), tr("Warning"), tr("Unidentified Operating Point"));
		OpPoint::setCurOpp(NULL);
		pXDirect->setOpp();
	}
	updateView();
}


void MainFrame::onShowMousePos()
{
	s_bShowMousePos = !s_bShowMousePos;
	m_pShowMousePosAct->setChecked(s_bShowMousePos);
}


void MainFrame::onStyleSettings()
{
	QXDirect *pXDirect   = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)m_pMiarex;

	Settings DSdlg(this);

	DSdlg.initDialog();

	DSdlg.exec();
	pMiarex->m_bResetglGeom = true;
	pMiarex->m_bResetglBody = true;
	pMiarex->m_bResetglLegend = true;

	if(DSdlg.m_bIsGraphModified)
	{
		setGraphSettings(&Settings::s_RefGraph);
	}
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	pAFoil->setTableFont();

	pXDirect->m_CpGraph.setInverted(true);
	pMiarex->m_CpGraph.setInverted(true);

	setMainFrameCentralWidget();

	updateView();
}


void MainFrame::onUnits()
{
	Units uDlg(this);
	uDlg.initDialog();

	if(uDlg.exec()==QDialog::Accepted)
	{
		setSaveState(false);

		if(m_iApp==XFLR5::MIAREX)
		{
			QMiarex *pMiarex= (QMiarex*)m_pMiarex;
			pMiarex->updateUnits();
		}
	}
}


void MainFrame::onXDirect()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	m_iApp = XFLR5::XFOILANALYSIS;

	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlXDirectToolBar->show();

	m_pctrlAFoilWidget->hide();
	m_pctrlMiarexWidget->hide();
	m_pctrlXInverseWidget->hide();
	m_pctrlXDirectWidget->show();

	pXDirect->setFoil();
	updateFoilListBox();
	setMainFrameCentralWidget();
	setMenus();
	checkGraphActions();

	pXDirect->setControls();
	pXDirect->setFoilScale();
	pXDirect->updateView();
}




void MainFrame::onMiarex()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	m_iApp = XFLR5::MIAREX;

	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlMiarexToolBar->show();

	m_pctrlXDirectWidget->hide();
	m_pctrlAFoilWidget->hide();
	m_pctrlXInverseWidget->hide();
	m_pctrlMiarexWidget->show();

	updatePlaneListBox();
	pMiarex->setPlane();

	setMenus();
	setMainFrameCentralWidget();
	checkGraphActions();
	pMiarex->setControls();
	pMiarex->setCurveParams();
	updateView();
}



void MainFrame::onXInverse()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
//	pXInverse->SetScale();
	m_iApp = XFLR5::INVERSEDESIGN;

	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->show();

	m_pctrlAFoilWidget->hide();
	m_pctrlMiarexWidget->hide();
	m_pctrlXDirectWidget->hide();
	m_pctrlXInverseWidget->show();

	setMainFrameCentralWidget();
	setMenus();
	checkGraphActions();
	pXInverse->setParams();
	pXInverse->updateView();
}


void MainFrame::onXInverseMixed()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
//	pXInverse->SetScale();
	m_iApp = XFLR5::INVERSEDESIGN;
	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->show();

	m_pctrlAFoilWidget->hide();
	m_pctrlMiarexWidget->hide();
	m_pctrlXDirectWidget->hide();
	m_pctrlXInverseWidget->show();
	pXInverse->m_bFullInverse = false;
	setMainFrameCentralWidget();
	setMenus();
	checkGraphActions();
	pXInverse->setParams();
	pXInverse->updateView();
}


void MainFrame::onOpenRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) return;

	QXDirect *pXDirect = (QXDirect*) m_pXDirect;

	XFLR5::enumApp App = loadXFLR5File(action->data().toString());
	if(m_iApp==XFLR5::NOAPP) m_iApp = App;

	if(App==XFLR5::NOAPP)
	{
		m_iApp = App;
		QString FileName = action->data().toString();
		m_RecentFiles.removeAll(FileName);
		updateRecentFileActions();
	}

	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		if(Polar::s_oaPolar.size())
		{
			if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
			else                       pXDirect->createOppCurves();
		}
		updateFoilListBox();
		onXDirect();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		onMiarex();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->setAFoilParams();
		onAFoil();
		updateView();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		onXInverse();
		updateView();
	}
}


void *MainFrame::readFoilFile(QTextStream &in, QString fileName)
{
	QString strong;
	QString tempStr;
	QString FoilName;

	Foil* pFoil = NULL;
	int pos, i, ip;
	pos = 0;
	double x, y, z, area;
	double xp, yp;
	bool bRead;


	pFoil = new Foil();
	if(!pFoil)	return NULL;

	while(tempStr.length()==0 && !in.atEnd())
	{
		strong = in.readLine();
		pos = strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)strong.truncate(pos);
		tempStr = strong;
		tempStr.remove(" ");
		FoilName = strong;
	}

	if(!in.atEnd())
	{
		// FoilName contains the last comment

		if(readValues(strong,x,y,z)==2)
		{
			//there isn't a name on the first line, use the file's name
			FoilName = fileName;
			{
				pFoil->xb[0] = x;
				pFoil->yb[0] = y;
				pFoil->nb=1;
				xp = x;
				yp = y;
			}
		}
		else FoilName = strong;
		// remove fore and aft spaces
		FoilName = FoilName.trimmed();
	}

	bRead = true;
	xp=-9999.0;
	yp=-9999.0;
	do
	{
		strong = in.readLine();
		pos = strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)strong.truncate(pos);
		tempStr = strong;
		tempStr.remove(" ");
		if (!strong.isNull() && bRead && tempStr.length())
		{
			if(readValues(strong, x,y,z)==2)
			{
				//add values only if the point is not coincident with the previous one
				double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
				if(dist>0.000001)
				{
					pFoil->xb[pFoil->nb] = x;
					pFoil->yb[pFoil->nb] = y;
					pFoil->nb++;
					if(pFoil->nb>IQX)
					{
						delete pFoil;
						return NULL;
					}
					xp = x;
					yp = y;
				}
			}
			else bRead = false;
		}
	}while (bRead && !strong.isNull());

	pFoil->foilName() = FoilName;

	// Check if the foil was written clockwise or counter-clockwise

	area = 0.0;
	for (i=0; i<pFoil->nb; i++)
	{
		if(i==pFoil->nb-1)	ip = 0;
		else				ip = i+1;
		area +=  0.5*(pFoil->yb[i]+pFoil->yb[ip])*(pFoil->xb[i]-pFoil->xb[ip]);
	}

	if(area < 0.0)
	{
		//reverse the points order
		double xtmp, ytmp;
		for (int i=0; i<pFoil->nb/2; i++)
		{
			xtmp         = pFoil->xb[i];
			ytmp         = pFoil->yb[i];
			pFoil->xb[i] = pFoil->xb[pFoil->nb-i-1];
			pFoil->yb[i] = pFoil->yb[pFoil->nb-i-1];
			pFoil->xb[pFoil->nb-i-1] = xtmp;
			pFoil->yb[pFoil->nb-i-1] = ytmp;
		}
	}

	memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
	memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
	pFoil->n = pFoil->nb;

	QColor clr = randomColor();
	pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
	pFoil->initFoil();

	return pFoil;
}


/**
 *Reads a Foil and its related Polar objects from a binary stream associated to a .plr file.
 * @param ar the binary stream
 * @return the pointer to the Foil object which has been created, or NULL if failure.
 */
void MainFrame::readPolarFile(QDataStream &ar)
{
	Foil* pFoil = NULL;
	Polar *pPolar = NULL;
	Polar * pOldPolar;
	int i, n, l;

	ar >> n;

	if(n<100000)
	{
		//old format
		QMessageBox::warning(window(), tr("Warning"), tr("Obsolete format, cannot read"));
		return;
	}
	else if (n >=100000)
	{
		//new format XFLR5 v1.99+
		//first read all available foils
		ar>>n;
		for (i=0;i<n; i++)
		{
			pFoil = new Foil();
			if (!pFoil->serialize(ar, false))
			{
				delete pFoil;
				return;
			}
			pFoil->insertThisFoil();
		}

		//next read all available polars

		ar>>n;
		for (i=0;i<n; i++)
		{
			pPolar = new Polar();

			if (!pPolar->serialize(ar, false))
			{
				delete pPolar;
				return;
			}
			for (l=0; l<Polar::s_oaPolar.size(); l++)
			{
				pOldPolar = (Polar*)Polar::s_oaPolar[l];
				if (pOldPolar->foilName() == pPolar->foilName() &&
					pOldPolar->polarName()  == pPolar->polarName())
				{
					//just overwrite...
					Polar::s_oaPolar.removeAt(l);
					delete pOldPolar;
					//... and continue to add
				}
			}
			Polar::addPolar(pPolar);
		}
	}
}



bool MainFrame::saveProject(QString PathName)
{
	QString Filter = "XFLR5 v6 Project File (*.xfl)";
	QString FileName = s_ProjectName;

	if(!PathName.length())
	{
		PathName = QFileDialog::getSaveFileName(this, tr("Save the Project File"),
												Settings::s_LastDirName+"/"+FileName,
												"XFLR5 v6 Project File (*.xfl)",
												&Filter);

		if(!PathName.length()) return false;//nothing more to do

		int pos = PathName.indexOf(".xfl", Qt::CaseInsensitive);
		if(pos<0) PathName += ".xfl";

		PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

		pos = PathName.lastIndexOf("/");
		if(pos>0) Settings::s_LastDirName = PathName.left(pos);
	}

	QFile fp(PathName);

	if (!fp.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
		return false;
	}

	QDataStream ar(&fp);
	serializeProjectXFL(ar,true);

	m_FileName = PathName;
	fp.close();

	saveSettings();

	setSaveState(true);

	return true;
}



void MainFrame::onSavePlaneAsProject()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	QString strong;
	if(pMiarex->m_pCurPlane) strong = pMiarex->m_pCurPlane->planeName();
	else
	{
		QMessageBox::warning(this, tr("Warning"), tr("Nothing to save"));
		return ;
	}

	QString PathName;
	QString Filter = "XFLR5 v6 Project File (*.xfl)";
	QString FileName = strong;

	PathName = QFileDialog::getSaveFileName(this, tr("Save the Project File"),
											Settings::s_LastDirName+"/"+FileName,
											"XFLR5 v6 Project File (*.xfl)",
											&Filter);

	if(!PathName.length()) return;//nothing more to do
	int pos = PathName.indexOf(".xfl", Qt::CaseInsensitive);
	if(pos<0) PathName += ".xfl";
	PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator
	pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);


	QFile fp(PathName);

	if (!fp.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
		return;
	}

	QDataStream ar(&fp);

	serializePlaneProject(ar);
	fp.close();
}



bool MainFrame::serializePlaneProject(QDataStream &ar)
{
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	if(!pMiarex->m_pCurPlane)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Nothing to save"));
		return false;
	}

	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	WPolar *pWPolar = NULL;
	Polar *pPolar   = NULL;

	QString PlaneName = pMiarex->m_pCurPlane->planeName();

	bool bIsStoring = true;
	int i, iSize;

	int ArchiveFormat = 200001;
	ar << ArchiveFormat;
	// 200001 : First instance of new ".xfl" format

	//Save unit data
	ar << Units::lengthUnitIndex();
	ar << Units::areaUnitIndex();
	ar << Units::weightUnitIndex();
	ar << Units::speedUnitIndex();
	ar << Units::forceUnitIndex();
	ar << Units::momentUnitIndex();

	//Save default Polar data. Not in the Settings, since this is Project dependant
	if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
	else if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
	else if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDAOAPOLAR)   ar<<4;
	else if(WPolarDlg::s_WPolar.polarType()==XFLR5::BETAPOLAR)       ar<<5;
	else if(WPolarDlg::s_WPolar.polarType()==XFLR5::STABILITYPOLAR)  ar<<7;
	else ar << 0;

	if(WPolarDlg::s_WPolar.analysisMethod()==XFLR5::LLTMETHOD)        ar << 1;
	else if(WPolarDlg::s_WPolar.analysisMethod()==XFLR5::VLMMETHOD)   ar << 2;
	else if(WPolarDlg::s_WPolar.analysisMethod()==XFLR5::PANELMETHOD) ar << 3;
	else ar << 0;

	ar << WPolarDlg::s_WPolar.mass();
	ar << WPolarDlg::s_WPolar.m_QInfSpec;
	ar << WPolarDlg::s_WPolar.CoG().x;
	ar << WPolarDlg::s_WPolar.CoG().y;
	ar << WPolarDlg::s_WPolar.CoG().z;

	ar << WPolarDlg::s_WPolar.density();
	ar << WPolarDlg::s_WPolar.viscosity();
	ar << WPolarDlg::s_WPolar.m_AlphaSpec;
	ar << WPolarDlg::s_WPolar.m_BetaSpec;

	ar << WPolarDlg::s_WPolar.bTilted();
	ar << WPolarDlg::s_WPolar.bWakeRollUp();

	// save the plane
	ar << 1;
	pMiarex->m_pCurPlane->serializePlaneXFL(ar, bIsStoring);

	// save the WPolars associated to this plane
	//count the polars
	iSize = 0;
	for (i=0; i<Objects3D::s_oaWPolar.size();i++)
	{
		pWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
		if(pWPolar->planeName()==PlaneName) iSize++;
	}
	ar << iSize;
	for (i=0; i<Objects3D::s_oaWPolar.size();i++)
	{
		pWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
		if(pWPolar->planeName()==PlaneName) pWPolar->serializeWPlrXFL(ar, bIsStoring);
	}

	ar << 0; //no need to save the operating points

	// then the foils
	// list the foils associated to this Plane's wings
	QList<Foil*> foilList;
	for(i=0; i<Foil::s_oaFoil.size(); i++)
	{
		Foil *pFoil = (Foil*)Foil::s_oaFoil.at(i);
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(pMiarex->m_pCurPlane->m_Wing[iw].isWingFoil(pFoil))
			{
				foilList.append(pFoil);
				break;
			}
		}
	}

	ar << foilList.count();
	for(int iFoil=0; iFoil<foilList.size(); iFoil++)
	{
		foilList.at(iFoil)->serializeFoilXFL(ar, bIsStoring);
	}

	// the foil polars
	// list the foil polars associated to this Plane's wings
	QList<Polar*> polarList;
	for(i=0; i<Polar::s_oaPolar.size(); i++)
	{
		pPolar = (Polar*)Polar::s_oaPolar.at(i);
		for(int iFoil=0; iFoil<foilList.count(); iFoil++)
		{
			if(pPolar->foilName() == foilList.at(iFoil)->foilName())
			{
				bool bListed = false;
				for(int ip=0; ip<polarList.count(); ip++)
				{
					if(pPolar==polarList.at(ip))
					{
						bListed = true;
						break; // polar list
					}
				}
				if(!bListed)
				{
					polarList.append(pPolar);
					break;  //foil list
				}
			}
		}
	}

	ar << polarList.size();
	for (int ip=0; ip<polarList.size();ip++)
	{
		polarList.at(ip)->serializePolarXFL(ar, true);
	}

	ar << 0; //no need to save the operating points

	// and the spline foil whilst we're at it
	pAFoil->m_pSF->serialize(ar, bIsStoring);

	return true;
}


void MainFrame::saveSettings()
{
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	if(!m_bSaveSettings) return;
#ifdef Q_OS_MAC
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
	settings.beginGroup("MainFrame");
	{
		settings.setValue("SettingsFormat", SETTINGSFORMAT);
		settings.setValue("FrameGeometryx", frameGeometry().x());
		settings.setValue("FrameGeometryy", frameGeometry().y());
		settings.setValue("SizeWidth", size().width());
		settings.setValue("SizeHeight", size().height());
		settings.setValue("SizeMaximized", isMaximized());
		settings.setValue("StyleName", Settings::s_StyleName);

		if (Settings::s_ExportFileType==XFLR5::TXT) settings.setValue("ExportFileType", 0);
		else                                        settings.setValue("ExportFileType", 1);

		settings.setValue("GraphExportFilter", m_GraphExportFilter);
		settings.setValue("Miarex_Float", m_pctrlMiarexWidget->isFloating());
		settings.setValue("XDirect_Float", m_pctrlXDirectWidget->isFloating());
		settings.setValue("AFoil_Float", m_pctrlAFoilWidget->isFloating());
		settings.setValue("XInverse_Float", m_pctrlXInverseWidget->isFloating());
		settings.setValue("StabView_Float", m_pctrlStabViewWidget->isFloating());
		settings.setValue("Miarex_x", m_pctrlMiarexWidget->frameGeometry().x());
		settings.setValue("Miarex_y", m_pctrlMiarexWidget->frameGeometry().y());
		settings.setValue("XDirect_x", m_pctrlXDirectWidget->frameGeometry().x());
		settings.setValue("XDirect_y", m_pctrlXDirectWidget->frameGeometry().y());
		settings.setValue("AFoil_x", m_pctrlAFoilWidget->frameGeometry().x());
		settings.setValue("AFoil_y", m_pctrlAFoilWidget->frameGeometry().y());
		settings.setValue("XInverse_x", m_pctrlXInverseWidget->frameGeometry().x());
		settings.setValue("XInverse_y", m_pctrlXInverseWidget->frameGeometry().y());
		settings.setValue("StabView_x", m_pctrlStabViewWidget->frameGeometry().x());
		settings.setValue("StabView_y", m_pctrlStabViewWidget->frameGeometry().y());

		settings.setValue("XDirectSize", m_pctrlXDirectWidget->size());
		settings.setValue("AFoilSize", m_pctrlAFoilWidget->size());
		settings.setValue("XInverseSize", m_pctrlXInverseWidget->size());
		settings.setValue("MiarexSize", m_pctrlMiarexWidget->size());
		settings.setValue("StabSize", m_pctrlStabViewWidget->size());

		settings.setValue("ImageDirName", m_ImageDirName);
		settings.setValue("ExportLastDirName", m_ExportLastDirName);

		settings.setValue("LengthUnit",   Units::lengthUnitIndex());
		settings.setValue("AreaUnit",     Units::areaUnitIndex());
		settings.setValue("WeightUnit",   Units::weightUnitIndex());
		settings.setValue("SpeedUnit",    Units::speedUnitIndex());
		settings.setValue("ForceUnit",    Units::forceUnitIndex());
		settings.setValue("MomentUnit",   Units::momentUnitIndex());
		settings.setValue("PressureUnit", Units::pressureUnitIndex());
		settings.setValue("InertiaUnit",  Units::inertiaUnitIndex());

		settings.setValue("LanguageFilePath", s_LanguageFilePath);
		settings.setValue("ImageFormat", m_ImageFormat);
		settings.setValue("AutoSaveProject", m_bAutoSave);
		settings.setValue("AutoSaveInterval", m_SaveInterval);
		settings.setValue("AutoLoadLastProject",m_bAutoLoadLast);
		settings.setValue("SaveOpps", m_bSaveOpps);
		settings.setValue("SaveWOpps", m_bSaveWOpps);
		settings.setValue("RecentFileSize", m_RecentFiles.size());
		settings.setValue("ShowStyleSheets", Settings::s_bStyleSheets);
		settings.setValue("StyleSheetName", Settings::s_StyleSheetName);

		settings.setValue("ShowMousePosition", s_bShowMousePos);

		QString RecentF;
		for(int i=0; i<m_RecentFiles.size() && i<MAXRECENTFILES; i++)
		{
			RecentF = QString("RecentFile_%1").arg(i);
			if(m_RecentFiles[i].length()) settings.setValue(RecentF, m_RecentFiles.at(i));
			else                          settings.setValue(RecentF, "");
		}
		for(int i=m_RecentFiles.size(); i<MAXRECENTFILES; i++)
		{
			RecentF = QString("RecentFile_%1").arg(i);
			settings.setValue(RecentF, "");
		}
	}
	settings.endGroup();

	Settings::saveSettings(&settings);
	pAFoil->SaveSettings(&settings);
	pXDirect->saveSettings(&settings);
	pMiarex->saveSettings(&settings);
	pXInverse->saveSettings(&settings);
	GL3DScales::saveSettings(&settings);
	W3dPrefsDlg::saveSettings(&settings);
}


void MainFrame::setMainFrameCentralWidget()
{
	if(m_iApp==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
		if (pMiarex->m_iView==XFLR5::WOPPVIEW || pMiarex->m_iView==XFLR5::WPOLARVIEW || pMiarex->m_iView==XFLR5::WCPVIEW ||
			pMiarex->m_iView==XFLR5::STABPOLARVIEW  || pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			m_pctrlCentralWidget->setCurrentWidget(m_pMiarexTileWidget);
			pMiarex->setGraphTiles();
			m_pMiarexTileWidget->setFocus();
		}
		else if(pMiarex->m_iView==XFLR5::W3DVIEW)
		{
			m_pctrlCentralWidget->setCurrentWidget(m_pgl3Widget);
			m_pgl3Widget->setFocus();
		}
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		m_pctrlCentralWidget->setCurrentWidget(m_pDirect2dWidget);
		m_pDirect2dWidget->setFocus();
	}
	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		m_pctrlCentralWidget->setCurrentWidget(m_pXDirectTileWidget);
		pXDirect->setGraphTiles();
		m_pXDirectTileWidget->setFocus();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		m_pctrlCentralWidget->setCurrentWidget(m_p2dWidget);
		m_p2dWidget->setFocus();
	}
}


void MainFrame::selectFoil(void*pFoilPtr)
{
	Foil *pFoil = (Foil*)pFoilPtr;
	if(!m_pctrlFoil->count()) return;

	m_pctrlFoil->blockSignals(true);

	if (pFoil)
	{
		int pos = m_pctrlFoil->findText(pFoil->foilName());

		if (pos>=0) m_pctrlFoil->setCurrentIndex(pos);
		else        m_pctrlFoil->setCurrentIndex(0);
	}
	else
	{
		m_pctrlFoil->setCurrentIndex(0);
	}
	m_pctrlFoil->blockSignals(false);
}




void MainFrame::selectPolar(void*pPolarPtr)
{
	Polar *pPolar = (Polar*)pPolarPtr;
	if(!m_pctrlPolar->count()) return;

	m_pctrlPolar->blockSignals(true);

	if (pPolar)
	{
		int pos = m_pctrlPolar->findText(pPolar->polarName());

		if (pos>=0) m_pctrlPolar->setCurrentIndex(pos);
		else        m_pctrlPolar->setCurrentIndex(0);
	}
	else
	{
		m_pctrlPolar->setCurrentIndex(0);
	}

	m_pctrlPolar->blockSignals(false);
}



/**
 *Selects the operating point in the combobox and returns true
 *On error, selects the first and returns false
 */
void MainFrame::selectOpPoint(void *pOppPtr)
{
	Polar *pCurPlr = Polar::curPolar();
	OpPoint *pOpp = (OpPoint*)pOppPtr;
	if(!pOpp || !pCurPlr) return;
	if(!m_pctrlOpPoint->count()) return;

	QString strange;
	bool bOK;

	m_pctrlOpPoint->blockSignals(true);

	for(int i=0; i<m_pctrlOpPoint->count(); i++)
	{
		strange = m_pctrlOpPoint->itemText(i).trimmed();

		double val = locale().toDouble(strange, &bOK);

		if(pCurPlr->polarType() == XFOIL::FIXEDAOAPOLAR)
		{
			if(bOK && qAbs(val-pOpp->Reynolds())<1.0)
			{
				m_pctrlOpPoint->setCurrentIndex(i);
				break;
			}
		}
		else
		{
			if(bOK && qAbs(val-pOpp->aoa())<0.001)
			{
				m_pctrlOpPoint->setCurrentIndex(i);
				break;
			}
		}
	}
    m_pctrlOpPoint->blockSignals(false);
}


void MainFrame::selectPlane(void *pPlanePtr)
{
	if(!m_pctrlPlane->count()) return;
	m_pctrlPlane->blockSignals(true);

	Plane *pPlane = (Plane*)pPlanePtr;

	if(pPlane)
	{
		int pos = m_pctrlPlane->findText(pPlane->planeName());
		if(pos>=0) m_pctrlPlane->setCurrentIndex(pos);
		else if(m_pctrlPlane->count())
			m_pctrlPlane->setCurrentIndex(0);
	}
	else
	{
		m_pctrlPlane->setCurrentIndex(0);
	}

	m_pctrlPlane->blockSignals(false);
}


void MainFrame::selectWPolar(void *pWPolarPtr)
{
	if(!m_pctrlPlanePolar->count()) return;

	WPolar *pWPolar = (WPolar*)pWPolarPtr;

	m_pctrlPlanePolar->blockSignals(true);

	if(pWPolarPtr)
	{
		int pos = m_pctrlPlanePolar->findText(pWPolar->polarName());
		if (pos>=0) m_pctrlPlanePolar->setCurrentIndex(pos);
		else if(m_pctrlPlanePolar->count())
			m_pctrlPlanePolar->setCurrentIndex(0);
	}
	else
	{
		m_pctrlPlanePolar->setCurrentIndex(0);
	}
	m_pctrlPlanePolar->blockSignals(false);
}


void MainFrame::selectPlaneOpp(void *pPlaneOppPtr)
{
	double x = 0.0;
	PlaneOpp *pPlaneOpp = (PlaneOpp*)pPlaneOppPtr;

	if(pPlaneOpp)
	{
		if(pPlaneOpp->polarType()<XFLR5::FIXEDAOAPOLAR)        x = pPlaneOpp->m_Alpha;
		else if(pPlaneOpp->polarType()==XFLR5::FIXEDAOAPOLAR)  x = pPlaneOpp->m_QInf;
		else if(pPlaneOpp->polarType()==XFLR5::BETAPOLAR)      x = pPlaneOpp->m_Beta;
		else if(pPlaneOpp->polarType()==XFLR5::STABILITYPOLAR) x = pPlaneOpp->m_Ctrl;
	}
	//Selects a pOpp in the combobox
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	WPolar *pCurWPlr    = pMiarex->m_pCurWPolar;
	double val;
	bool bOK;
	QString strange;

	m_pctrlPlaneOpp->blockSignals(true);
//	x = (double)qRound(x*100.0)/100.0;

	for(int i=0; i<m_pctrlPlaneOpp->count(); i++)
	{
		strange = m_pctrlPlaneOpp->itemText(i);
		val = locale().toDouble(strange, &bOK);

		if(pCurWPlr->polarType()<XFLR5::FIXEDAOAPOLAR)
		{
			if(bOK && qAbs(val-x)<0.001)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
		else if(pCurWPlr->polarType()==XFLR5::FIXEDAOAPOLAR)
		{
			if(bOK && qAbs(val-x)<1.0)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
		else if(pCurWPlr->polarType()==XFLR5::BETAPOLAR)
		{
			if(bOK && qAbs(val-x)<0.001)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
		else if(pCurWPlr->polarType()==XFLR5::STABILITYPOLAR)
		{
			if(bOK && qAbs(val-x)<.001)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
	}
	m_pctrlPlaneOpp->blockSignals(false);
}





bool MainFrame::serializeProjectXFL(QDataStream &ar, bool bIsStoring)
{
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	WPolar *pWPolar = NULL;
	PlaneOpp *pPOpp = NULL;
	Plane *pPlane   = NULL;
	Polar *pPolar   = NULL;
	OpPoint *pOpp    = NULL;

	int i, n;

	if (bIsStoring)
	{
		// storing code
		int ArchiveFormat = 200001;
		ar << ArchiveFormat;
		// 200001 : First instance of new ".xfl" format

		//Save unit data
		ar << Units::lengthUnitIndex();
		ar << Units::areaUnitIndex();
		ar << Units::weightUnitIndex();
		ar << Units::speedUnitIndex();
		ar << Units::forceUnitIndex();
		ar << Units::momentUnitIndex();


		//Save default Polar data. Not in the Settings, since this is Project dependant
		if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
		else if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
		else if(WPolarDlg::s_WPolar.polarType()==XFLR5::FIXEDAOAPOLAR)   ar<<4;
		else if(WPolarDlg::s_WPolar.polarType()==XFLR5::BETAPOLAR)       ar<<5;
		else if(WPolarDlg::s_WPolar.polarType()==XFLR5::STABILITYPOLAR)  ar<<7;
		else ar << 0;

		if(WPolarDlg::s_WPolar.analysisMethod()==XFLR5::LLTMETHOD)        ar << 1;
		else if(WPolarDlg::s_WPolar.analysisMethod()==XFLR5::VLMMETHOD)   ar << 2;
		else if(WPolarDlg::s_WPolar.analysisMethod()==XFLR5::PANELMETHOD) ar << 3;
		else ar << 0;

		ar << WPolarDlg::s_WPolar.mass();
		ar << WPolarDlg::s_WPolar.m_QInfSpec;
		ar << WPolarDlg::s_WPolar.CoG().x;
		ar << WPolarDlg::s_WPolar.CoG().y;
		ar << WPolarDlg::s_WPolar.CoG().z;

		ar << WPolarDlg::s_WPolar.density();
		ar << WPolarDlg::s_WPolar.viscosity();
		ar << WPolarDlg::s_WPolar.m_AlphaSpec;
		ar << WPolarDlg::s_WPolar.m_BetaSpec;

		ar << WPolarDlg::s_WPolar.bTilted();
		ar << WPolarDlg::s_WPolar.bWakeRollUp();

		// save the planes...
		ar << Objects3D::s_oaPlane.size();
		for (i=0; i<Objects3D::s_oaPlane.size();i++)
		{
			pPlane = (Plane*)Objects3D::s_oaPlane.at(i);
			pPlane->serializePlaneXFL(ar, bIsStoring);
		}

		// save the WPolars
		ar << Objects3D::s_oaWPolar.size();
		for (i=0; i<Objects3D::s_oaWPolar.size();i++)
		{
			pWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
			pWPolar->serializeWPlrXFL(ar, bIsStoring);
		}

		if(m_bSaveWOpps)
		{
			// not forgetting their POpps
			ar << Objects3D::s_oaPOpp.size();
			for (i=0; i<Objects3D::s_oaPOpp.size();i++)
			{
				pPOpp = (PlaneOpp*)Objects3D::s_oaPOpp.at(i);
				pPOpp->serializePOppXFL(ar, bIsStoring);
			}
		}
		else ar << 0;

		// then the foils
		ar << Foil::s_oaFoil.size();
		for(i=0; i<Foil::s_oaFoil.size(); i++)
		{
			Foil *pFoil = (Foil*)Foil::s_oaFoil.at(i);
			pFoil->serializeFoilXFL(ar, bIsStoring);
		}

		//the foil polars
		ar << Polar::s_oaPolar.size();
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			pPolar->serializePolarXFL(ar, bIsStoring);
		}

		//the oppoints
		if(m_bSaveOpps)
		{
			ar << OpPoint::s_oaOpp.size();
			for (i=0; i<OpPoint::s_oaOpp.size();i++)
			{
				pOpp = (OpPoint*)OpPoint::s_oaOpp.at(i);
				pOpp->serializeOppXFL(ar, bIsStoring);
			}
		}
		else ar << 0;

		// and the spline foil whilst we're at it
		pAFoil->m_pSF->serializeXFL(ar, bIsStoring);

		ar << Units::pressureUnitIndex();
		ar << Units::inertiaUnitIndex();
		//add provisions
		// space allocation for the future storage of more data, without need to change the format
		for (int i=2; i<20; i++) ar << 0;
		for (int i=0; i<50; i++) ar << (double)0.0;
	}
	else
	{
		// LOADING CODE

		int ArchiveFormat;
		ar >> ArchiveFormat;
		if(ArchiveFormat!=200001) return false;

		//Load unit data
		ar >> n; Units::setLengthUnitIndex(n);
		ar >> n; Units::setAreaUnitIndex(n);
		ar >> n; Units::setWeightUnitIndex(n);
		ar >> n; Units::setSpeedUnitIndex(n);
		ar >> n; Units::setForceUnitIndex(n);
		ar >> n; Units::setMomentUnitIndex(n);
		//pressure and inertia units are added later on in the provisions.


		Units::setUnitConversionFactors();


		//Load the default Polar data. Not in the Settings, since this is Project dependant
		ar >> n;
		if(n==1)      WPolarDlg::s_WPolar.polarType()=XFLR5::FIXEDSPEEDPOLAR;
		else if(n==2) WPolarDlg::s_WPolar.polarType()=XFLR5::FIXEDLIFTPOLAR;
		else if(n==4) WPolarDlg::s_WPolar.polarType()=XFLR5::FIXEDAOAPOLAR;
		else if(n==5) WPolarDlg::s_WPolar.polarType()=XFLR5::BETAPOLAR;
		else if(n==7) WPolarDlg::s_WPolar.polarType()=XFLR5::STABILITYPOLAR;

		ar >> n;
		if(n==1)      WPolarDlg::s_WPolar.analysisMethod()=XFLR5::LLTMETHOD;
		else if(n==2) WPolarDlg::s_WPolar.analysisMethod()=XFLR5::VLMMETHOD;
		else if(n==3) WPolarDlg::s_WPolar.analysisMethod()=XFLR5::PANELMETHOD;

		ar >> WPolarDlg::s_WPolar.mass();
		ar >> WPolarDlg::s_WPolar.m_QInfSpec;
		ar >> WPolarDlg::s_WPolar.CoG().x;
		ar >> WPolarDlg::s_WPolar.CoG().y;
		ar >> WPolarDlg::s_WPolar.CoG().z;

		ar >> WPolarDlg::s_WPolar.density();
		ar >> WPolarDlg::s_WPolar.viscosity();
		ar >> WPolarDlg::s_WPolar.m_AlphaSpec;
		ar >> WPolarDlg::s_WPolar.m_BetaSpec;

		ar >> WPolarDlg::s_WPolar.bTilted();
		ar >> WPolarDlg::s_WPolar.bWakeRollUp();

		// load the planes...
		// assumes all object have been deleted and the array cleared.
		ar >> n;
		for(i=0; i<n; i++)
		{
			pPlane = new Plane();
			if(pPlane->serializePlaneXFL(ar, bIsStoring)) Objects3D::s_oaPlane.append(pPlane);
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// load the WPolars
		ar >> n;
		for(i=0; i<n; i++)
		{
			pWPolar = new WPolar();
			if(pWPolar->serializeWPlrXFL(ar, bIsStoring))
			{
				// clean up : the project may be carrying useless WPolars due to past programming errors
				pPlane = Objects3D::getPlane(pWPolar->planeName());
				if(pPlane)
				{
					Objects3D::s_oaWPolar.append(pWPolar);
					if(pWPolar->referenceDim()==XFLR5::PLANFORMREFDIM)
					{
						pWPolar->referenceArea()       = pPlane->planformArea();
						pWPolar->referenceSpanLength() = pPlane->planformSpan();
						if(pPlane->BiPlane()) pWPolar->referenceArea() += pPlane->wing2()->m_PlanformArea;
						pWPolar->referenceChordLength() = pPlane->mac();
					}
					else if(pWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM)
					{
						pWPolar->referenceArea()       = pPlane->projectedArea();
						pWPolar->referenceSpanLength() = pPlane->projectedSpan();
						if(pPlane->BiPlane()) pWPolar->referenceArea() += pPlane->wing2()->m_ProjectedArea;
						pWPolar->referenceChordLength() = pPlane->mac();
					}
				}
				else
				{
				}
			}
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// the PlaneOpps
		ar >> n;
		for(i=0; i<n; i++)
		{
			pPOpp = new PlaneOpp();
			if(pPOpp->serializePOppXFL(ar, bIsStoring))
			{
				//just append, since POpps have been sorted when first inserted
				pPlane = Objects3D::getPlane(pPOpp->planeName());
				pWPolar = Objects3D::getWPolar(pPlane, pPOpp->polarName());

				// clean up : the project may be carrying useless PlaneOpps due to past programming errors
				if(pPlane && pWPolar) Objects3D::s_oaPOpp.append(pPOpp);
				else
				{
				}
			}
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// load the Foils
		ar >> n;
		for(i=0; i<n; i++)
		{
			Foil *pFoil = new Foil();
			if(pFoil->serializeFoilXFL(ar, bIsStoring))
			{
				// delete any former foil with that name - necessary in the case of project insertion to avoid duplication
				// there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
				Foil *pOldFoil = Foil::foil(pFoil->foilName());
				if(pOldFoil) Foil::deleteFoil(pOldFoil);
				Foil::s_oaFoil.append(pFoil);
			}
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// load the Polars
		ar >> n;
		for(i=0; i<n; i++)
		{
			pPolar = new Polar();
			if(pPolar->serializePolarXFL(ar, bIsStoring)) Polar::s_oaPolar.append(pPolar);
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// OpPoints
		ar >> n;
		for(i=0; i<n; i++)
		{
			pOpp = new OpPoint();
			if(pOpp->serializeOppXFL(ar, bIsStoring))  OpPoint::s_oaOpp.append(pOpp);
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}


		// and the spline foil whilst we're at it
		pAFoil->m_pSF->serializeXFL(ar, bIsStoring);

		ar >> n; Units::setPressureUnitIndex(n);
		ar >> n; Units::setInertiaUnitIndex(n);

		// space allocation
		int k ;
		double dble;
		for (int i=2; i<20; i++) ar >> k;
		for (int i=0; i<50; i++) ar >> dble;
	}

	return true;
}



bool MainFrame::serializeProjectWPA(QDataStream &ar, bool bIsStoring)
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	Wing *pWing     = NULL;
	WPolar *pWPolar = NULL;
	WingOpp *pWOpp  = NULL;
	PlaneOpp *pPOpp = NULL;
	Plane *pPlane   = NULL;
	Body *pBody     = NULL;
	Polar *pPolar   = NULL;
	Foil *pFoil     = NULL;

	QString str;
	int i, n, j, k;
	float f;

	if (bIsStoring)
	{
		//not storing to .wpa format anymore
		return true;
	}
	else
	{
		// LOADING CODE

		int ArchiveFormat;
		ar >> n;

		if(n<100000)
		{
		// then n is the number of wings to load
		// up to v1.99beta15
			ArchiveFormat = 100000;
		}
		else
		{
		// then n is the ArchiveFormat number
			ArchiveFormat = n;
			ar >> n; Units::setLengthUnitIndex(n);
			ar >> n; Units::setAreaUnitIndex(n);
			ar >> n; Units::setWeightUnitIndex(n);
			ar >> n; Units::setSpeedUnitIndex(n);
			ar >> n; Units::setForceUnitIndex(n);

			if(ArchiveFormat>=100005)
			{
				ar >> n; Units::setMomentUnitIndex(n);
			}

			Units::setUnitConversionFactors();

			if(ArchiveFormat>=100004)
			{
				ar >>k;
				if(k==1)      WPolarDlg::s_WPolar.polarType() = XFLR5::FIXEDSPEEDPOLAR;
				else if(k==2) WPolarDlg::s_WPolar.polarType() = XFLR5::FIXEDLIFTPOLAR;
				else if(k==4) WPolarDlg::s_WPolar.polarType() = XFLR5::FIXEDAOAPOLAR;
				else if(k==5) WPolarDlg::s_WPolar.polarType() = XFLR5::BETAPOLAR;
				else if(k==7) WPolarDlg::s_WPolar.polarType() = XFLR5::STABILITYPOLAR;

				ar >> f; WPolarDlg::s_WPolar.mass()=f;
				ar >> f; WPolarDlg::s_WPolar.m_QInfSpec=f;
				if(ArchiveFormat>=100013)
				{
					ar >> f; WPolarDlg::s_WPolar.CoG().x=f;
					ar >> f; WPolarDlg::s_WPolar.CoG().y=f;
					ar >> f; WPolarDlg::s_WPolar.CoG().z=f;
				}
				else
				{
					ar >> f; WPolarDlg::s_WPolar.CoG().x=f;
					WPolarDlg::s_WPolar.CoG().y=0;
					WPolarDlg::s_WPolar.CoG().z=0;
				}
				if(ArchiveFormat<100010) WPolarDlg::s_WPolar.CoG().x=f/1000.0;
				ar >> f; WPolarDlg::s_WPolar.density()   = f;
				ar >> f; WPolarDlg::s_WPolar.viscosity() = f;
				ar >> f; WPolarDlg::s_WPolar.m_AlphaSpec     = f;
				if(ArchiveFormat>=100012)
				{
					ar >>f; WPolarDlg::s_WPolar.m_BetaSpec=f;
				}

				ar >> k;
				if(k==1)      WPolarDlg::s_WPolar.analysisMethod()=XFLR5::LLTMETHOD;
				else if(k==2) WPolarDlg::s_WPolar.analysisMethod()=XFLR5::VLMMETHOD;
				else if(k==3) WPolarDlg::s_WPolar.analysisMethod()=XFLR5::PANELMETHOD;
			}
			if(ArchiveFormat>=100006)
			{
				ar >> k;
				if (k) WPolarDlg::s_WPolar.bVLM1() = true;
				else   WPolarDlg::s_WPolar.bVLM1() = false;

				ar >> k;
			}

			if(ArchiveFormat>=100008)
			{
				ar >> k;
				if (k) WPolarDlg::s_WPolar.bTilted() = true;
				else   WPolarDlg::s_WPolar.bTilted() = false;

				ar >> k;
				if (k) WPolarDlg::s_WPolar.bWakeRollUp() = true;
				else   WPolarDlg::s_WPolar.bWakeRollUp() = false;
			}
			// and read n again
			ar >> n;
		}

		// WINGS FIRST
		for (i=0;i<n; i++)
		{
			pWing = new Wing;

			if (!pWing->serializeWingWPA(ar, bIsStoring))
			{
				if(pWing) delete pWing;
				return false;
			}
			if(pWing)
			{
				//create a plane with this wing
				pPlane = new Plane();
				pPlane->setPlaneName(pWing->wingName());
				pPlane->m_Wing[0].duplicate(pWing);
				pPlane->setBody(NULL);
				pPlane->setWings(false, false, false);
				Objects3D::addPlane(pPlane);
				delete pWing;
			}
		}

		//THEN WPOLARS

		ar >> n;// number of WPolars to load
		bool bWPolarOK;

		for (i=0;i<n; i++)
		{
			pWPolar = new WPolar;
			bWPolarOK = pWPolar->serializeWPlrWPA(ar, bIsStoring);
			//force compatibilty
			if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD && pWPolar->polarType()==XFLR5::STABILITYPOLAR) pWPolar->bThinSurfaces() = true;

			if (!bWPolarOK)
			{
				if(pWPolar) delete pWPolar;
				return false;
			}
			if(!pWPolar->analysisMethod()==XFLR5::LLTMETHOD && ArchiveFormat <100003)	pWPolar->clearData();//former VLM version was flawed
//			if(pWPolar->polarType()==STABILITYPOLAR)	pWPolar->bThinSurfaces() = true;

			if(pWPolar->polarFormat()!=1020 || pWPolar->polarType()!=XFLR5::STABILITYPOLAR) Objects3D::addWPolar(pWPolar);
		}

		//THEN WOPPS
		ar >> n;// number of WOpps to load
		bool bWOppOK;
		for (i=0;i<n; i++)
		{
			pWOpp = new WingOpp();
			if (ArchiveFormat<=100001)
			{
				return false;
			}
			else
			{
				bWOppOK = pWOpp->serializeWingOppWPA(ar, bIsStoring);
				if (!bWOppOK)
				{
					if(pWOpp) delete pWOpp;
					return false;
				}
			}

			delete pWOpp;
		}

//		ar >> n;
		//=100000 ... unused
		//THEN FOILS, POLARS and OPPS
		if(ArchiveFormat>=100009)
		{
			if(!loadPolarFileV3(ar, bIsStoring,100002))
			{
				return false;
			}
		}
		else
		{
			if(ArchiveFormat>=100006)
			{
				if(!loadPolarFileV3(ar, bIsStoring))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		if(n==100000)
		{
			for (j=0; j<Polar::s_oaPolar.size(); j++)
			{
				pPolar = (Polar*)Polar::s_oaPolar.at(j);
				for (k=0; k<Foil::s_oaFoil.size(); k++)
				{
					pFoil = (Foil*)Foil::s_oaFoil.at(k);
					if(pFoil->foilName()==pPolar->foilName())
					{
						pPolar->NCrit()      = 9.0;
						pPolar->XtrTop()     = 1.0;
						pPolar->XtrBot()     = 1.0;
						str = QString("_N%1").arg(9.0,4,'f',1);
						pPolar->polarName() += str;
						break;
					}
				}
			}
		}

		if(ArchiveFormat>=100011)
		{
			ar >> n;// number of Bodies to load
			for (i=0;i<n; i++)
			{
				pBody = new Body();

				if (pBody->serializeBodyWPA(ar, bIsStoring))
				{
					Objects3D::s_oaBody.append(pBody);

				}
				else
				{
					if(pBody) delete pBody;
					return false;
				}
			}
		}
		if(ArchiveFormat>=100006)
		{ //read the planes
			ar >> n;
			for (i=0; i<n;i++)
			{
				pPlane = new Plane();
				if(pPlane)
				{
					if(pPlane->serializePlaneWPA(ar, bIsStoring))
					{
						Objects3D::addPlane(pPlane);
						if(pPlane->bodyName().length()) pPlane->body()->duplicate(Objects3D::getBody(pPlane->bodyName()));
					}
					else
					{
						if(pPlane) delete pPlane;
						return false;
					}
				}
			}

			// attach the body pointers to the Plane objects
			for (int ib=0; ib<Objects3D::s_oaBody.size(); ib++)
			{
				Body *pBody = (Body*)Objects3D::s_oaBody.at(ib);
				Objects3D::addBody(pBody);
			}


			//and their pPolars
			if(ArchiveFormat <100007)
			{
				ar >> n;// number of WPolars to load
				for (i=0;i<n; i++)
				{
					pWPolar = new WPolar();

					if (!pWPolar->serializeWPlrWPA(ar, bIsStoring))
					{
						if(pWPolar) delete pWPolar;
						return false;
					}
					if(!pWPolar->analysisMethod()==XFLR5::LLTMETHOD && ArchiveFormat <100003)
						pWPolar->clearData();
					Objects3D::addWPolar(pWPolar);
				}
			}

			ar >> n;// number of PlaneOpps to load
			for (i=0;i<n; i++)
			{
				pPOpp = new PlaneOpp();

				if (!pPOpp->serializePOppWPA(ar, bIsStoring))
				{
					if(pPOpp) delete pPOpp;
					return false;
				}
				Objects3D::insertPOpp(pPOpp);
//				Objects3D::s_oaPOpp.append(pPOpp);
			}
		}
		pMiarex->m_pCurPOpp = NULL;

		pAFoil->m_pSF->serialize(ar, bIsStoring);

		if(m_iApp==XFLR5::MIAREX) pMiarex->setPlane();

		pMiarex->updateUnits();

		return true;
	}
}



void MainFrame::setMenus()
{
	if(m_iApp==XFLR5::NOAPP)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pXDirectViewMenu);
		menuBar()->addMenu(m_pXDirectFoilMenu);
		menuBar()->addMenu(m_pDesignMenu);
		menuBar()->addMenu(m_pXFoilAnalysisMenu);
		menuBar()->addMenu(m_pPolarMenu);
		menuBar()->addMenu(m_pOpPointMenu);
		menuBar()->addMenu(m_pGraphMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pXInverseViewMenu);
		menuBar()->addMenu(m_pXInverseGraphMenu);
		menuBar()->addMenu(m_pXInverseFoilMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pAFoilViewMenu);
		menuBar()->addMenu(m_pAFoilDesignMenu);
		menuBar()->addMenu(m_pAFoilSplineMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp== XFLR5::MIAREX)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pMiarexViewMenu);
		menuBar()->addMenu(m_pPlaneMenu);
		menuBar()->addMenu(m_pMiarexWPlrMenu);
		menuBar()->addMenu(m_pMiarexWOppMenu);
		menuBar()->addMenu(m_pMiarexAnalysisMenu);
		menuBar()->addMenu(m_pGraphMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
}


void MainFrame::setProjectName(QString PathName)
{
	m_FileName = PathName;
	int pos = PathName.lastIndexOf("/");
	if (pos>0) s_ProjectName = PathName.right(PathName.length()-pos-1);
	else       s_ProjectName = PathName;

	if(s_ProjectName.length()>4)
	{
		s_ProjectName = s_ProjectName.left(s_ProjectName.length()-4);
		m_pctrlProjectName->setText(s_ProjectName);
	}
}


void MainFrame::setSaveState(bool bSave)
{
	s_bSaved = bSave;
	if(bSave)
		m_pctrlProjectName->setText(s_ProjectName);
	else
		m_pctrlProjectName->setText(s_ProjectName+ "*");
}



void MainFrame::setGraphSettings(QGraph *pGraph)
{
	QXDirect *pXDirect   = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)m_pMiarex;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	pXDirect->m_CpGraph.copySettings(pGraph, false);
	for(int ig=0; ig<qMax(MAXPOLARGRAPHS,pXDirect->m_PlrGraph.count()); ig++) pXDirect->m_PlrGraph[ig]->copySettings(pGraph, false);

	pXInverse->m_QGraph.copySettings(pGraph, false);

	pMiarex->m_CpGraph.copySettings(pGraph, false);

	for(int ig=0; ig<pMiarex->m_WingGraph.count(); ig++) pMiarex->m_WingGraph[ig]->copySettings(pGraph, false);
	for(int ig=0; ig<pMiarex->m_WPlrGraph.count(); ig++) pMiarex->m_WPlrGraph[ig]->copySettings(pGraph, false);
	for(int ig=0; ig<pMiarex->m_TimeGraph.count(); ig++) pMiarex->m_TimeGraph[ig]->copySettings(pGraph, false);
	for(int ig=0; ig<pMiarex->m_StabPlrGraph.count(); ig++) pMiarex->m_StabPlrGraph[ig]->copySettings(pGraph, false);

}



QString MainFrame::shortenFileName(QString &PathName)
{
	QString strong, strange;
	if(PathName.length()>60)
	{
		int pos = PathName.lastIndexOf('/');
		if (pos>0)
		{
			strong = '/'+PathName.right(PathName.length()-pos-1);
			strange = PathName.left(60-strong.length()-6);
			pos = strange.lastIndexOf('/');
			if(pos>0) strange = strange.left(pos)+"/...  ";
			strong = strange+strong;
		}
		else
		{
			strong = PathName.left(40);
		}
	}
	else strong = PathName;
	return strong;
}

/**
* Fills the combobox with  plane names.
* then selects the current wing or plane, if any;
* else selects the first, if any;
* else disables the combobox.
*/
void MainFrame::updatePlaneListBox()
{
	int i;
	QMiarex *pMiarex= (QMiarex*)m_pMiarex;
	Plane *pPlane, *pCurPlane;

	m_pctrlPlane->blockSignals(true);
	m_pctrlPlane->clear();

	pCurPlane = pMiarex->m_pCurPlane;
	QStringList PlaneNames;
	for (i=0; i<Objects3D::s_oaPlane.size(); i++)
	{
		pPlane = (Plane*)Objects3D::s_oaPlane[i];
		PlaneNames.append(pPlane->planeName());
	}

#if QT_VERSION >= 0x050000
	PlaneNames.sort(Qt::CaseInsensitive);
#else
	PlaneNames.sort();
#endif

	m_pctrlPlane->addItems(PlaneNames);

	//select the current Plane, if any...
	if(pCurPlane) selectPlane(pCurPlane);

	m_pctrlPlane->setEnabled(m_pctrlPlane->count());
	m_pctrlPlane->blockSignals(false);

	updateWPolarListBox();
}



void MainFrame::updateWPolarListBox()
{
//	fills the combobox with WPolar names associated to Miarex's current wing
//	then selects Miarex current WPolar if any, else selects the first, if any
//	else disables the combobox
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	WPolar *pWPolar;
	QString PlaneName;
	int i;

	m_pctrlPlanePolar->blockSignals(true);
	m_pctrlPlanePolar->clear();

	Plane  *pCurPlane = pMiarex->m_pCurPlane;
	WPolar *pCurWPlr  = pMiarex->m_pCurWPolar;

	if(pCurPlane)      PlaneName = pCurPlane->planeName();
	else               PlaneName = "";

	if(!PlaneName.length())
	{
		pMiarex->m_pCurWPolar = NULL;
		m_pctrlPlanePolar->setEnabled(false);
		m_pctrlPlaneOpp->setEnabled(false);
		return;
	}

	for (i=0; i<Objects3D::s_oaWPolar.size(); i++)
	{
		pWPolar = (WPolar*)Objects3D::s_oaWPolar[i];
		if(pWPolar->planeName() == PlaneName)
		{
			m_pctrlPlanePolar->addItem(pWPolar->polarName());
		}
	}

    m_pctrlPlanePolar->setEnabled(m_pctrlPlanePolar->count());

	if(pCurWPlr) selectWPolar(pCurWPlr);

	m_pctrlPlanePolar->blockSignals(false);
	updatePOppListBox();
}


/**
 * Fills the combobox with the WOpp parameters associated to Miarex' current WPLr,
 * then selects the current WingOpp or PlaneOpp if any, else selects the first, if any,
 * else disables the combobox.
 */
void MainFrame::updatePOppListBox()
{
	m_pctrlPlaneOpp->blockSignals(true);
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;

	PlaneOpp *pPOpp;
	Plane  *pCurPlane   = pMiarex->m_pCurPlane;
	WPolar *pCurWPlr    = pMiarex->m_pCurWPolar;

	QString str;
	m_pctrlPlaneOpp->clear();


	if (!pCurPlane || !pCurPlane->planeName().length() || !pCurWPlr || !pCurWPlr->polarName().length())
	{
		m_pctrlPlaneOpp->setEnabled(false);
		return;
	}

	for (int iPOpp=0; iPOpp<Objects3D::s_oaPOpp.size(); iPOpp++)
	{
		pPOpp = (PlaneOpp*)Objects3D::s_oaPOpp.at(iPOpp);
		if (pPOpp->planeName()==pCurPlane->planeName() && pPOpp->polarName()==pCurWPlr->polarName())
		{
			if(pCurWPlr->polarType()<XFLR5::FIXEDAOAPOLAR)        str = QString("%L1").arg(pPOpp->m_Alpha,8,'f',3);
			else if(pCurWPlr->polarType()==XFLR5::FIXEDAOAPOLAR)  str = QString("%L1").arg(pPOpp->m_QInf,8,'f',3);
			else if(pCurWPlr->polarType()==XFLR5::BETAPOLAR)      str = QString("%L1").arg(pPOpp->m_Beta,8,'f',3);
			else if(pCurWPlr->polarType()==XFLR5::STABILITYPOLAR) str = QString("%L1").arg(pPOpp->m_Ctrl,8,'f',3);
			m_pctrlPlaneOpp->addItem(str);
		}
	}

	if(pMiarex->m_pCurPOpp) selectPlaneOpp(pMiarex->m_pCurPOpp);
	else                    m_pctrlPlaneOpp->setCurrentIndex(0);

	// otherwise disable control
	m_pctrlPlaneOpp->setEnabled(m_pctrlPlaneOpp->count());

	m_pctrlPlaneOpp->blockSignals(false);
}


/**
 * Fills the combobox with the Foil names,
 * then selects the current Foil if any, else selects the first, if any,
 * else disables the combobox
 */
void MainFrame::updateFoilListBox()
{
	m_pctrlFoil->blockSignals(true);
	m_pctrlFoil->clear();

	QStringList foilList;
	for (int iFoil=0; iFoil<Foil::s_oaFoil.size(); iFoil++)
	{
		Foil *pFoil = (Foil*)Foil::s_oaFoil[iFoil];
		foilList.append(pFoil->foilName());
	}

#if QT_VERSION >= 0x050000
	foilList.sort(Qt::CaseInsensitive);
#else
	foilList.sort();
#endif

	m_pctrlFoil->addItems(foilList);
	m_pctrlFoil->setEnabled(m_pctrlFoil->count());
	selectFoil(Foil::curFoil());

	m_pctrlFoil->blockSignals(false);

	updatePolarListBox();
}


/**
 * Fills the combobox with polar names associated to the current foil,
 * then selects XDirect current polar if any, else selects the first, if any,
 * else disables the combobox,
 */
void MainFrame::updatePolarListBox()
{
	m_pctrlPolar->blockSignals(true);
	m_pctrlPolar->clear();

	if(!Foil::curFoil())
	{
	}
	else
	{
		QStringList polarList;
		for (int i=0; i<Polar::s_oaPolar.size(); i++)
		{
			Polar *pPolar = (Polar*)Polar::s_oaPolar[i];
			if(pPolar->foilName() == Foil::curFoil()->foilName())
			{
				polarList.append(pPolar->polarName());
			}
		}

#if QT_VERSION >= 0x050000
		polarList.sort(Qt::CaseInsensitive);
#else
		polarList.sort();
#endif

		m_pctrlPolar->addItems(polarList);
	}

	m_pctrlPolar->setEnabled(m_pctrlPolar->count());
	selectPolar(Polar::curPolar());

	m_pctrlPolar->blockSignals(false);

	updateOppListBox();
}


/**
 * Fills the combobox with the OpPoint values associated to the current foil,
 * then selects the current OpPoint if any, else selects the first, if any,
 * else disables the combobox.
 */
void MainFrame::updateOppListBox()
{
	m_pctrlOpPoint->blockSignals(true);
	m_pctrlOpPoint->clear();

	Polar *pCurPlr    = Polar::curPolar();

	if (!Foil::curFoil()  || !pCurPlr)
	{
		m_pctrlOpPoint->clear();
		m_pctrlOpPoint->setEnabled(false);
		m_pctrlOpPoint->blockSignals(false);
	}
	else
	{
		QStringList oppList;
		QString str;
		for (int iOpp=0; iOpp<OpPoint::s_oaOpp.size(); iOpp++)
		{
			OpPoint *pOpp = (OpPoint*)OpPoint::s_oaOpp[iOpp];
			if (pOpp->foilName()==Foil::curFoil()->foilName() && pOpp->polarName()==pCurPlr->polarName())
			{
				if (pCurPlr->polarType() != XFOIL::FIXEDAOAPOLAR)
					str = QString("%L1").arg(pOpp->aoa(),8,'f',3);
				else
					str = QString("%L1").arg(pOpp->Reynolds(),8,'f',0);
				oppList.append(str);
			}
		}
/*
#if QT_VERSION >= 0x050000
		oppList.sort(Qt::CaseInsensitive);
#else
		oppList.sort();
#endif
*/
		m_pctrlOpPoint->addItems(oppList);
	}

	m_pctrlOpPoint->setEnabled(m_pctrlOpPoint->count());
	selectOpPoint(OpPoint::curOpp());

	m_pctrlOpPoint->blockSignals(false);
}



void MainFrame::updateRecentFileActions()
{
	int numRecentFiles = qMin(m_RecentFiles.size(), MAXRECENTFILES);

	QString text;
	for (int i=0; i<numRecentFiles; ++i)
	{
		text = tr("&%1 %2").arg(i + 1).arg(shortenFileName(m_RecentFiles[i]));
		if(i==0) text +="\tCtrl+7";

		m_pRecentFileActs[i]->setText(text);
		m_pRecentFileActs[i]->setData(m_RecentFiles[i]);
		m_pRecentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)
		m_pRecentFileActs[j]->setVisible(false);

	m_pSeparatorAct->setVisible(numRecentFiles > 0);
}



void MainFrame::updateView()
{
	switch(m_iApp)
	{
		case XFLR5::XFOILANALYSIS:
		{
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pXDirect->updateView();
			break;
		}
		case XFLR5::DIRECTDESIGN:
		{
			m_pDirect2dWidget->update();
			break;
		}
		case XFLR5::MIAREX:
		{
			QMiarex *pMiarex= (QMiarex*)m_pMiarex;
			pMiarex->updateView();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->updateView();
			break;
		}
		default:
			break;
	}
}


void MainFrame::writePolars(QDataStream &ar, void *pFoilPtr)
{
	Foil *pFoil = (Foil*)pFoilPtr;

	int i;
	if(!pFoil)
	{
		int ArchiveFormat = 100003;
		ar << ArchiveFormat;
		//100003 : added foil comment
		//100002 : means we are serializings opps in the new numbered format
		//100001 : transferred NCrit, XTopTr, XBotTr to polar file
		//first write foils
		ar << Foil::s_oaFoil.size();

		for (i=0; i<Foil::s_oaFoil.size(); i++)
		{
			pFoil = (Foil*)Foil::s_oaFoil.at(i);
			pFoil->serialize(ar, true);
		}

		//then write polars
		ar << Polar::s_oaPolar.size();
		Polar * pPolar ;
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			pPolar->serialize(ar, true);
		}
	}
	else
	{
		ar << 100003;
		//100003 : added foil comment
		//100002 : means we are serializings opps in the new numbered format
		//100001 : transferred NCrit, XTopTr, XBotTr to polar file
		//first write foil
		ar << 1;//only one this time
		pFoil->serialize(ar,true);
		//count polars associated to the foil
		Polar * pPolar ;
		int n=0;
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			if (pPolar->foilName() == pFoil->foilName()) n++;
		}
		//then write polars
		ar << n;
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			if (pPolar->foilName() == pFoil->foilName()) pPolar->serialize(ar, true);
		}		
	}

	if(m_bSaveOpps)
	{
		ar << OpPoint::s_oaOpp.size();
		OpPoint * pOpp ;
		for (i=0; i<OpPoint::s_oaOpp.size();i++)
		{
			pOpp = (OpPoint*)OpPoint::s_oaOpp.at(i);
			pOpp->serializeOppWPA(ar,true,100002);
		}
	}
	else ar << 0;

}

 

void MainFrame::setupDataDir()
{
#ifdef Q_OS_MAC
	s_TranslationDir.setPath(qApp->applicationDirPath()+"/translations/");
	s_StylesheetDir.setPath(qApp->applicationDirPath()+"/qss/");
#endif
#ifdef Q_OS_WIN
    s_TranslationDir.setPath(qApp->applicationDirPath()+"/translations");
    s_StylesheetDir.setPath(qApp->applicationDirPath()+"/qss");
#endif
#ifdef Q_OS_LINUX
	s_TranslationDir.setPath("/usr/share/xflr5/translations");
	s_StylesheetDir.setPath("/usr/share/xflr5/qss");
#endif

}


void MainFrame::readStyleSheet(QString styleSheetName, QString &styleSheet)
{
	QFile file(s_StylesheetDir.canonicalPath()+"/"+styleSheetName+".qss");

    if(file.open(QFile::ReadOnly))
	{
		styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(styleSheet);
	}
}



void MainFrame::onProjectModified()
{
	setSaveState(false);
}



/**
 * The user has requested the launch of the interface to manage Foil objects.
 */
void MainFrame::onManageFoils()
{
	ManageFoilsDlg mfDlg(this);

	QString FoilName = "";
	if(Foil::curFoil()) FoilName = Foil::curFoil()->foilName();
	mfDlg.initDialog(FoilName);
	mfDlg.exec();

	if(mfDlg.m_bChanged) setSaveState(false);

	if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->setFoil(mfDlg.m_pFoil);
		updateFoilListBox();
		pXDirect->setControls();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->fillFoilTable();
		pAFoil->selectFoil();
	}

	updateView();
}



void MainFrame::checkGraphActions()
{
	for(int ig=0; ig<MAXGRAPHS; ig++)	m_pSingleGraph[ig]->setChecked(false);
	m_pTwoGraphs->setChecked(false);
	m_pFourGraphs->setChecked(false);
	m_pAllGraphs->setChecked(false);
	m_pSingleGraph[0]->setEnabled(true);

	if(m_iApp==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
		switch(pMiarex->m_iView)
		{
			case XFLR5::WOPPVIEW:
			{
				if(pMiarex->m_iWingView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
				else if(pMiarex->m_iWingView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
				else if(pMiarex->m_iWingView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
				else if(pMiarex->m_iWingView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
				m_pSingleGraph[1]->setEnabled(true);
				m_pSingleGraph[2]->setEnabled(true);
				m_pSingleGraph[3]->setEnabled(true);
				m_pSingleGraph[4]->setEnabled(true);
				m_pSingleGraph[5]->setEnabled(true);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(true);
				m_pAllGraphs->setEnabled(true);
				break;
			}
			case XFLR5::WPOLARVIEW:
			{
				if(pMiarex->m_iWPlrView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
				else if(pMiarex->m_iWPlrView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
				else if(pMiarex->m_iWPlrView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
				else if(pMiarex->m_iWPlrView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
				m_pSingleGraph[1]->setEnabled(true);
				m_pSingleGraph[2]->setEnabled(true);
				m_pSingleGraph[3]->setEnabled(true);
				m_pSingleGraph[4]->setEnabled(true);
				m_pSingleGraph[5]->setEnabled(true);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(true);
				m_pAllGraphs->setEnabled(true);
				break;
			}
			case XFLR5::STABPOLARVIEW:
			{
				if(pMiarex->m_bLongitudinal== XFLR5::ONEGRAPH)      m_pSingleGraph[0]->setChecked(true);
				else if(pMiarex->m_bLongitudinal== XFLR5::ONEGRAPH) m_pSingleGraph[1]->setChecked(true);
				m_pSingleGraph[2]->setEnabled(false);
				m_pSingleGraph[3]->setEnabled(false);
				m_pSingleGraph[4]->setEnabled(false);
				m_pSingleGraph[5]->setEnabled(false);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(false);
				m_pAllGraphs->setEnabled(false);
				break;
			}
			case XFLR5::STABTIMEVIEW:
			{
				if(pMiarex->m_iStabTimeView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
				else if(pMiarex->m_iStabTimeView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
				else if(pMiarex->m_iStabTimeView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
				else if(pMiarex->m_iStabTimeView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
				m_pSingleGraph[1]->setEnabled(true);
				m_pSingleGraph[2]->setEnabled(true);
				m_pSingleGraph[3]->setEnabled(true);
				m_pSingleGraph[4]->setEnabled(false);
				m_pSingleGraph[5]->setEnabled(false);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(true);
				m_pAllGraphs->setEnabled(false);
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else if (m_iApp==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		if(!pXDirect->m_bPolarView)
		{
			m_pSingleGraph[0]->setChecked(true);

			m_pSingleGraph[1]->setEnabled(false);
			m_pSingleGraph[2]->setEnabled(false);
			m_pSingleGraph[3]->setEnabled(false);
			m_pSingleGraph[4]->setEnabled(false);
			m_pSingleGraph[5]->setEnabled(false);
			m_pTwoGraphs->setEnabled(false);
			m_pFourGraphs->setEnabled(false);
			m_pAllGraphs->setEnabled(false);
		}
		else
		{
			if(pXDirect->m_iPlrView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pXDirectTileWidget->activeGraphIndex()]->setChecked(true);
			else if(pXDirect->m_iPlrView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
			else if(pXDirect->m_iPlrView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
			else if(pXDirect->m_iPlrView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);

			m_pSingleGraph[1]->setEnabled(true);
			m_pSingleGraph[2]->setEnabled(true);
			m_pSingleGraph[3]->setEnabled(true);
			m_pSingleGraph[4]->setEnabled(true);
			m_pSingleGraph[5]->setEnabled(false);
			m_pTwoGraphs->setEnabled(true);
			m_pFourGraphs->setEnabled(true);
			m_pAllGraphs->setEnabled(true);
		}
	}
	else if (m_iApp==XFLR5::INVERSEDESIGN)
	{
		m_pSingleGraph[0]->setChecked(true);
	}
}


void MainFrame::onResetCurGraphScales()
{
	switch(m_iApp)
	{
		case XFLR5::MIAREX:
		{
			m_pMiarexTileWidget->onResetCurGraphScales();
			break;
		}
		case XFLR5::XFOILANALYSIS:
		{
			m_pXDirectTileWidget->onResetCurGraphScales();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->m_QGraph.setAuto(true);
			pXInverse->releaseZoom();
			pXInverse->updateView();
			return;
		}
		default:
			return;
	}

}


void MainFrame::onExportCurGraph()
{
	switch(m_iApp)
	{
		case XFLR5::MIAREX:
		{
			m_pMiarexTileWidget->onExportCurGraph();
			break;
		}
		case XFLR5::XFOILANALYSIS:
		{
			m_pXDirectTileWidget->onExportCurGraph();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->m_QGraph.exportGraph();
			break;
		}
		default:
			break;
	}
}





void MainFrame::onCurGraphSettings()
{
	switch(m_iApp)
	{
		case XFLR5::MIAREX:
		{
			m_pMiarexTileWidget->onCurGraphSettings();
			break;
		}
		case XFLR5::XFOILANALYSIS:
		{
			m_pXDirectTileWidget->onCurGraphSettings();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->onQGraphSettings();
			break;
		}
		default:
			break;
	}
}



void MainFrame::loadLastProject()
{
	XFLR5::enumApp iApp = loadXFLR5File(m_RecentFiles.at(0));
	if(m_iApp==XFLR5::NOAPP) m_iApp = iApp;
	if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		onXDirect();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		onMiarex();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		onAFoil();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		QXInverse *pXInverse = (QXInverse*)m_pXInverse;
		if(pXInverse->m_bFullInverse) onXInverse();
		else                          onXInverseMixed();
	}
}






void MainFrame::onExecuteScript()
{
	XflScriptExec scriptExecutor;
	scriptExecutor.readScript();
	scriptExecutor.makeFoils();
	scriptExecutor.loadFoilPolarFiles();
	scriptExecutor.makePlanes();
	scriptExecutor.makePlaneAnalysisList();

//	scriptExecutor.runScript();

}


