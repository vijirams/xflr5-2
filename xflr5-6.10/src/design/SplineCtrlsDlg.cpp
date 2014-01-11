/****************************************************************************

	SplineCtrlsDlg
	Copyright (C) 2009-2014 Andre Deperrois adeperrois@xflr5.com

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

#include "../misc/Settings.h"
#include "AFoil.h"
#include "SplineCtrlsDlg.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>


void *SplineCtrlsDlg::s_pAFoil = NULL;

SplineCtrlsDlg::SplineCtrlsDlg(QWidget *pParent): QDialog(pParent)
{
	setWindowTitle(tr("Spline Parameters"));
	m_pSF = NULL;
	SetupLayout();
}

SplineCtrlsDlg::~SplineCtrlsDlg()
{
	delete [] m_precision;
}

void SplineCtrlsDlg::InitDialog()
{
	int i;

	QString str;
	m_pctrlDegExtrados->clear();
	m_pctrlDegIntrados->clear();
	for (i=2; i<6; i++)
	{
		str = QString("%1").arg(i);
		m_pctrlDegExtrados->addItem(str);
		m_pctrlDegIntrados->addItem(str);
	}
	m_pctrlDegExtrados->setEnabled(true);
	m_pctrlDegIntrados->setEnabled(true);


	m_pctrlDegExtrados->setCurrentIndex(m_pSF->m_Extrados.m_iDegree-2);
	m_pctrlDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
	m_pctrlOutExtrados->SetValue(m_pSF->m_Extrados.m_iRes);
	m_pctrlOutIntrados->SetValue(m_pSF->m_Intrados.m_iRes);


	//upper point list
	m_pUpperListModel = new QStandardItemModel;
	m_pUpperListModel->setRowCount(10);//temporary
	m_pUpperListModel->setColumnCount(3);

	m_pUpperListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Point"));
	m_pUpperListModel->setHeaderData(1, Qt::Horizontal, QObject::tr("x"));
	m_pUpperListModel->setHeaderData(2, Qt::Horizontal, QObject::tr("y"));

	m_pctrlUpperList->setModel(m_pUpperListModel);

	QHeaderView *HorizontalHeader = m_pctrlUpperList->horizontalHeader();
	HorizontalHeader->setStretchLastSection(true);

	m_pUpperFloatDelegate = new FloatEditDelegate;
	m_pctrlUpperList->setItemDelegate(m_pUpperFloatDelegate);

	//Lower point list
	m_pLowerListModel = new QStandardItemModel;
	m_pLowerListModel->setRowCount(10);//temporary
	m_pLowerListModel->setColumnCount(3);

	m_pLowerListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Point"));
	m_pLowerListModel->setHeaderData(1, Qt::Horizontal, QObject::tr("x"));
	m_pLowerListModel->setHeaderData(2, Qt::Horizontal, QObject::tr("y"));

	m_pctrlLowerList->setModel(m_pLowerListModel);

	HorizontalHeader = m_pctrlLowerList->horizontalHeader();
	HorizontalHeader->setStretchLastSection(true);

	m_pLowerFloatDelegate = new FloatEditDelegate;
	m_pctrlLowerList->setItemDelegate(m_pLowerFloatDelegate);

	m_precision = new int[3];
	m_precision[0] = 0;
	m_precision[1] = 5;
	m_precision[2] = 5;
	m_pUpperFloatDelegate->SetPrecision(m_precision);
	m_pLowerFloatDelegate->SetPrecision(m_precision);

	connect(m_pUpperFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(OnUpdate()));
	connect(m_pLowerFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(OnUpdate()));

	m_pctrlPtWeight->setValue(m_pSF->m_Extrados.m_PtWeight);

	FillPointLists();

	SetControls();
}


void SplineCtrlsDlg::showEvent(QShowEvent *event)
{
	int w = m_pctrlUpperList->width();
	m_pctrlUpperList->setColumnWidth(0,(int)(w/3)-20);
	m_pctrlUpperList->setColumnWidth(1,(int)(w/3)-20);
	m_pctrlUpperList->setColumnWidth(2,(int)(w/3)-20);
	w = m_pctrlLowerList->width();
	m_pctrlLowerList->setColumnWidth(0,(int)(w/3)-20);
	m_pctrlLowerList->setColumnWidth(1,(int)(w/3)-20);
	m_pctrlLowerList->setColumnWidth(2,(int)(w/3)-20);
	event->accept();
}



void SplineCtrlsDlg::SetupLayout()
{
	QGroupBox *UpperSideBox = new QGroupBox(tr("Upper side"));
	{
		QVBoxLayout *UpperSideLayout = new QVBoxLayout;
		{
			QGridLayout *UpperLayout = new QGridLayout;
			{
				QLabel *labupper1 = new QLabel(tr("Spline degree"));
				QLabel *labupper2 = new QLabel(tr("Output"));
				m_pctrlDegExtrados = new QComboBox;
				m_pctrlOutExtrados = new DoubleEdit;
				m_pctrlOutExtrados->SetPrecision(0);
				UpperLayout->addWidget(labupper1, 1,1);
				UpperLayout->addWidget(labupper2, 2,1);
				UpperLayout->addWidget(m_pctrlDegExtrados, 1,2);
				UpperLayout->addWidget(m_pctrlOutExtrados, 2,2);
			}


			m_pctrlUpperList = new QTableView;
			m_pctrlUpperList->setFont(Settings::s_TableFont);
			m_pctrlUpperList->setWindowTitle(QObject::tr("Upper side points"));
			m_pctrlUpperList->setMinimumHeight(200);
			m_pctrlUpperList->setMinimumWidth(250);
			m_pctrlUpperList->setSelectionBehavior(QAbstractItemView::SelectRows);
			UpperSideLayout->addLayout(UpperLayout);
			UpperSideLayout->addStretch(1);
			UpperSideLayout->addWidget(m_pctrlUpperList);
		}
		UpperSideBox->setLayout(UpperSideLayout);
	}

	QGroupBox *LowerSideBox = new QGroupBox(tr("Lower side"));
	{
		QVBoxLayout *LowerSideLayout = new QVBoxLayout;
		{
			QGridLayout *LowerLayout = new QGridLayout;
			{
				QLabel *lablower1 = new QLabel(tr("Spline degree"));
				QLabel *lablower2 = new QLabel(tr("Output"));
				m_pctrlDegIntrados = new QComboBox;
				m_pctrlOutIntrados = new DoubleEdit;
				m_pctrlOutIntrados->SetPrecision(0);
				LowerLayout->addWidget(lablower1, 1,1);
				LowerLayout->addWidget(lablower2, 2,1);
				LowerLayout->addWidget(m_pctrlDegIntrados, 1,2);
				LowerLayout->addWidget(m_pctrlOutIntrados, 2,2);
			}


			m_pctrlLowerList = new QTableView;
			m_pctrlLowerList->setFont(Settings::s_TableFont);
			m_pctrlLowerList->setWindowTitle(QObject::tr("Lower side points"));
			m_pctrlLowerList->setMinimumHeight(200);
			m_pctrlLowerList->setMinimumWidth(250);
			m_pctrlLowerList->setSelectionBehavior(QAbstractItemView::SelectRows);
			LowerSideLayout->addLayout(LowerLayout);
			LowerSideLayout->addStretch(1);
			LowerSideLayout->addWidget(m_pctrlLowerList);
		}
		LowerSideBox->setLayout(LowerSideLayout);
	}

	QHBoxLayout *SideLayout = new QHBoxLayout;
	{
		SideLayout->addWidget(UpperSideBox);
		SideLayout->addWidget(LowerSideBox);
	}

	m_pctrlSymetric = new QCheckBox(tr("Symetric foil"));

	QHBoxLayout *WeightLayout = new QHBoxLayout;
	{
		QLabel *labWeight = new QLabel(tr("Point Weight ="));
		m_pctrlPtWeight = new QSlider(Qt::Horizontal);
		m_pctrlPtWeight->setMinimum(1);
		m_pctrlPtWeight->setMaximum(11);
		m_pctrlPtWeight->setSliderPosition(1);
		m_pctrlPtWeight->setTickInterval(1);
		m_pctrlPtWeight->setTickPosition(QSlider::TicksBelow);
//		m_pctrlPtWeight->setSizePolicy(szPolicyMinimum);
		WeightLayout->addWidget(labWeight);
		WeightLayout->addWidget(m_pctrlPtWeight);
	}

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		OKButton        = new QPushButton(tr("OK"));
		CancelButton    = new QPushButton(tr("Cancel"));
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
	}

	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		MainLayout->addLayout(SideLayout);
		MainLayout->addStretch(1);
		MainLayout->addWidget(m_pctrlSymetric);
//		MainLayout->addLayout(WeightLayout);
		MainLayout->addStretch(1);
		MainLayout->addLayout(CommandButtons);
		setLayout(MainLayout);
	}


	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlSymetric, SIGNAL(clicked()), this, SLOT(OnUpdate()));
	connect(m_pctrlDegExtrados, SIGNAL(activated(int)), this, SLOT(OnUpdate()));
	connect(m_pctrlDegIntrados, SIGNAL(activated(int)), this, SLOT(OnUpdate()));
	connect(m_pctrlOutExtrados, SIGNAL(editingFinished()), this, SLOT(OnUpdate()));
	connect(m_pctrlOutIntrados, SIGNAL(editingFinished()), this, SLOT(OnUpdate()));

	connect(m_pctrlPtWeight, SIGNAL(sliderMoved(int)), this, SLOT(OnUpdate()));
	connect(m_pctrlPtWeight, SIGNAL(sliderReleased()), this, SLOT(OnUpdate()));

}



void SplineCtrlsDlg::FillPointLists()
{
	m_pUpperListModel->setRowCount(m_pSF->m_Extrados.m_CtrlPoint.size());
	for (int i=0; i<m_pSF->m_Extrados.m_CtrlPoint.size(); i++)
	{
		QModelIndex index = m_pUpperListModel->index(i, 0, QModelIndex());
		m_pUpperListModel->setData(index, i+1);

		QModelIndex Xindex =m_pUpperListModel->index(i, 1, QModelIndex());
		m_pUpperListModel->setData(Xindex, m_pSF->m_Extrados.m_CtrlPoint[i].x);

		QModelIndex Zindex =m_pUpperListModel->index(i, 2, QModelIndex());
		m_pUpperListModel->setData(Zindex, m_pSF->m_Extrados.m_CtrlPoint[i].y);
	}

	m_pLowerListModel->setRowCount(m_pSF->m_Intrados.m_CtrlPoint.size());
	for (int i=0; i<m_pSF->m_Intrados.m_CtrlPoint.size(); i++)
	{
		QModelIndex index = m_pLowerListModel->index(i, 0, QModelIndex());
		m_pLowerListModel->setData(index, i+1);

		QModelIndex Xindex =m_pLowerListModel->index(i, 1, QModelIndex());
		m_pLowerListModel->setData(Xindex, m_pSF->m_Intrados.m_CtrlPoint[i].x);

		QModelIndex Zindex =m_pLowerListModel->index(i, 2, QModelIndex());
		m_pLowerListModel->setData(Zindex, m_pSF->m_Intrados.m_CtrlPoint[i].y);
	}
}


void SplineCtrlsDlg::ReadData()
{
	for(int i=0; i<m_pSF->m_Extrados.m_CtrlPoint.size(); i++)
	{
		QModelIndex index = m_pUpperListModel->index(i, 1, QModelIndex());
		m_pSF->m_Extrados.m_CtrlPoint[i].x = index.data().toDouble();

		index = m_pUpperListModel->index(i, 2, QModelIndex());
		m_pSF->m_Extrados.m_CtrlPoint[i].y = index.data().toDouble();
	}
	for (int i=0; i<m_pSF->m_Intrados.m_CtrlPoint.size(); i++)
	{
		QModelIndex index = m_pLowerListModel->index(i, 1, QModelIndex());
		m_pSF->m_Intrados.m_CtrlPoint[i].x = index.data().toDouble();

		index = m_pLowerListModel->index(i, 2, QModelIndex());
		m_pSF->m_Intrados.m_CtrlPoint[i].y = index.data().toDouble();
	}

	int ideg = m_pctrlDegExtrados->currentIndex()+2;
	if(ideg<m_pSF->m_Extrados.m_CtrlPoint.size())
	{
		// there are enough control points for this degree
		m_pSF->m_Extrados.m_iDegree = ideg;
	}
	else
	{
		// too few control points, adjust the degree
		QMessageBox::warning(this,tr("Warning"), tr("The spline degree must be less than the number of control points"));
		m_pSF->m_Extrados.m_iDegree = qMax(2,m_pSF->m_Extrados.m_CtrlPoint.size()-1);
		m_pctrlDegExtrados->setCurrentIndex(m_pSF->m_Extrados.m_iDegree-2);
	}

	ideg = m_pctrlDegIntrados->currentIndex()+2;
	if(ideg<m_pSF->m_Intrados.m_CtrlPoint.size())
	{
		// there are enough control points for this degree
		m_pSF->m_Intrados.m_iDegree = ideg;
	}
	else
	{
		// too few control points, adjust the degree
		QMessageBox::warning(this,tr("Warning"), tr("The spline degree must be less than the number of control points"));

		m_pSF->m_Intrados.m_iDegree = qMax(2,m_pSF->m_Intrados.m_CtrlPoint.size()-1);
		m_pctrlDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
	}

	m_pSF->m_Extrados.m_iRes = m_pctrlOutExtrados->Value();
	m_pSF->m_Intrados.m_iRes = m_pctrlOutExtrados->Value();
	m_pSF->m_bSymetric = m_pctrlSymetric->isChecked();

	if(m_pSF->m_bSymetric)
	{
		m_pSF->m_Intrados.CopySymetric(&m_pSF->m_Extrados);
	}

	double w = (double)m_pctrlPtWeight->value();
	m_pSF->m_Extrados.m_PtWeight = exp(w);
	m_pSF->m_Extrados.m_PtWeight = exp(w);
}


void SplineCtrlsDlg::SetControls()
{
	m_pctrlSymetric->setChecked(m_pSF->m_bSymetric);
	if(m_pSF->m_bSymetric)
	{
		m_pctrlDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
		m_pctrlOutIntrados->SetValue(m_pSF->m_Intrados.m_iRes);
		FillPointLists();
	}
	m_pctrlLowerList->setEnabled(!m_pSF->m_bSymetric);
	m_pctrlDegIntrados->setEnabled(!m_pSF->m_bSymetric);
	m_pctrlOutIntrados->setEnabled(!m_pSF->m_bSymetric);

}




void SplineCtrlsDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
			}
			else
			{
				OnOK();
			}
			break;
		}
		default:
			event->ignore();
			break;
	}
}

void SplineCtrlsDlg::OnOK()
{
	ReadData();
	accept();
}


void SplineCtrlsDlg::OnUpdate()
{
	ReadData();
	SetControls();

	UpdateSplines();
}


void SplineCtrlsDlg::UpdateSplines()
{
	m_pSF->m_Extrados.SplineKnots();
	m_pSF->m_Extrados.SplineCurve();
	m_pSF->m_Intrados.SplineKnots();
	m_pSF->m_Intrados.SplineCurve();
	m_pSF->UpdateSplineFoil();
	QAFoil *pAFoil = (QAFoil*)s_pAFoil;
	pAFoil->UpdateView();
}










