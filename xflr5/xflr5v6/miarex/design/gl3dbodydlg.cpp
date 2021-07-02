/****************************************************************************

    GL3dBodyDlg Class
    Copyright (C) 2009-2019 André Deperrois

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

#include <QFileDialog>
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>



#include <miarex/design/bodyscaledlg.h>
#include <miarex/design/bodytransdlg.h>
#include <miarex/design/gl3dbodydlg.h>
#include <miarex/design/inertiadlg.h>
#include <miarex/mgt/xmlplanereader.h>
#include <miarex/mgt/xmlplanewriter.h>
#include <xfl3d/controls/w3dprefsdlg.h>
#include <misc/lengthunitdlg.h>
#include <misc/options/settings.h>
#include <twodwidgets/bodyframewt.h>
#include <twodwidgets/bodylinewt.h>
#include <xfl3d/controls/arcball.h>
#include <xfl3d/gl3dbodyview.h>
#include <xflcore/units.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/body.h>
#include <xflobjects/objects3d/plane.h>
#include <xflwidgets/color/colorbtn.h>
#include <xflwidgets/customwts/bodytabledelegate.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/line/linepickerdlg.h>

QByteArray GL3dBodyDlg::m_VerticalSplitterSizes;
QByteArray GL3dBodyDlg::m_HorizontalSplitterSizes;
QByteArray GL3dBodyDlg::m_LeftSplitterSizes;



QByteArray GL3dBodyDlg::s_WindowGeometry;



bool GL3dBodyDlg::s_bOutline    = true;
bool GL3dBodyDlg::s_bSurfaces   = true;
bool GL3dBodyDlg::s_bVLMPanels  = false;
bool GL3dBodyDlg::s_bAxes       = true;
bool GL3dBodyDlg::s_bShowMasses = false;
bool GL3dBodyDlg::s_bFoilNames  = false;

GL3dBodyDlg::GL3dBodyDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Body Edition"));
    setWindowFlags(Qt::Window);
    setMouseTracking(true);


    m_pBody = nullptr;

    m_pPointDelegate = nullptr;
    m_pFrameDelegate = nullptr;

    m_pPointModel = nullptr;
    m_pFrameModel = nullptr;

    m_StackPos  = 0; //the current position on the stack
    m_bResetFrame = true;

    m_bChanged    = false;
    m_bEnableName = true;

    m_pScaleBody        = new QAction(tr("Scale"), this);
    m_pResetScales      = new QAction(tr("Reset Scales")+("\t(R)"), this);

    m_pUndo= new QAction(QIcon(":/resources/images/OnUndo.png"), tr("Undo"), this);
    m_pUndo->setStatusTip(tr("Cancels the last modification"));
    m_pUndo->setShortcut(QKeySequence::Undo);

    m_pRedo = new QAction(QIcon(":/resources/images/OnRedo.png"), tr("Redo"), this);
    m_pRedo->setStatusTip(tr("Restores the last cancelled modification"));
    m_pRedo->setShortcut(QKeySequence::Redo);

    m_pExportBodyGeom = new QAction(tr("Export Body Geometry to text File"), this);
    m_pExportBodyDef = new QAction(tr("Export Body Definition to txt File"), this);
    m_pExportBodyXML= new QAction(tr("Export body definition to an XML file"), this);
    m_pImportBodyDef = new QAction(tr("Import Body Definition from a text file"), this);
    m_pImportBodyXML= new QAction(tr("Import body definition from an XML file"), this);
    m_pBodyInertia = new QAction(tr("Define Inertia")+"\tF12", this);
    m_pTranslateBody = new QAction(tr("Translate"), this);

    setupLayout();
    setTableUnits();
    connectSignals();
}


void GL3dBodyDlg::connectSignals()
{
    connect(m_pUndo, SIGNAL(triggered()), this, SLOT(onUndo()));
    connect(m_pRedo, SIGNAL(triggered()), this, SLOT(onRedo()));
    connect(m_pExportBodyGeom, SIGNAL(triggered()), this, SLOT(onExportBodyGeom()));
    connect(m_pExportBodyDef, SIGNAL(triggered()), this, SLOT(onExportBodyDef()));
    connect(m_pExportBodyXML, SIGNAL(triggered()), this, SLOT(onExportBodyXML()));
    connect(m_pImportBodyDef, SIGNAL(triggered()), this, SLOT(onImportBodyDef()));
    connect(m_pImportBodyXML, SIGNAL(triggered()), this, SLOT(onImportBodyXML()));
    connect(m_pTranslateBody, SIGNAL(triggered()), this, SLOT(onTranslateBody()));
    connect(m_pBodyInertia, SIGNAL(triggered()), this, SLOT(onBodyInertia()));

    connect(m_pScaleBody,        SIGNAL(triggered()), this, SLOT(onScaleBody()));
    connect(m_pResetScales,      SIGNAL(triggered()), this, SLOT(onResetScales()));

    connect(m_ppbReset,      SIGNAL(clicked()), this, SLOT(onResetScales()));

    connect(m_pchAxes,       SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onAxes(bool)));
    connect(m_pchPanels,     SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onPanels(bool)));
    connect(m_pchSurfaces,   SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onSurfaces(bool)));
    connect(m_pchOutline,    SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onOutline(bool)));
    connect(m_pchShowMasses, SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onShowMasses(bool)));

    connect(m_ptbIso,        SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dIso()));
    connect(m_ptbX,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dFront()));
    connect(m_ptbY,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dLeft()));
    connect(m_ptbZ,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dTop()));
    connect(m_ptbFlip,       SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dFlip()));

    connect(&m_gl3dBodyview, SIGNAL(viewModified()), SLOT(onCheckViewIcons()));

    //    connect(m_pctrlEdgeWeight, SIGNAL(sliderReleased()), SLOT(onEdgeWeight()));
    connect(m_pslPanelBunch, SIGNAL(sliderMoved(int)), SLOT(onNURBSPanels()));

    connect(m_prbFlatPanels, SIGNAL(clicked()), SLOT(onLineType()));
    connect(m_prbBSplines,   SIGNAL(clicked()), SLOT(onLineType()));
    connect(m_pcbBodyColor,  SIGNAL(clicked()), SLOT(onBodyColor()));

    connect(m_pleBodyName,     SIGNAL(editingFinished()), SLOT(onBodyName()));
    connect(m_prbTextures,   SIGNAL(clicked()), SLOT(onTextures()));
    connect(m_prbColor,      SIGNAL(clicked()), SLOT(onTextures()));

    connect(m_pdeNHoopPanels,  SIGNAL(editingFinished()), SLOT(onNURBSPanels()));
    connect(m_pdeNXPanels,     SIGNAL(editingFinished()), SLOT(onNURBSPanels()));
    connect(m_pcbXDegree,      SIGNAL(activated(int)), SLOT(onSelChangeXDegree(int)));
    connect(m_pcbHoopDegree,   SIGNAL(activated(int)), SLOT(onSelChangeHoopDegree(int)));

    connect(m_ppbUndo,       SIGNAL(clicked()),SLOT(onUndo()));
    connect(m_ppbRedo,       SIGNAL(clicked()),SLOT(onRedo()));

    connect(m_pSelectionModelFrame, SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onFrameItemClicked(QModelIndex)));
    connect(m_pFrameDelegate,       SIGNAL(closeEditor(QWidget *)), SLOT(onFrameCellChanged(QWidget *)));
    connect(m_pSelectionModelPoint, SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onPointItemClicked(QModelIndex)));
    connect(m_pPointDelegate,       SIGNAL(closeEditor(QWidget *)), SLOT(onPointCellChanged(QWidget *)));

    connect(m_pBodyLineWt, SIGNAL(frameSelChanged()), SLOT(onFrameClicked()));
    connect(m_pFrameWt,    SIGNAL(pointSelChanged()), SLOT(onPointClicked()));

    connect(m_pBodyLineWt, SIGNAL(objectModified()), SLOT(onUpdateBody()));
    connect(m_pFrameWt,    SIGNAL(objectModified()), SLOT(onUpdateBody()));
}


void GL3dBodyDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)           accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void GL3dBodyDlg::setTableUnits()
{
    QString length;
    Units::getLengthUnitLabel(length);

    m_pFrameModel->setHeaderData(0, Qt::Horizontal, "x ("+length+")");
    m_pFrameModel->setHeaderData(1, Qt::Horizontal, tr("NPanels"));
    m_pPointModel->setHeaderData(0, Qt::Horizontal, "y ("+length+")");
    m_pPointModel->setHeaderData(1, Qt::Horizontal, "z ("+length+")");
    m_pPointModel->setHeaderData(2, Qt::Horizontal, tr("NPanels"));
}


GL3dBodyDlg::~GL3dBodyDlg()
{
    clearStack(-1);
    if(m_pFrameDelegate)  delete m_pFrameDelegate;

    if(m_pPointDelegate)  delete m_pPointDelegate;
    delete m_pFrameModel;
    delete m_pPointModel;
}


void GL3dBodyDlg::fillFrameCell(int iItem, int iSubItem)
{
    QModelIndex ind;

    switch (iSubItem)
    {
        case 0:
        {
            ind = m_pFrameModel->index(iItem, 0, QModelIndex());
            m_pFrameModel->setData(ind, m_pBody->frame(iItem)->m_Position.x * Units::mtoUnit());
            break;
        }
        case 1:
        {
            ind = m_pFrameModel->index(iItem, 1, QModelIndex());
            m_pFrameModel->setData(ind, m_pBody->m_xPanels[iItem]);
            break;
        }
        default:
        {
            break;
        }
    }
}


void GL3dBodyDlg::fillFrameDataTable()
{
    if(!m_pBody) return;
    m_pFrameModel->setRowCount(m_pBody->frameCount());

    for(int i=0; i<m_pBody->frameCount(); i++)
    {
        fillFrameTableRow(i);
    }
}


void GL3dBodyDlg::fillFrameTableRow(int row)
{
    QModelIndex ind;

    ind = m_pFrameModel->index(row, 0, QModelIndex());
    m_pFrameModel->setData(ind, m_pBody->frame(row)->m_Position.x * Units::mtoUnit());

    ind = m_pFrameModel->index(row, 1, QModelIndex());
    m_pFrameModel->setData(ind, m_pBody->m_xPanels[row]);
}


void GL3dBodyDlg::fillPointDataTable()
{
    if(!m_pBody) return;

    m_pPointModel->setRowCount(m_pBody->sideLineCount());
    for(int i=0; i<m_pBody->sideLineCount(); i++)
    {
        fillPointTableRow(i);
    }
}


void GL3dBodyDlg::fillPointCell(int iItem, int iSubItem)
{
    QModelIndex ind;

    if(!m_pBody) return;
    int l = m_pBody->m_iActiveFrame;

    switch (iSubItem)
    {
        case 0:
        {
            ind = m_pPointModel->index(iItem, 0, QModelIndex());
            m_pPointModel->setData(ind, m_pBody->frame(l)->m_CtrlPoint[iItem].y * Units::mtoUnit());
            break;
        }
        case 1:
        {
            ind = m_pPointModel->index(iItem, 1, QModelIndex());
            m_pPointModel->setData(ind, m_pBody->frame(l)->m_CtrlPoint[iItem].z*Units::mtoUnit());
            break;
        }
        case 2:
        {
            ind = m_pPointModel->index(iItem, 2, QModelIndex());
            m_pPointModel->setData(ind,m_pBody->m_hPanels[iItem]);
            break;
        }

        default:
        {
            break;
        }
    }
}


void GL3dBodyDlg::fillPointTableRow(int row)
{
    if(!m_pFrame) return;
    QModelIndex ind;

    ind = m_pPointModel->index(row, 0, QModelIndex());
    m_pPointModel->setData(ind, m_pFrame->m_CtrlPoint[row].y * Units::mtoUnit());

    ind = m_pPointModel->index(row, 1, QModelIndex());
    m_pPointModel->setData(ind, m_pFrame->m_CtrlPoint[row].z * Units::mtoUnit());

    ind = m_pPointModel->index(row, 2, QModelIndex());
    m_pPointModel->setData(ind, m_pBody->m_hPanels[row]);
}


void GL3dBodyDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Z:
        {
            if(bCtrl)
            {
                if(bShift)
                {
                    onRedo();
                }
                else onUndo();
                pEvent->accept();
            }
            else pEvent->ignore();
            break;
        }
        case Qt::Key_Y:
        {
            if(bCtrl)
            {
                onRedo();
                pEvent->accept();
            }
            else pEvent->ignore();
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_F12:
        {
            onBodyInertia();
            break;
        }
        default:
            pEvent->ignore();
    }
}



void GL3dBodyDlg::setViewControls()
{
    m_ptbX->setChecked(false);
    m_ptbY->setChecked(false);
    m_ptbZ->setChecked(false);
    m_ptbIso->setChecked(false);
}


void GL3dBodyDlg::onBodyName()
{
    if(m_pBody)
    {
        m_pBody->m_BodyName = m_pleBodyName->text();
        m_pBody->m_BodyDescription = m_pteBodyDescription->toPlainText();
    }
}


void GL3dBodyDlg::onTextures()
{
    if(m_pBody) m_pBody->m_bTextures = m_prbTextures->isChecked();
    m_gl3dBodyview.resetGLBody();
    setControls();
    updateView();
}


void GL3dBodyDlg::onBodyColor()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor clr = QColorDialog::getColor(m_pBody->bodyColor(),
                                        this, "Color selection", dialogOptions);

    if(clr.isValid())
    {
        m_pBody->setBodyColor(clr);
        m_gl3dBodyview.resetGLBody();

        m_pcbBodyColor->setColor(clr);
        update();
    }
}


/**
 * Unselects all the 3D-view icons.
 */
void GL3dBodyDlg::onCheckViewIcons()
{
    m_ptbIso->setChecked(false);
    m_ptbX->setChecked(false);
    m_ptbY->setChecked(false);
    m_ptbZ->setChecked(false);
}



void GL3dBodyDlg::onBodyInertia()
{
    if(!m_pBody) return;
    InertiaDlg dlg(this);
    dlg.m_pBody  = m_pBody;
    dlg.m_pPlane = nullptr;
    dlg.m_pWing  = nullptr;
    dlg.initDialog();
    dlg.move(pos().x()+25, pos().y()+25);
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    m_pBody->computeBodyAxisInertia();
    m_bChanged = true;
    updateView();
}



void GL3dBodyDlg::onExportBodyXML()
{
    if(!m_pBody)return ;// is there anything to export ?

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = m_pBody->bodyName();
    FileName = QFileDialog::getSaveFileName(this, tr("Export plane definition to xml file"),
                                            Settings::s_LastDirName +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);

    pos = FileName.indexOf(".xml", Qt::CaseInsensitive);
    if(pos<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XMLPlaneWriter planeWriter(XFile);

    planeWriter.writeXMLBody(m_pBody);

    XFile.close();
}



void GL3dBodyDlg::onExportBodyDef()
{
    if(!m_pBody) return;

    QString FileName;

    FileName = m_pBody->m_BodyName;
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Body Definition"),
                                            Settings::s_LastDirName,
                                            QObject::tr("Text Format (*.txt)"));
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream outStream(&XFile);

    m_pBody->exportBodyDefinition(outStream, Units::mtoUnit());
}


void GL3dBodyDlg::onExportBodyGeom()
{
    if(!m_pBody) return;
    QString LengthUnit, FileName;

    Units::getLengthUnitLabel(LengthUnit);

    FileName = m_pBody->m_BodyName;
    FileName.replace("/", " ");

    int type = 1;

    QString filter =".csv";

    FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Body Geometry"),
                                            Settings::s_LastDirName ,
                                            QObject::tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) type = 2;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    m_pBody->exportGeometry(out, type, Units::mtoUnit(), NXPOINTS, NHOOPPOINTS);
}


void GL3dBodyDlg::onImportBodyDef()
{
    Body memBody;
    memBody.duplicate(m_pBody);

    double mtoUnit = 1.0;

    LengthUnitDlg luDlg(this);

    luDlg.m_Question = QObject::tr("Choose the length unit to read this file :");
    luDlg.initDialog(Units::lengthUnitIndex());

    if(luDlg.exec() == QDialog::Accepted)
    {
        mtoUnit = luDlg.mtoUnit();
    }
    else return;

    QString PathName;

    PathName = QFileDialog::getOpenFileName(this, QObject::tr("Open File"),
                                            Settings::s_LastDirName,
                                            QObject::tr("All files (*.*)"));
    if(!PathName.length()) return;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = QObject::tr("Could not read the file\n")+PathName;
        QMessageBox::warning(this, QObject::tr("Warning"), strange);
        return;
    }

    QTextStream in(&XFile);
    QString errorMsg;
    if(!m_pBody->importDefinition(in, mtoUnit, errorMsg))
    {
        QMessageBox::warning(this, QObject::tr("Warning"), errorMsg);
        m_pBody->duplicate(&memBody);
        return;
    }

    XFile.close();

    setBody();

    m_gl3dBodyview.resetGLBody();

    m_bChanged = true;

    updateView();
}



void GL3dBodyDlg::onImportBodyXML()
{
    //    Body memBody;
    //    memBody.duplicate(m_pBody);

    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, tr("Open XML File"),
                                            Settings::s_LastDirName,
                                            tr("Plane XML file")+"(*.xml)");
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+PathName;
        QMessageBox::warning(this, tr("Warning"), strange);
        return;
    }

    Plane a_plane;
    XMLPlaneReader planeReader(XFile, &a_plane);
    planeReader.readXMLPlaneFile();

    XFile.close();

    if(planeReader.hasError())
    {
        QString errorMsg = planeReader.errorString() + QString("\nline %1 column %2").arg(planeReader.lineNumber()).arg(planeReader.columnNumber());
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        //        m_pBody->duplicate(&memBody);
        return;
    }

    m_pBody->duplicate(a_plane.body());
    setBody();

    m_gl3dBodyview.resetGLBody();


    m_bChanged = true;

    updateView();
}


void GL3dBodyDlg::readFrameSectionData(int sel)
{
    if(sel>=m_pFrameModel->rowCount()) return;
    double x=0;
    int k=0;

    bool bOK=false;
    QString strong;
    QStandardItem *pItem;

    pItem = m_pFrameModel->item(sel,0);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    x = strong.toDouble(&bOK);
    if(bOK) m_pBody->frame(sel)->setuPosition(x / Units::mtoUnit());

    for(int ic=0; ic<m_pBody->frame(sel)->pointCount(); ic++)
    {
        m_pBody->frame(sel)->m_CtrlPoint[ic].x  = x / Units::mtoUnit();
    }

    pItem = m_pFrameModel->item(sel,1);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    k = strong.toInt(&bOK);
    if(bOK) m_pBody->m_xPanels[sel] = k;
}


/** The user has clicked a point in the body line view */
void GL3dBodyDlg::onFrameClicked()
{
    m_pctrlFrameTable->selectRow(m_pBody->m_iActiveFrame);
}


void GL3dBodyDlg::onFrameItemClicked(const QModelIndex &index)
{
    m_pBody->m_iActiveFrame = index.row();
    setFrame(m_pBody->m_iActiveFrame);
    updateView();
}


void GL3dBodyDlg::onFrameCellChanged(QWidget *)
{
    takePicture();
    m_bChanged = true;
    //    int n = m_pBody->m_iActiveFrame;
    readFrameSectionData(m_pBody->m_iActiveFrame);
    m_gl3dBodyview.resetGLBody();


    updateView();
}


void GL3dBodyDlg::onLineType()
{
    m_bChanged = true;
    if(m_prbFlatPanels->isChecked())
    {
        m_pBody->m_LineType = xfl::BODYPANELTYPE;
        m_pdeNXPanels->setEnabled(false);
        m_pdeNHoopPanels->setEnabled(false);
        m_pcbXDegree->setEnabled(false);
        m_pcbHoopDegree->setEnabled(false);
    }
    else
    {
        m_pBody->m_LineType = xfl::BODYSPLINETYPE;
        m_pdeNXPanels->setEnabled(true);
        m_pdeNHoopPanels->setEnabled(true);
        m_pcbXDegree->setEnabled(true);
        m_pcbHoopDegree->setEnabled(true);
    }
    m_gl3dBodyview.resetGLBody();

    updateView();
}


void GL3dBodyDlg::onPointCellChanged(QWidget *)
{
    if(!m_pFrame) return;

    takePicture();
    m_bChanged = true;
    for(int ip=0; ip<m_pPointModel->rowCount(); ip++)
        readPointSectionData(ip);
    m_gl3dBodyview.resetGLBody();
    updateView();
}


/** The user has clicked a point in the frame view */
void GL3dBodyDlg::onPointClicked()
{
    if(m_pFrame)
        m_pctrlPointTable->selectRow(m_pFrame->selectedIndex());
}


void GL3dBodyDlg::onPointItemClicked(const QModelIndex &index)
{
    if(!m_pFrame) return;
    m_pFrame->setSelected(index.row());
    m_pFrame->setHighlighted(index.row());
    updateView();
}



void GL3dBodyDlg::onResetScales()
{
    m_gl3dBodyview.on3dReset();
    m_pBodyLineWt->onResetScales();
    m_pFrameWt->onResetScales();
    updateView();
}


void GL3dBodyDlg::onScaleBody()
{
    if(!m_pBody) return;

    BodyScaleDlg dlg(this);

    dlg.m_FrameID = m_pBody->m_iActiveFrame;
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        takePicture();
        m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
        m_gl3dBodyview.resetGLBody();

        fillFrameDataTable();
        fillPointDataTable();

        updateView();
    }
}


void GL3dBodyDlg::onUpdateBody()
{
    takePicture();

    m_bChanged = true;
    m_gl3dBodyview.resetGLBody();

    m_pFrame = m_pBody->activeFrame();

    fillFrameDataTable();
    fillPointDataTable();


    updateView();
}



void GL3dBodyDlg::onSelChangeXDegree(int sel)
{
    if(!m_pBody) return;
    if (sel <0) return;

    takePicture();
    m_bChanged = true;

    int deg = sel+1;
    if(deg>=m_pBody->nurbs().frameCount())
    {
        QString strange = tr("The degree must be less than the number of Frames");
        QMessageBox::warning(this, QObject::tr("Warning"), strange);
        deg=m_pBody->nurbs().frameCount();
        m_pcbXDegree->setCurrentIndex(m_pBody->nurbs().frameCount()-2);
    }

    m_pBody->m_SplineSurface.setuDegree(deg);
    m_pBody->setNURBSKnots();
    m_gl3dBodyview.resetGLBody();

    updateView();
}


void GL3dBodyDlg::onSelChangeHoopDegree(int sel)
{
    if(!m_pBody) return;
    if (sel<0) return;

    takePicture();

    m_bChanged = true;

    int deg = sel+1;
    if(deg>=m_pBody->nurbs().framePointCount())
    {
        QString strange = tr("The degree must be less than the number of side lines");
        QMessageBox::warning(this, QObject::tr("Warning"), strange);
        deg=m_pBody->nurbs().framePointCount();
        m_pcbHoopDegree->setCurrentIndex(m_pBody->nurbs().framePointCount()-2);
    }

    m_pBody->m_SplineSurface.setvDegree(deg);
    m_pBody->setNURBSKnots();
    m_gl3dBodyview.resetGLBody();

    updateView();
}

void GL3dBodyDlg::onEdgeWeight()
{
    /*    if(!m_pBody) return;

    m_bChanged = true;
    takePicture();

    double w= (double)m_pctrlEdgeWeight->value()/100.0 + 1.0;
    m_pBody->setEdgeWeight(w, w);

    m_bResetglBody   = true;
    updateView();*/
}



void GL3dBodyDlg::onNURBSPanels()
{
    if(!m_pBody) return;

    m_bChanged = true;
    takePicture();

    m_pBody->m_Bunch = m_pslPanelBunch->sliderPosition()/100.0;

    m_pBody->m_nhPanels = int(m_pdeNHoopPanels->value());
    m_pBody->m_nxPanels = int(m_pdeNXPanels->value());
    m_pBody->setPanelPos();

    m_gl3dBodyview.resetGLBody();

    updateView();
}





void GL3dBodyDlg::onTranslateBody()
{
    if(!m_pBody) return;

    BodyTransDlg dlg(this);
    dlg.m_FrameID    = m_pBody->m_iActiveFrame;
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        takePicture();
        m_pBody->translate(dlg.m_XTrans, dlg.m_YTrans, dlg.m_ZTrans, dlg.m_bFrameOnly, dlg.m_FrameID);
        fillFrameDataTable();
        fillPointDataTable();

        m_gl3dBodyview.resetGLBody();

        updateView();
    }
}


void GL3dBodyDlg::readPointSectionData(int sel)
{
    if(sel>=m_pPointModel->rowCount()) return;
    if(!m_pFrame) return;

    double d=0;
    int k=0;

    bool bOK=false;
    QString strong;
    QStandardItem *pItem;

    pItem = m_pPointModel->item(sel,0);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pFrame->m_CtrlPoint[sel].y =d / Units::mtoUnit();

    pItem = m_pPointModel->item(sel,1);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pFrame->m_CtrlPoint[sel].z =d / Units::mtoUnit();

    pItem = m_pPointModel->item(sel,2);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    k =strong.toInt(&bOK);
    if(bOK) m_pBody->m_hPanels[sel] = k;
}


void GL3dBodyDlg::accept()
{
    if(m_pBody)
    {
        m_pBody->bodyDescription() = m_pteBodyDescription->toPlainText();
        QColor clr = m_pcbBodyColor->color();
        m_pBody->setBodyColor(clr);
    }

    s_bOutline    = m_gl3dBodyview.m_bOutline;
    s_bSurfaces   = m_gl3dBodyview.m_bSurfaces;
    s_bVLMPanels  = m_gl3dBodyview.m_bVLMPanels;
    s_bAxes       = m_gl3dBodyview.m_bAxes;
    s_bShowMasses = m_gl3dBodyview.m_bShowMasses;
    s_bFoilNames  = m_gl3dBodyview.m_bFoilNames;

    done(QDialog::Accepted);
}


void GL3dBodyDlg::reject()
{
    s_bOutline    = m_gl3dBodyview.m_bOutline;
    s_bSurfaces   = m_gl3dBodyview.m_bSurfaces;
    s_bVLMPanels  = m_gl3dBodyview.m_bVLMPanels;
    s_bAxes       = m_gl3dBodyview.m_bAxes;
    s_bShowMasses = m_gl3dBodyview.m_bShowMasses;
    s_bFoilNames  = m_gl3dBodyview.m_bFoilNames;

    if(m_bChanged)
    {
        m_pBody->m_BodyName = m_pleBodyName->text();

        int res = QMessageBox::question(this, tr("Body Dlg Exit"), tr("Save the Body ?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (QMessageBox::No == res)
        {
            m_pBody = nullptr;
            QDialog::reject();
        }
        else if (QMessageBox::Cancel == res) return;
        else
        {
            m_pBody = nullptr;
            done(QDialog::Accepted);
            return;
        }
    }
    else m_pBody = nullptr;

    done(QDialog::Rejected);
}


void GL3dBodyDlg::resizeEvent(QResizeEvent *pEvent)
{
    //    SetBodyScale();
    //    SetRectangles();

    resizeTables();
    pEvent->accept();
}


bool GL3dBodyDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("GL3dBody");
    {
        s_WindowGeometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        m_HorizontalSplitterSizes = settings.value("HorizontalSplitterSizes").toByteArray();
        m_LeftSplitterSizes = settings.value("LeftSplitterSizes").toByteArray();
    }
    settings.endGroup();
    return true;
}



bool GL3dBodyDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("GL3dBody");
    {
        settings.setValue("WindowGeom", s_WindowGeometry);

        settings.setValue("HorizontalSplitterSizes", m_HorizontalSplitterSizes);
        settings.setValue("LeftSplitterSizes", m_LeftSplitterSizes);

    }
    settings.endGroup();
    return true;
}



void GL3dBodyDlg::setControls()
{
    m_pleBodyName->setEnabled(m_bEnableName);

    m_pcbBodyColor->setEnabled(m_prbColor->isChecked());

    m_pchOutline->setChecked(m_gl3dBodyview.m_bOutline);
    m_pchPanels->setChecked(m_gl3dBodyview.m_bVLMPanels);
    m_pchAxes->setChecked(m_gl3dBodyview.m_bAxes);
    m_pchShowMasses->setChecked(m_gl3dBodyview.m_bShowMasses);
    m_pchSurfaces->setChecked(m_gl3dBodyview.m_bSurfaces);


    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);

    //    m_pctrlEdgeWeight->setSliderPosition((int)((m_pBody->m_SplineSurface.m_EdgeWeightu-1.0)*100.0));

    if(m_pBody && m_pBody->m_LineType==xfl::BODYPANELTYPE)
    {
        m_pdeNXPanels->setEnabled(false);
        m_pdeNHoopPanels->setEnabled(false);
        m_pcbXDegree->setEnabled(false);
        m_pcbHoopDegree->setEnabled(false);
    }
    else if(m_pBody && m_pBody->m_LineType==xfl::BODYSPLINETYPE)
    {
        m_pdeNXPanels->setEnabled(true);
        m_pdeNHoopPanels->setEnabled(true);
        m_pcbXDegree->setEnabled(true);
        m_pcbHoopDegree->setEnabled(true);
    }

    if(m_pBody)
    {
        m_pslPanelBunch->setSliderPosition(int(m_pBody->m_Bunch*100.0));
        m_pcbBodyColor->setColor(m_pBody->m_BodyColor);

        m_pdeNXPanels->setValue(m_pBody->m_nxPanels);
        m_pdeNHoopPanels->setValue(m_pBody->m_nhPanels);

        m_pcbXDegree->setCurrentIndex(m_pBody->m_SplineSurface.uDegree()-1);
        m_pcbHoopDegree->setCurrentIndex(m_pBody->m_SplineSurface.vDegree()-1);
    }
}


bool GL3dBodyDlg::initDialog(Body *pBody)
{
    if(!pBody) return false;

    m_pctrlFrameTable->setFont(DisplayOptions::tableFont());
    m_pctrlPointTable->setFont(DisplayOptions::tableFont());

    m_gl3dBodyview.setBody(pBody);
    m_gl3dBodyview.setScale(pBody->length());

    m_pBodyLineWt->setUnitFactor(Units::mtoUnit());
    m_pFrameWt->setUnitFactor(Units::mtoUnit());

    return setBody(pBody);
}


bool GL3dBodyDlg::setBody(Body *pBody)
{
    if(pBody) m_pBody = pBody;

    m_prbColor->setChecked(!m_pBody->hasTextures());
    m_prbTextures->setChecked(m_pBody->hasTextures());

    m_prbFlatPanels->setChecked(m_pBody->m_LineType==xfl::BODYPANELTYPE);
    m_prbBSplines->setChecked(m_pBody->m_LineType==xfl::BODYSPLINETYPE);

    m_pBodyLineWt->setBody(m_pBody);
    m_pFrameWt->setBody(m_pBody);

    m_pFrame = m_pBody->activeFrame();

    setControls();
    fillFrameDataTable();
    fillPointDataTable();

    m_pleBodyName->setText(m_pBody->m_BodyName);

    takePicture();

    return true;
}


void GL3dBodyDlg::setFrame(int iFrame)
{
    if(!m_pBody) return;
    if(iFrame<0 || iFrame>=m_pBody->frameCount()) m_pFrame = nullptr;
    else                                          m_pFrame = m_pBody->frame(iFrame);
    m_pBody->m_iActiveFrame = iFrame;

    m_gl3dBodyview.resetGLBody();

    fillPointDataTable();;
}


void GL3dBodyDlg::setFrame(Frame *pFrame)
{
    if(!m_pBody || !pFrame) return;

    m_pBody->setActiveFrame(pFrame);

    m_gl3dBodyview.resetGLBody();

    fillPointDataTable();;
}



void GL3dBodyDlg::setupLayout()
{
    QString str;

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

//    m_gl3dBodyview.m_pglBodyDlg = this;
    m_gl3dBodyview.m_bOutline    = s_bOutline;
    m_gl3dBodyview.m_bSurfaces   = s_bSurfaces;
    m_gl3dBodyview.m_bVLMPanels  = s_bVLMPanels;
    m_gl3dBodyview.m_bAxes       = s_bAxes;
    m_gl3dBodyview.m_bShowMasses = s_bShowMasses;
    m_gl3dBodyview.m_bFoilNames  = s_bFoilNames;

    QVBoxLayout *pControlsLayout = new QVBoxLayout;
    {
        QGridLayout *pThreeDParamsLayout = new QGridLayout;
        {
            m_pchAxes       = new QCheckBox(tr("Axes"));
            m_pchSurfaces   = new QCheckBox(tr("Surfaces"));
            m_pchOutline    = new QCheckBox(tr("Outline"));
            m_pchPanels     = new QCheckBox(tr("Panels"));
            m_pchShowMasses = new QCheckBox(tr("Masses"));
            m_pchAxes->setSizePolicy(szPolicyMinimum);
            m_pchSurfaces->setSizePolicy(szPolicyMinimum);
            m_pchOutline->setSizePolicy(szPolicyMinimum);
            m_pchPanels->setSizePolicy(szPolicyMinimum);
            pThreeDParamsLayout->addWidget(m_pchAxes, 1,1);
            pThreeDParamsLayout->addWidget(m_pchPanels, 1,2);
            pThreeDParamsLayout->addWidget(m_pchSurfaces, 2,1);
            pThreeDParamsLayout->addWidget(m_pchOutline, 2,2);
            pThreeDParamsLayout->addWidget(m_pchShowMasses, 2,3);
        }

        QHBoxLayout *pAxisViewLayout = new QHBoxLayout;
        {
            m_ptbX          = new QToolButton;
            m_ptbY          = new QToolButton;
            m_ptbZ          = new QToolButton;
            m_ptbIso        = new QToolButton;
            m_ptbFlip       = new QToolButton;
            if(m_ptbX->iconSize().height()<=48)
            {
                m_ptbX->setIconSize(QSize(32,32));
                m_ptbY->setIconSize(QSize(32,32));
                m_ptbZ->setIconSize(QSize(32,32));
                m_ptbIso->setIconSize(QSize(32,32));
                m_ptbFlip->setIconSize(QSize(32,32));
            }
            m_pXView    = new QAction(QIcon(":/resources/images/OnXView.png"), tr("X View"), this);
            m_pYView    = new QAction(QIcon(":/resources/images/OnYView.png"), tr("Y View"), this);
            m_pZView    = new QAction(QIcon(":/resources/images/OnZView.png"), tr("Z View"), this);
            m_pIsoView  = new QAction(QIcon(":/resources/images/OnIsoView.png"), tr("Iso View"), this);
            m_pFlipView = new QAction(QIcon(":/resources/images/OnFlipView.png"), tr("Flip View"), this);
            m_pXView->setCheckable(true);
            m_pYView->setCheckable(true);
            m_pZView->setCheckable(true);
            m_pIsoView->setCheckable(true);

            m_ptbX->setDefaultAction(m_pXView);
            m_ptbY->setDefaultAction(m_pYView);
            m_ptbZ->setDefaultAction(m_pZView);
            m_ptbIso->setDefaultAction(m_pIsoView);
            m_ptbFlip->setDefaultAction(m_pFlipView);

            pAxisViewLayout->addWidget(m_ptbX);
            pAxisViewLayout->addWidget(m_ptbY);
            pAxisViewLayout->addWidget(m_ptbZ);
            pAxisViewLayout->addWidget(m_ptbIso);
            pAxisViewLayout->addWidget(m_ptbFlip);
        }

        QHBoxLayout* pThreeDViewLayout = new QHBoxLayout;
        {
            m_ppbReset      = new QPushButton(tr("Reset Scale"));
            m_ppbReset->setSizePolicy(szPolicyMinimum);

            pThreeDViewLayout->addWidget(m_ppbReset);
        }


        QHBoxLayout *pActionButtonsLayout = new QHBoxLayout;
        {
            m_ppbUndo = new QPushButton(QIcon(":/resources/images/OnUndo.png"), tr("Undo"));
            m_ppbRedo = new QPushButton(QIcon(":/resources/images/OnRedo.png"), tr("Redo"));

            m_ppbMenuButton = new QPushButton(tr("Other"));

            BodyMenu = new QMenu(tr("Actions..."),this);

            BodyMenu->addAction(m_pImportBodyDef);
            BodyMenu->addAction(m_pExportBodyDef);
            BodyMenu->addSeparator();
            BodyMenu->addAction(m_pImportBodyXML);
            BodyMenu->addAction(m_pExportBodyXML);
            BodyMenu->addSeparator();
            BodyMenu->addAction(m_pExportBodyGeom);
            BodyMenu->addSeparator();
            BodyMenu->addAction(m_pBodyInertia);
            BodyMenu->addSeparator();
            BodyMenu->addAction(m_pTranslateBody);
            BodyMenu->addAction(m_pScaleBody);
            BodyMenu->addSeparator();
            m_ppbMenuButton->setMenu(BodyMenu);

            pActionButtonsLayout->addWidget(m_ppbUndo);
            pActionButtonsLayout->addWidget(m_ppbRedo);
            pActionButtonsLayout->addWidget(m_ppbMenuButton);
        }


        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }


        pControlsLayout->addLayout(pAxisViewLayout);
        pControlsLayout->addLayout(pThreeDParamsLayout);
        pControlsLayout->addLayout(pThreeDViewLayout);
        pControlsLayout->addStretch(1);
        pControlsLayout->addLayout(pActionButtonsLayout);
        pControlsLayout->addStretch(1);
        pControlsLayout->addWidget(m_pButtonBox);
    }


    QVBoxLayout *pBodyParamsLayout = new QVBoxLayout;
    {
        m_pleBodyName = new QLineEdit(tr("BodyName"));

        QGroupBox *pStyleBox = new QGroupBox(tr("Style"));
        {
            QHBoxLayout *pStyleLayout = new QHBoxLayout;
            {
                m_prbTextures = new QRadioButton(tr("Textures"));
                m_prbColor    = new QRadioButton(tr("Color"));
                pStyleLayout->addWidget(m_prbTextures);
                pStyleLayout->addWidget(m_prbColor);

                m_pcbBodyColor = new ColorBtn(this);
                m_pcbBodyColor->setSizePolicy(szPolicyMinimum);
                pStyleLayout->addStretch();
                pStyleLayout->addWidget(m_pcbBodyColor);
            }
            pStyleBox->setLayout(pStyleLayout);
        }
        QLabel *BodyDes = new QLabel(tr("Description:"));

        m_pteBodyDescription = new QTextEdit();
        m_pteBodyDescription->setToolTip(tr("Enter here a short description for the body"));
        pBodyParamsLayout->setStretchFactor(m_pteBodyDescription,1);

        pBodyParamsLayout->addWidget(m_pleBodyName);
        pBodyParamsLayout->addWidget(pStyleBox);
        pBodyParamsLayout->addWidget(BodyDes);
        pBodyParamsLayout->addWidget(m_pteBodyDescription);
        pBodyParamsLayout->addStretch(1);
    }


    QVBoxLayout *pBodySettingsLayout = new QVBoxLayout;
    {
        QGroupBox *pBodyTypeBox = new QGroupBox(tr("Type"));
        {
            QHBoxLayout *pBodyTypeLayout = new QHBoxLayout;
            {
                m_prbFlatPanels = new QRadioButton(tr("Flat Panels"));
                m_prbBSplines   = new QRadioButton(tr("BSplines"));
                m_prbFlatPanels->setSizePolicy(szPolicyMinimum);
                m_prbBSplines->setSizePolicy(szPolicyMinimum);
                pBodyTypeLayout->addWidget(m_prbFlatPanels);
                pBodyTypeLayout->addWidget(m_prbBSplines);
            }
            pBodyTypeBox->setLayout(pBodyTypeLayout);
        }

        QGridLayout *pSplineParams = new QGridLayout;
        {
            QLabel *lab1 = new QLabel(tr("x"));
            QLabel *lab2 = new QLabel(tr("Hoop"));
            QLabel *lab3 = new QLabel(tr("Degree"));
            QLabel *lab4 = new QLabel(tr("Panels"));
            QLabel *labBunch = new QLabel(tr("Panel bunch"));

            m_pcbXDegree = new QComboBox;
            m_pcbHoopDegree = new QComboBox;
            m_pdeNXPanels = new DoubleEdit;
            m_pdeNHoopPanels = new DoubleEdit;
            /*            m_pctrlEdgeWeight = new QSlider(Qt::Horizontal);
            m_pctrlEdgeWeight->setMinimum(0);
            m_pctrlEdgeWeight->setMaximum(100);
            m_pctrlEdgeWeight->setSliderPosition(1);
            m_pctrlEdgeWeight->setTickInterval(10);
            m_pctrlEdgeWeight->setTickPosition(QSlider::TicksBelow);
            m_pctrlEdgeWeight->setSizePolicy(szPolicyMinimum);*/

            m_pslPanelBunch= new QSlider(Qt::Horizontal);
            m_pslPanelBunch->setMinimum(0    );
            m_pslPanelBunch->setMaximum(100.0);
            m_pslPanelBunch->setSliderPosition(0);
            m_pslPanelBunch->setTickInterval(10);
            m_pslPanelBunch->setTickPosition(QSlider::TicksBelow);
            m_pslPanelBunch->setSizePolicy(szPolicyMinimum);


            lab1->setSizePolicy(szPolicyMinimum);
            lab2->setSizePolicy(szPolicyMinimum);
            lab3->setSizePolicy(szPolicyMinimum);
            lab4->setSizePolicy(szPolicyMinimum);
            m_pcbXDegree->setSizePolicy(szPolicyMinimum);
            m_pcbHoopDegree->setSizePolicy(szPolicyMinimum);
            m_pdeNXPanels->setSizePolicy(szPolicyMinimum);
            m_pdeNHoopPanels->setSizePolicy(szPolicyMinimum);
            m_pdeNXPanels->setDigits(0);
            m_pdeNHoopPanels->setDigits(0);
            pSplineParams->addWidget(lab1,1,2, Qt::AlignCenter);
            pSplineParams->addWidget(lab2,1,3, Qt::AlignCenter);
            pSplineParams->addWidget(lab3,2,1, Qt::AlignRight);
            pSplineParams->addWidget(lab4,3,1, Qt::AlignRight);
            pSplineParams->addWidget(m_pcbXDegree,2,2);
            pSplineParams->addWidget(m_pcbHoopDegree,2,3);
            pSplineParams->addWidget(m_pdeNXPanels,3,2);
            pSplineParams->addWidget(m_pdeNHoopPanels,3,3);
            //            SplineParams->addWidget(labWeight,4,1);
            //            SplineParams->addWidget(m_pctrlEdgeWeight,4,2,1,2);
            pSplineParams->addWidget(labBunch,5,1);
            pSplineParams->addWidget(m_pslPanelBunch,5,2,1,2);
        }

        pBodySettingsLayout->addWidget(pBodyTypeBox);
        pBodySettingsLayout->addStretch();
        pBodySettingsLayout->addLayout(pSplineParams);
    }


    QVBoxLayout * pFramePosLayout = new QVBoxLayout;
    {
        m_pctrlFrameTable = new QTableView;
        //    m_pctrlFrameTable->setSizePolicy(szPolicyMinimum);
        m_pctrlFrameTable->setWindowTitle(tr("Frames"));
        QLabel *LabelFrame = new QLabel(tr("Frame Positions"));
        LabelFrame->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        pFramePosLayout->addWidget(LabelFrame);
        //    FramePosLayout->addStretch(1);
        m_pctrlFrameTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pctrlFrameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pctrlFrameTable->setEditTriggers(
//                                            QAbstractItemView::CurrentChanged |
                                            QAbstractItemView::DoubleClicked |
                                            QAbstractItemView::SelectedClicked |
                                            QAbstractItemView::EditKeyPressed |
                                            QAbstractItemView::AnyKeyPressed);
        pFramePosLayout->addWidget(m_pctrlFrameTable);
    }


    QVBoxLayout * pFramePointLayout = new QVBoxLayout;
    {
        m_pctrlPointTable = new QTableView;
        //    m_pctrlPointTable->setSizePolicy(szPolicyMinimum);
        m_pctrlPointTable->setWindowTitle(tr("Points"));
        QLabel *LabelPoints = new QLabel(tr("Current Frame Definition"));
        LabelPoints->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        pFramePointLayout->addWidget(LabelPoints);
        //    FramePointLayout->addStretch(1);
        m_pctrlPointTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pctrlPointTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pctrlPointTable->setEditTriggers(
//                                            QAbstractItemView::CurrentChanged |
                                            QAbstractItemView::DoubleClicked |
                                            QAbstractItemView::SelectedClicked |
                                            QAbstractItemView::EditKeyPressed |
                                            QAbstractItemView::AnyKeyPressed);

        pFramePointLayout->addWidget(m_pctrlPointTable);
    }

    m_pwtControls = new QWidget;
    {
        QHBoxLayout *pAllControls = new QHBoxLayout;
        {
            pAllControls->addLayout(pBodyParamsLayout);
            pAllControls->addStretch(1);
            pAllControls->addLayout(pBodySettingsLayout);
            pAllControls->addStretch(1);
            pAllControls->addLayout(pFramePosLayout);
            pAllControls->addStretch(1);
            pAllControls->addLayout(pFramePointLayout);
            pAllControls->addStretch(1);
            pAllControls->addLayout(pControlsLayout);
        }
        m_pwtControls->setLayout(pAllControls);
    }


    m_pHorizontalSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pLeftSplitter = new QSplitter(Qt::Vertical, this);
        {
            m_pBodyLineWt = new BodyLineWt(this);
            m_pBodyLineWt->setSizePolicy(szPolicyMaximum);
            m_pBodyLineWt->sizePolicy().setVerticalStretch(2);

            m_pLeftSplitter->addWidget(m_pBodyLineWt);
            m_pLeftSplitter->addWidget(&m_gl3dBodyview);
        }
        m_pFrameWt = new BodyFrameWt(this);
        m_pHorizontalSplitter->addWidget(m_pLeftSplitter);
        m_pHorizontalSplitter->addWidget(m_pFrameWt);
    }

    m_pVerticalSplitter = new QSplitter(Qt::Vertical, this);
    {
        m_pVerticalSplitter->addWidget(m_pHorizontalSplitter);
        m_pVerticalSplitter->addWidget(m_pwtControls);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pVerticalSplitter);
    }
    setLayout(pMainLayout);

    for (int i=1; i<6; i++)
    {
        str = QString("%1").arg(i);
        m_pcbXDegree->addItem(str);
        m_pcbHoopDegree->addItem(str);
    }

    //Setup Frame table
    m_pctrlFrameTable->horizontalHeader()->setStretchLastSection(true);

    m_pFrameModel = new QStandardItemModel;
    m_pFrameModel->setRowCount(10);//temporary
    m_pFrameModel->setColumnCount(2);

    m_pctrlFrameTable->setModel(m_pFrameModel);

    m_pSelectionModelFrame = new QItemSelectionModel(m_pFrameModel);
    m_pctrlFrameTable->setSelectionModel(m_pSelectionModelFrame);

    m_pFrameDelegate = new BodyTableDelegate(this);
    m_pctrlFrameTable->setItemDelegate(m_pFrameDelegate);
    m_pFrameDelegate->setPrecision({3,0});

    //Setup Point Table
    m_pctrlPointTable->horizontalHeader()->setStretchLastSection(true);

    m_pPointModel = new QStandardItemModel(this);
    m_pPointModel->setRowCount(10);//temporary
    m_pPointModel->setColumnCount(3);
    m_pctrlPointTable->setModel(m_pPointModel);
    m_pSelectionModelPoint = new QItemSelectionModel(m_pPointModel);
    m_pctrlPointTable->setSelectionModel(m_pSelectionModelPoint);

    m_pPointDelegate = new BodyTableDelegate;
    m_pctrlPointTable->setItemDelegate(m_pPointDelegate);
    m_pPointDelegate->setPrecision({3,3,0});
}


void GL3dBodyDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_WindowGeometry);

    if(m_VerticalSplitterSizes.length()>0)
        m_pHorizontalSplitter->restoreState(m_VerticalSplitterSizes);
    if(m_HorizontalSplitterSizes.length()>0)
        m_pHorizontalSplitter->restoreState(m_HorizontalSplitterSizes);
    if(m_LeftSplitterSizes.length()>0)
        m_pLeftSplitter->restoreState(m_LeftSplitterSizes);

    setTableUnits();
    m_bChanged    = false;
    m_gl3dBodyview.resetGLBody();

    resizeTables();

    updateView();

    pEvent->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void GL3dBodyDlg::hideEvent(QHideEvent *pEvent)
{
    s_WindowGeometry = saveGeometry();

    m_VerticalSplitterSizes  = m_pVerticalSplitter->saveState();
    m_HorizontalSplitterSizes  = m_pHorizontalSplitter->saveState();
    m_LeftSplitterSizes  = m_pLeftSplitter->saveState();
    pEvent->accept();
}


void GL3dBodyDlg::resizeTables()
{
    int ColumnWidth = int(double(m_pctrlFrameTable->width())/2.5);
    m_pctrlFrameTable->setColumnWidth(0,ColumnWidth);
    m_pctrlFrameTable->setColumnWidth(1,ColumnWidth);
    //    m_pctrlFrameTable->setColumnWidth(2,ColumnWidth);
    ColumnWidth = int(double(m_pctrlPointTable->width())/4);
    m_pctrlPointTable->setColumnWidth(0,ColumnWidth);
    m_pctrlPointTable->setColumnWidth(1,ColumnWidth);
    m_pctrlPointTable->setColumnWidth(2,ColumnWidth);
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void GL3dBodyDlg::clearStack(int pos)
{
    for(int il=m_UndoStack.size()-1; il>pos; il--)
    {
        delete m_UndoStack.at(il);
        m_UndoStack.removeAt(il);     // remove from the stack
    }
    m_StackPos = m_UndoStack.size()-1;
}


/**
 * Restores a SplineFoil definition from the current position in the stack.
 */
void GL3dBodyDlg::setPicture()
{
    Body const *pTmpBody = m_UndoStack.at(m_StackPos);
    m_pBody->duplicate(pTmpBody);
    m_pBody->setNURBSKnots();


    fillFrameDataTable();

    m_pFrame = m_pBody->activeFrame();

    fillPointDataTable();

    m_gl3dBodyview.resetGLBody();

    updateView();
}



/**
 * Copies the current Body object to a new Body and pushes it on the stack.
 */
void GL3dBodyDlg::takePicture()
{
    m_bChanged = true;

    //clear the downstream part of the stack which becomes obsolete
    clearStack(m_StackPos);

    // append a copy of the current object
    Body *pBody = new Body();
    pBody->duplicate(m_pBody);
    m_UndoStack.append(pBody);

    // the new current position is the top of the stack
    m_StackPos = m_UndoStack.size()-1;
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void GL3dBodyDlg::onUndo()
{
    if(m_StackPos>0)
    {
        m_StackPos--;
        setPicture();
        m_ppbRedo->setEnabled(true);
    }
    else
    {
        //nothing to restore
    }
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void GL3dBodyDlg::onRedo()
{
    if(m_StackPos<m_UndoStack.size()-1)
    {
        m_StackPos++;
        setPicture();
    }
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void GL3dBodyDlg::updateView()
{
    if(isVisible()) m_gl3dBodyview.update();
    m_pFrameWt->update();
    m_pBodyLineWt->update();
}


void GL3dBodyDlg::blockSignalling(bool bBlock)
{
    blockSignals(bBlock);
    m_pPointDelegate->blockSignals(bBlock);
    m_pFrameDelegate->blockSignals(bBlock);
    m_pctrlPointTable->blockSignals(bBlock);
    m_pctrlFrameTable->blockSignals(bBlock);

    m_pSelectionModelPoint->blockSignals(bBlock);
    m_pSelectionModelFrame->blockSignals(bBlock);
}

