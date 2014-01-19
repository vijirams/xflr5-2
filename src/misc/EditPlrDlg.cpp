/****************************************************************************

	EditPlrDlg Class
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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QHeaderView>


#include "EditPlrDlg.h"
#include "./Settings.h"
#include "../xdirect/XDirect.h"
#include "../miarex/Miarex.h"


QPoint EditPlrDlg::s_Position;
QSize  EditPlrDlg::s_WindowSize;
bool EditPlrDlg::s_bWindowMaximized;



EditPlrDlg::EditPlrDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Polar Points Edition"));
	m_pXDirect    = NULL;
	m_pMiarex     = NULL;

	m_pPolar      = NULL;
	m_pWPolar     = NULL;

	m_pctrlPointTable = NULL;
	m_pPointModel     = NULL;
	m_pFloatDelegate  = NULL;

	SetupLayout();
}

EditPlrDlg::~EditPlrDlg()
{
	delete [] m_precision;
}

void EditPlrDlg::InitDialog(void *pXDirect, Polar* pPolar, void *pMiarex, WPolar*pWPolar)
{
	m_pXDirect = pXDirect;
	m_pMiarex = pMiarex;
	m_pPolar = pPolar;
	m_pWPolar = pWPolar;


	m_pPointModel = new QStandardItemModel(this);
	m_pPointModel->setRowCount(10);//temporary
	m_pPointModel->setColumnCount(14);

	m_pPointModel->setHeaderData(0, Qt::Horizontal, "Alpha");
	m_pPointModel->setHeaderData(1, Qt::Horizontal, "Cl");
	m_pPointModel->setHeaderData(2, Qt::Horizontal, "Cd");
	m_pPointModel->setHeaderData(3, Qt::Horizontal, "Cm");
	m_pPointModel->setHeaderData(4, Qt::Horizontal, "XTr_top");
	m_pPointModel->setHeaderData(5, Qt::Horizontal, "XTr_bot");
	m_pPointModel->setHeaderData(6, Qt::Horizontal, "Cl/Cd");
	m_pPointModel->setHeaderData(7, Qt::Horizontal, "Cl^3/2/Cd");
	m_pPointModel->setHeaderData(8, Qt::Horizontal, "sqrt(Cl)");
	m_pPointModel->setHeaderData(9, Qt::Horizontal, "XCp");
	m_pPointModel->setHeaderData(10, Qt::Horizontal, "HMom");
	m_pPointModel->setHeaderData(11, Qt::Horizontal, "Cdp");
	m_pPointModel->setHeaderData(12, Qt::Horizontal, "Cpmn");
	m_pPointModel->setHeaderData(13, Qt::Horizontal, "Re");

	m_pctrlPointTable->setModel(m_pPointModel);


	QHeaderView *HorizontalHeader = m_pctrlPointTable->horizontalHeader();
	HorizontalHeader->setStretchLastSection(true);

	m_pFloatDelegate = new FloatEditDelegate(this);
	m_pctrlPointTable->setItemDelegate(m_pFloatDelegate);

	m_precision = new int[14];
	m_precision[0] = 3;
	m_precision[1] = 3;
	m_precision[2] = 3;
	m_precision[3] = 3;
	m_precision[4] = 3;
	m_precision[5] = 3;
	m_precision[6] = 3;
	m_precision[7] = 3;
	m_precision[8] = 3;
	m_precision[9] = 3;
	m_precision[10] = 3;
	m_precision[11] = 3;
	m_precision[12] = 3;
	m_precision[13] = 0;

	m_pFloatDelegate->SetPrecision(m_precision);

	if(m_pXDirect && pPolar)        FillPolarData();
	else if(m_pMiarex && m_pWPolar) FillWPolarData();
}


void EditPlrDlg::FillPolarData()
{
	m_pPointModel->setColumnCount(14);
	m_pPointModel->setRowCount(m_pPolar->m_Alpha.size());
	QModelIndex index;
	for (int i=0; i<m_pPolar->m_Alpha.size(); i++)
	{
		index = m_pPointModel->index(i, 0, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Alpha.at(i));

		index = m_pPointModel->index(i, 1, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Cl.at(i));

		index = m_pPointModel->index(i, 2, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Cd.at(i));

		index = m_pPointModel->index(i, 3, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Cm.at(i));

		index = m_pPointModel->index(i, 4, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_XTr1.at(i));

		index = m_pPointModel->index(i, 5, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_XTr2.at(i));

		index = m_pPointModel->index(i, 6, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_ClCd.at(i));

		index = m_pPointModel->index(i, 7, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Cl32Cd.at(i));

		index = m_pPointModel->index(i, 8, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_RtCl.at(i));

		index = m_pPointModel->index(i, 9, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_XCp.at(i));

		index = m_pPointModel->index(i, 10, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_HMom.at(i));

		index = m_pPointModel->index(i, 11, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Cdp.at(i));

		index = m_pPointModel->index(i, 12, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Cpmn.at(i));

		index = m_pPointModel->index(i, 13, QModelIndex());
		m_pPointModel->setData(index, m_pPolar->m_Re.at(i));
	}
	m_pctrlPointTable->resizeRowsToContents();
//	m_pctrlPointTable->resizeColumnsToContents();
}



void EditPlrDlg::FillWPolarData()
{
	m_pPointModel->setColumnCount(4);
	m_pPointModel->setRowCount(m_pWPolar->m_Alpha.size());
	QModelIndex index;
	for (int i=0; i<m_pWPolar->m_Alpha.size(); i++)
	{
		index = m_pPointModel->index(i, 0, QModelIndex());
		m_pPointModel->setData(index, m_pWPolar->m_Alpha.at(i));

		index = m_pPointModel->index(i, 1, QModelIndex());
		m_pPointModel->setData(index, m_pWPolar->m_CL.at(i));

		index = m_pPointModel->index(i, 2, QModelIndex());
		m_pPointModel->setData(index, m_pWPolar->m_TCd.at(i));

		index = m_pPointModel->index(i, 3, QModelIndex());
		m_pPointModel->setData(index, m_pWPolar->m_GCm.at(i));
	}
	m_pctrlPointTable->resizeRowsToContents();
//	m_pctrlPointTable->resizeColumnsToContents();
}




void EditPlrDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
			}
			else
			{
				QDialog::accept();
			}
			break;
		}
		case Qt::Key_Escape:
		{
			QDialog::reject();
			return;
		}
		default:
			event->ignore();
	}
}


void EditPlrDlg::OnDeletePoint()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex= (QMiarex*)m_pMiarex;

	QModelIndex index = m_pctrlPointTable->currentIndex();

	if(pXDirect)
	{
		m_pPolar->Remove(index.row());
		FillPolarData();
		pXDirect->CreatePolarCurves();
		pXDirect->UpdateView();
	}
	else if(pMiarex)
	{
		m_pWPolar->Remove(index.row());
		FillWPolarData();
		pMiarex->CreateWPolarCurves();
		pMiarex->UpdateView();
	}

	if(index.row()>=m_pPointModel->rowCount()-1)
	{
		index = m_pPointModel->index(m_pPointModel->rowCount()-1,0);
	}
	if(m_pPointModel->rowCount()) m_pctrlPointTable->setCurrentIndex(index);
}



void EditPlrDlg::OnDeleteAllPoints()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex= (QMiarex*)m_pMiarex;

	if(pXDirect)
	{
		m_pPolar->ResetPolar();
		FillPolarData();
		pXDirect->CreatePolarCurves();
		pXDirect->UpdateView();
	}
	else if(pMiarex)
	{
		m_pWPolar->ClearData();
		FillWPolarData();
		pMiarex->CreateWPolarCurves();
		pMiarex->UpdateView();
	}
}


void EditPlrDlg::SetupLayout()
{
	QHBoxLayout *CommandButtonsLayout = new QHBoxLayout;
	{
		m_pctrlDeleteAllPoints = new QPushButton(tr("Delete All Points"));
		m_pctrlDeleteAllPoints->adjustSize();
		m_pctrlDeletePoint	   = new QPushButton(tr("Delete Point"));
		OKButton               = new QPushButton(tr("OK"));
		CancelButton           = new QPushButton(tr("Cancel"));
		CommandButtonsLayout->addStretch(1);
		CommandButtonsLayout->addWidget(m_pctrlDeleteAllPoints);
		CommandButtonsLayout->addWidget(m_pctrlDeletePoint);
		CommandButtonsLayout->addStretch(2);
		CommandButtonsLayout->addWidget(OKButton);
		CommandButtonsLayout->addWidget(CancelButton);
		CommandButtonsLayout->addStretch(1);
	}

	m_pctrlPointTable = new QTableView(this);
	m_pctrlPointTable->setFont(Settings::s_TableFont);
	m_pctrlPointTable->setMinimumHeight(500);
	m_pctrlPointTable->setMinimumWidth(500);
	m_pctrlPointTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pctrlPointTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pctrlPointTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout * MainLayout = new QVBoxLayout(this);
	{
		MainLayout->addWidget(m_pctrlPointTable);
		MainLayout->addLayout(CommandButtonsLayout);
	}

	setLayout(MainLayout);


	connect(m_pctrlDeletePoint, SIGNAL(clicked()),this, SLOT(OnDeletePoint()));
	connect(m_pctrlDeleteAllPoints, SIGNAL(clicked()),this, SLOT(OnDeleteAllPoints()));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}




void EditPlrDlg::showEvent(QShowEvent *event)
{
	move(s_Position);
	resize(s_WindowSize);
	if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

	event->accept();
}


void EditPlrDlg::hideEvent(QHideEvent*event)
{
	s_Position = pos();
	s_WindowSize = size();
	s_bWindowMaximized = isMaximized();
	event->accept();
}



void EditPlrDlg::resizeEvent(QResizeEvent*event)
{
	if(!m_pPointModel || !m_pctrlPointTable) return;
	int n = m_pPointModel->columnCount();
	int w = m_pctrlPointTable->width();
	int w14 = (int)((double)(w-25)/(double)n);

	for(int i=0; i<m_pPointModel->columnCount(); i++)
		m_pctrlPointTable->setColumnWidth(i,w14);

	event->accept();
}
