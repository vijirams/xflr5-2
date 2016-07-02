/****************************************************************************

	AFoilGridDlg Class
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

#include "AFoil.h"
#include "GridSettingsDlg.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <misc/LinePickerDlg.h>

GridSettingsDlg::GridSettingsDlg(QWidget *pParent): QDialog(pParent)
{
	setWindowTitle(tr("Grid Options"));
	m_pParent = pParent;

	m_bScale = false;

	m_bXGrid     = false;
	m_XUnit  = 0.05;
	m_XStyle = 1;
	m_XWidth = 1;
	m_XColor = QColor(150,150,150);

	m_bYGrid     = false;
	m_YUnit  = 0.05;
	m_YStyle = 1;
	m_YWidth = 1;
	m_YColor = QColor(150,150,150);

	m_bXMinGrid  = false;
	m_XMinUnit = 0.01;
	m_XMinStyle  = 2;
	m_XMinWidth  = 1;
	m_XMinColor  = QColor(70,70,70);

	m_bYMinGrid  = false;
	m_YMinUnit = 0.01;
	m_YMinStyle  = 2;
	m_YMinWidth  = 1;
	m_YMinColor  = QColor(70,70,70);

	m_NeutralStyle = 3;
	m_NeutralWidth = 1;
	m_NeutralColor = QColor(70,70,70);

	SetupLayout();

	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlScale, SIGNAL(clicked()), this, SLOT(OnScale()));
	connect(m_pctrlNeutralShow, SIGNAL(clicked(bool)), this, SLOT(OnNeutralShow(bool)));
	connect(m_pctrlXMajShow, SIGNAL(clicked(bool)), this, SLOT(OnXMajShow(bool)));
	connect(m_pctrlYMajShow, SIGNAL(clicked(bool)), this, SLOT(OnYMajShow(bool)));
	connect(m_pctrlXMinShow, SIGNAL(clicked(bool)), this, SLOT(OnXMinShow(bool)));
	connect(m_pctrlYMinShow, SIGNAL(clicked(bool)), this, SLOT(OnYMinShow(bool)));
    connect(m_pctrlNeutralStyle, SIGNAL(clickedLB()), this, SLOT(OnNeutralStyle()));
    connect(m_pctrlXMajStyle, SIGNAL(clickedLB()), this, SLOT(OnXMajStyle()));
    connect(m_pctrlYMajStyle, SIGNAL(clickedLB()), this, SLOT(OnYMajStyle()));
    connect(m_pctrlXMinStyle, SIGNAL(clickedLB()), this, SLOT(OnXMinStyle()));
    connect(m_pctrlYMinStyle, SIGNAL(clickedLB()), this, SLOT(OnYMinStyle()));

}

void GridSettingsDlg::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Escape:
		{
			done(0);
			break;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
//				m_bApplied  = true;
			}
			else
			{
				QDialog::accept();
			}
			break;
		}
		default:
			event->ignore();
	}
}


void GridSettingsDlg::initDialog()
{
	m_pctrlNeutralStyle->setStyle(m_NeutralStyle, m_NeutralWidth, m_NeutralColor,0);

	m_pctrlXMajStyle->setStyle(m_XStyle, m_XWidth, m_XColor,0);
	m_pctrlXMinStyle->setStyle(m_XMinStyle, m_XMinWidth, m_XMinColor,0);
	m_pctrlYMajStyle->setStyle(m_YStyle, m_YWidth, m_YColor,0);
	m_pctrlYMinStyle->setStyle(m_YMinStyle, m_YMinWidth, m_YMinColor,0);

	m_pctrlNeutralStyle->setEnabled(m_bNeutralLine);
	m_pctrlXMajStyle->setEnabled(m_bXGrid);
	m_pctrlYMajStyle->setEnabled(m_bYGrid);
	m_pctrlXMinStyle->setEnabled(m_bXMinGrid);
	m_pctrlYMinStyle->setEnabled(m_bYMinGrid);
	m_pctrlXUnit->setEnabled(m_bXGrid);
	m_pctrlYUnit->setEnabled(m_bYGrid);
	m_pctrlXMinUnit->setEnabled(m_bXMinGrid);
	m_pctrlYMinUnit->setEnabled(m_bYMinGrid);

	m_pctrlScale->setChecked(m_bScale);
	m_pctrlNeutralShow->setChecked(m_bNeutralLine);
	m_pctrlXMajShow->setChecked(m_bXGrid);
	m_pctrlYMajShow->setChecked(m_bYGrid);
	m_pctrlXMinShow->setChecked(m_bXMinGrid);
	m_pctrlYMinShow->setChecked(m_bYMinGrid);

	m_pctrlXUnit->setValue(m_XUnit);
	m_pctrlYUnit->setValue(m_YUnit);
	m_pctrlXMinUnit->setValue(m_XMinUnit);
	m_pctrlYMinUnit->setValue(m_YMinUnit);
}


void GridSettingsDlg::SetupLayout()
{
	QGridLayout *pGridData = new QGridLayout;
	{
		m_pctrlNeutralShow = new QCheckBox(tr("Neutral Line"));
		m_pctrlScale       = new QCheckBox(tr("X-Scale"));
		m_pctrlXMajShow = new QCheckBox(tr("X Major Grid"));
		m_pctrlYMajShow = new QCheckBox(tr("Y Major Grid"));
		m_pctrlXMinShow = new QCheckBox(tr("X Minor Grid"));
		m_pctrlYMinShow = new QCheckBox(tr("Y Minor Grid"));

		m_pctrlNeutralStyle = new LineBtn(this);
		m_pctrlXMajStyle = new LineBtn(this);
		m_pctrlYMajStyle = new LineBtn(this);
		m_pctrlXMinStyle = new LineBtn(this);
		m_pctrlYMinStyle = new LineBtn(this);

		m_pctrlXUnit = new DoubleEdit;
		m_pctrlYUnit = new DoubleEdit;
		m_pctrlXMinUnit = new DoubleEdit;
		m_pctrlYMinUnit = new DoubleEdit;
		m_pctrlXUnit->setPrecision(3);
		m_pctrlYUnit->setPrecision(3);
		m_pctrlXMinUnit->setPrecision(3);
		m_pctrlYMinUnit->setPrecision(3);

		pGridData->addWidget(m_pctrlNeutralShow,1,1);
		pGridData->addWidget(m_pctrlXMajShow,2,1);
		pGridData->addWidget(m_pctrlYMajShow,3,1);
		pGridData->addWidget(m_pctrlXMinShow,4,1);
		pGridData->addWidget(m_pctrlYMinShow,5,1);

		pGridData->addWidget(m_pctrlNeutralStyle,1,2);
		pGridData->addWidget(m_pctrlXMajStyle,2,2);
		pGridData->addWidget(m_pctrlYMajStyle,3,2);
		pGridData->addWidget(m_pctrlXMinStyle,4,2);
		pGridData->addWidget(m_pctrlYMinStyle,5,2);

		pGridData->addWidget(m_pctrlScale,1,3);
		pGridData->addWidget(m_pctrlXUnit,2,3);
		pGridData->addWidget(m_pctrlYUnit,3,3);
		pGridData->addWidget(m_pctrlXMinUnit,4,3);
		pGridData->addWidget(m_pctrlYMinUnit,5,3);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		OKButton      = new QPushButton(tr("Accept"));
		CancelButton  = new QPushButton(tr("Cancel"));

		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(OKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(CancelButton);
		pCommandButtons->addStretch(1);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addLayout(pGridData);
		pMainLayout->addLayout(pCommandButtons);
	}
	setLayout(pMainLayout);
}


void GridSettingsDlg::OnNeutralStyle()
{
    LinePickerDlg dlg(this);
	dlg.initDialog(m_NeutralStyle,m_NeutralWidth,m_NeutralColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_NeutralStyle = dlg.setStyle();
		m_NeutralWidth = dlg.width();
		m_NeutralColor = dlg.setColor();
		m_pctrlNeutralStyle->setStyle(dlg.setStyle());
		m_pctrlNeutralStyle->setWidth(dlg.width());
		m_pctrlNeutralStyle->setColor(dlg.setColor());
	}
}


void GridSettingsDlg::OnXMajStyle()
{
    LinePickerDlg dlg(this);
	dlg.initDialog(m_XStyle,m_XWidth,m_XColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_XStyle = dlg.setStyle();
		m_XWidth = dlg.width();
		m_XColor = dlg.setColor();
		m_pctrlXMajStyle->setStyle(dlg.setStyle());
		m_pctrlXMajStyle->setWidth(dlg.width());
		m_pctrlXMajStyle->setColor(dlg.setColor());
	}
}

void GridSettingsDlg::OnXMinStyle()
{
    LinePickerDlg dlg(this);
	dlg.initDialog(m_XMinStyle,m_XMinWidth,m_XMinColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_XMinStyle = dlg.setStyle();
		m_XMinWidth = dlg.width();
		m_XMinColor = dlg.setColor();
		m_pctrlXMinStyle->setStyle(dlg.setStyle());
		m_pctrlXMinStyle->setWidth(dlg.width());
		m_pctrlXMinStyle->setColor(dlg.setColor());
	}

}

void GridSettingsDlg::OnYMajStyle()
{
    LinePickerDlg dlg(this);

	dlg.initDialog(m_YStyle,m_YWidth,m_YColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_YStyle = dlg.setStyle();
		m_YWidth = dlg.width();
		m_YColor = dlg.setColor();
		m_pctrlYMajStyle->setStyle(dlg.setStyle());
		m_pctrlYMajStyle->setWidth(dlg.width());
		m_pctrlYMajStyle->setColor(dlg.setColor());
	}
}

void GridSettingsDlg::OnYMinStyle()
{
    LinePickerDlg dlg(this);

	dlg.initDialog(m_YMinStyle,m_YMinWidth,m_YMinColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_YMinStyle = dlg.setStyle();
		m_YMinWidth = dlg.width();
		m_YMinColor = dlg.setColor();
		m_pctrlYMinStyle->setStyle(dlg.setStyle());
		m_pctrlYMinStyle->setWidth(dlg.width());
		m_pctrlYMinStyle->setColor(dlg.setColor());
	}
}

void GridSettingsDlg::OnNeutralShow(bool bShow)
{
	m_bNeutralLine = bShow;
	m_pctrlNeutralStyle->setEnabled(m_bNeutralLine);
}

void GridSettingsDlg::OnScale()
{
	m_bScale = m_pctrlScale->isChecked();
}


void GridSettingsDlg::OnXMajShow(bool bShow)
{
	m_bXGrid = bShow;
	m_pctrlXMajStyle->setEnabled(m_bXGrid);
	m_pctrlXUnit->setEnabled(m_bXGrid);
}





void GridSettingsDlg::OnYMajShow(bool bShow)
{
	m_bYGrid = bShow;
	m_pctrlYMajStyle->setEnabled(m_bYGrid);
	m_pctrlYUnit->setEnabled(m_bYGrid);
}


void GridSettingsDlg::OnXMinShow(bool bShow)
{
	m_bXMinGrid = bShow;
	m_pctrlXMinStyle->setEnabled(m_bXMinGrid);
	m_pctrlXMinUnit->setEnabled(m_bXMinGrid);
}


void GridSettingsDlg::OnYMinShow(bool bShow)
{
	m_bYMinGrid = bShow;
	m_pctrlYMinStyle->setEnabled(m_bYMinGrid);
	m_pctrlYMinUnit->setEnabled(m_bYMinGrid);
}

void GridSettingsDlg::OnOK()
{
	m_XUnit = m_pctrlXUnit->value();
	m_YUnit = m_pctrlYUnit->value();
	m_XMinUnit = m_pctrlXMinUnit->value();
	m_YMinUnit = m_pctrlYMinUnit->value();
	accept();
}





