/****************************************************************************

	ObjectPropsDlg Class
	Copyright (C) 2010 Andre Deperrois adeperrois@xflr5.com

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

#include "ObjectPropsDlg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStringList>

#include "../globals.h"



ObjectPropsDlg::ObjectPropsDlg(QWidget *pParent) : QDialog(pParent)
{
	m_pMiarex     = NULL;
	m_pXDirect    = NULL;
	m_pPolar      = NULL;
	m_pWPolar     = NULL;
	m_pPOpp       = NULL;
	m_pOpp        = NULL;

	SetupLayout();
}


void ObjectPropsDlg::SetupLayout()
{
	setMinimumHeight(400);
	setMinimumWidth(700);


	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		QPushButton *OKButton = new QPushButton(tr("OK"));
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	}

	QVBoxLayout * MainLayout = new QVBoxLayout(this);
	{
		m_pctrlDescription = new QTextEdit;
		m_pctrlDescription->setFontFamily("Courier");
		m_pctrlDescription->setReadOnly(true);
		m_pctrlDescription->setLineWrapMode(QTextEdit::NoWrap);
		m_pctrlDescription->setWordWrapMode(QTextOption::NoWrap);
		MainLayout->addWidget(m_pctrlDescription);
		MainLayout->addSpacing(20);
		MainLayout->addLayout(CommandButtons);
	}

	setLayout(MainLayout);
}


void ObjectPropsDlg::InitDialog()
{
	QString strange;

	if(m_pXDirect)
	{
		if(m_pPolar)
		{
			m_pPolar->GetPolarProperties(strange, true);
			setWindowTitle(tr("Polar Properties"));

		}
		else if(m_pOpp)
		{
			m_pOpp->GetOppProperties(strange, true);
			setWindowTitle(tr("Operating Point Properties"));
		}
	}
	else if(m_pMiarex)
	{
		if(m_pWPolar)
		{
			m_pWPolar->GetPolarProperties(strange, true);
			setWindowTitle(tr("Polar Properties"));
		}
		else if(m_pPOpp)
		{
			m_pPOpp->GetPlaneOppProperties(strange);
			setWindowTitle(tr("Operating Point Properties"));
		}
	}

	m_pctrlDescription->setText(strange);
}








