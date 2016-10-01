/****************************************************************************

	QXDirectStyleDlg Class
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

#include "XDirectStyleDlg.h"
#include "XDirect.h"
#include <misc/LinePickerDlg.h>
#include "viewwidgets/oppointwidget.h"
#include <QGridLayout>
#include <QLabel>



XDirectStyleDlg::XDirectStyleDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("XDirect Styles"));
	m_pParent = pParent;

	OpPointWidget *pOpPointWidget = (OpPointWidget*)m_pParent;
	m_iNeutralStyle  = pOpPointWidget->m_iNeutralStyle;
	m_iNeutralWidth  = pOpPointWidget->m_iNeutralWidth;
	m_crNeutralColor = pOpPointWidget->m_crNeutralColor;

	m_iBLStyle  = pOpPointWidget->m_iBLStyle;
	m_iBLWidth  = pOpPointWidget->m_iBLWidth;
	m_crBLColor = pOpPointWidget->m_crBLColor;

	m_iPressureStyle  = pOpPointWidget->m_iPressureStyle;
	m_iPressureWidth  = pOpPointWidget->m_iPressureWidth;
	m_crPressureColor = pOpPointWidget->m_crPressureColor;

	setupLayout();

	m_pctrlNeutral->setStyle(m_iNeutralStyle, m_iNeutralWidth, m_crNeutralColor,0);
	m_pctrlBL->setStyle(m_iBLStyle, m_iBLWidth, m_crBLColor,0);
	m_pctrlPressure->setStyle(m_iPressureStyle, m_iPressureWidth, m_crPressureColor,0);

	onRestoreDefaults();
}


void XDirectStyleDlg::setupLayout()
{
	QGridLayout *StyleLayout = new QGridLayout;
	{
		m_pctrlNeutral  = new LineBtn(this);
		m_pctrlBL       = new LineBtn(this);
		m_pctrlPressure = new LineBtn(this);
		QLabel *lab1 = new QLabel(tr("Neutral Line"));
		QLabel *lab2 = new QLabel(tr("Boundary Layer"));
		QLabel *lab3 = new QLabel(tr("Pressure"));
		StyleLayout->addWidget(lab1,1,1);
		StyleLayout->addWidget(lab2,2,1);
		StyleLayout->addWidget(lab3,3,1);
		StyleLayout->addWidget(m_pctrlNeutral,1,2);
		StyleLayout->addWidget(m_pctrlBL,2,2);
		StyleLayout->addWidget(m_pctrlPressure,3,2);
        connect(m_pctrlNeutral,  SIGNAL(clickedLB()),this, SLOT(onNeutralStyle()));
        connect(m_pctrlBL,       SIGNAL(clickedLB()),this, SLOT(onBLStyle()));
        connect(m_pctrlPressure, SIGNAL(clickedLB()),this, SLOT(onPressureStyle()));
	}

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("OK"));
		QPushButton *DefaultsButton = new QPushButton(tr("Defaults"));
		QPushButton *CancelButton   = new QPushButton(tr("Cancel"));
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(DefaultsButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
		connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
		connect(DefaultsButton, SIGNAL(clicked()),this, SLOT(onRestoreDefaults()));
		connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}

	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		MainLayout->addLayout(StyleLayout);
		MainLayout->addStretch(1);
		MainLayout->addLayout(CommandButtons);
		MainLayout->addStretch(1);
	}

	setLayout(MainLayout);



}


void XDirectStyleDlg::onNeutralStyle()
{
    LinePickerDlg dlg(this);
	dlg.initDialog(m_iNeutralStyle, m_iNeutralWidth, m_crNeutralColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_iNeutralStyle = dlg.setStyle();
		m_iNeutralWidth = dlg.width();
		m_crNeutralColor = dlg.setColor();
		m_pctrlNeutral->setStyle(m_iNeutralStyle, m_iNeutralWidth, m_crNeutralColor,0);
	}
	OKButton->setFocus();
}


void XDirectStyleDlg::onPressureStyle()
{
    LinePickerDlg dlg(this);
	dlg.initDialog(m_iPressureStyle, m_iPressureWidth, m_crPressureColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_iPressureStyle = dlg.setStyle();
		m_iPressureWidth = dlg.width();
		m_crPressureColor = dlg.setColor();
		m_pctrlPressure->setStyle(m_iPressureStyle, m_iPressureWidth, m_crPressureColor,0);
	}
	OKButton->setFocus();
}


void XDirectStyleDlg::onBLStyle()
{
    LinePickerDlg dlg(this);
	dlg.initDialog(m_iBLStyle, m_iBLWidth, m_crBLColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_iBLStyle = dlg.setStyle();
		m_iBLWidth = dlg.width();
		m_crBLColor = dlg.setColor();
		m_pctrlBL->setStyle(m_iBLStyle, m_iBLWidth, m_crBLColor,0);
	}

	OKButton->setFocus();
}




void XDirectStyleDlg::onRestoreDefaults()
{
	m_iNeutralStyle = 2;
	m_iNeutralWidth = 1;
	m_crNeutralColor = QColor(200,200,255);
	m_pctrlNeutral->setStyle(m_iNeutralStyle, m_iNeutralWidth, m_crNeutralColor,0);

	m_crBLColor = QColor(200,70,70);
	m_iBLStyle = 1;
	m_iBLWidth = 1;
	m_pctrlBL->setStyle(m_iBLStyle, m_iBLWidth, m_crBLColor,0);

	m_crPressureColor= QColor(0,255,0);
	m_iPressureStyle = 0;
	m_iPressureWidth = 1;
	m_pctrlPressure->setStyle(m_iPressureStyle, m_iPressureWidth, m_crPressureColor,0);
}


void XDirectStyleDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus())
			{
				OKButton->setFocus();
				return;
			}
			else
			{
				accept();
				return;
			}
			event->ignore();
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
			break;
		}
	}
}
