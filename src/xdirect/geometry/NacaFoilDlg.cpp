/****************************************************************************

	Naca Foil Dlg
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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

#include "../analysis/XFoil.h"
#include "NacaFoilDlg.h"
#include <QGridLayout>
#include <QFormLayout>

void *NacaFoilDlg::s_pXFoil;
int NacaFoilDlg::s_Digits = 0;
int NacaFoilDlg::s_Panels = 100;


NacaFoilDlg::NacaFoilDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("NACA Foils"));

	m_pParent = pParent;

	m_bGenerated = false;
	m_pBufferFoil = NULL;

	setupLayout();

	m_pctrlNumber->setText(QString("%1").arg(s_Digits,4,10,QChar('0')));
	m_pctrlPanels->setValue(s_Panels);
}


void NacaFoilDlg::setupLayout()
{
	QFormLayout *pFormLayout = new QFormLayout;
	{
		m_pctrlNumber = new QLineEdit(this);
//		m_pctrlNumber->setValidator(new QIntValidator(m_pctrlNumber));
		m_pctrlNumber->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
		m_pctrlPanels = new IntEdit(100, this);
		m_pctrlPanels->setMax(IQX);

		pFormLayout->addRow(tr("4 or 5 digits:"), m_pctrlNumber);
		pFormLayout->addRow(tr("Number of Panels:"), m_pctrlPanels);
	}

	m_pctrlMessage = new QLabel();
	m_pctrlMessage->setMinimumWidth(120);

	QHBoxLayout *pCommandButtonsLayout = new QHBoxLayout;
	{
		m_pOKButton = new QPushButton(tr("OK"));
		m_pOKButton->setAutoDefault(false);
		CancelButton = new QPushButton(tr("Cancel"));
		CancelButton->setAutoDefault(false);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(m_pOKButton);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(CancelButton);
		pCommandButtonsLayout->addStretch(1);
		connect(m_pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
		connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addLayout(pFormLayout);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(m_pctrlMessage);
		pMainLayout->addSpacing(30);
		pMainLayout->addLayout(pCommandButtonsLayout);
	}

	setLayout(pMainLayout);

	connect(m_pctrlNumber, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(m_pctrlPanels, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}


void NacaFoilDlg::onEditingFinished()
{
//	s_Digits = locale().toInt(m_pctrlNumber->text().trimmed());

    bool bOK;
	QString strange = m_pctrlNumber->text();
    int d = m_pctrlNumber->text().toInt(&bOK);
//	int d1 = m_pctrlNumber->value();
    if(bOK) s_Digits = d;

	s_Panels = m_pctrlPanels->value();

	generateFoil();

	m_pctrlNumber->setText(QString("%1").arg(s_Digits,4,10,QChar('0')));
	m_pOKButton->setFocus();
}


void NacaFoilDlg::generateFoil()
{
	int itype;

	XFoil *pXFoil = (XFoil*)s_pXFoil;
	pXFoil->lflap = false;
	pXFoil->lbflap = false;

	if(s_Digits<=25099) itype = 5;
	if(s_Digits<=9999 ) itype = 4;

	if(itype==4) pXFoil->naca4(s_Digits, (int)(s_Panels/2));
	else if(itype==5)
	{
		int three  = s_Digits/100;
		if(three!=210 && three !=220 && three !=230 && three !=240 && three !=250)
		{
			m_pctrlNumber->selectAll();
			m_pctrlMessage->setText(tr("Illegal NACA Number"));
			m_bGenerated = false;
			return;
		}
		if(!pXFoil->naca5(s_Digits, s_Panels))
		{
			m_bGenerated = false;
			m_pctrlMessage->setText(tr("Illegal NACA Number"));
			return;
		}
	}
	else
	{
		m_pctrlNumber->selectAll();
		m_pctrlMessage->setText(tr("Illegal NACA Number"));
		m_bGenerated = false;
		return;
	}
	m_pctrlMessage->setText(" ");

	for (int j=0; j< pXFoil->nb; j++)
	{
		m_pBufferFoil->xb[j] = pXFoil->xb[j+1];
		m_pBufferFoil->yb[j] = pXFoil->yb[j+1];
		m_pBufferFoil->x[j]  = pXFoil->xb[j+1];
		m_pBufferFoil->y[j]  = pXFoil->yb[j+1];
	}
	m_pBufferFoil->nb = pXFoil->nb;
	m_pBufferFoil->n = pXFoil->nb;
	m_pBufferFoil->InitFoil();

	m_pParent->update();
	m_bGenerated = true;
}



void NacaFoilDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	// Generate the foil instead
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!m_pOKButton->hasFocus() && !CancelButton->hasFocus())
			{
				generateFoil();
				if(m_bGenerated) m_pOKButton->setFocus();
				else
				{
					m_pctrlNumber->selectAll();
				}
			}
			else if (m_pOKButton->hasFocus())
			{
				onOK();
			}
			return;
		}
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
	}
}


void NacaFoilDlg::onOK()
{
	generateFoil();
	accept();
}
