/****************************************************************************

	SaveOptionsDlg
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

#include "SaveOptionsDlg.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

SaveOptionsDlg::SaveOptionsDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Save Options"));
	m_bOpps = false;
	m_bWOpps = true;
	m_bAutoSave = true;
	m_bAutoLoadLast = false;
	m_SaveInterval = 17;
	setupLayout();
}

void SaveOptionsDlg::setupLayout()
{
	QGroupBox *pLoadBox = new QGroupBox(tr("Load options"));
	{
		QVBoxLayout *pLoadLayout = new QVBoxLayout;
		{
			m_pctrlAutoLoadLast = new QCheckBox(tr("Load last project on startup"));
			pLoadLayout->addWidget(m_pctrlAutoLoadLast);
		}
		pLoadBox->setLayout(pLoadLayout);
	}
	QGroupBox *pSaveOppBox = new QGroupBox(tr("Operating point save"));
	{
		QVBoxLayout *pSaveOppLayout = new QVBoxLayout;
		{
			QLabel *label = new QLabel(tr("Save:"));
			m_pctrlOpps  = new QCheckBox(tr("Foil Operating Points"));
			m_pctrlWOpps = new QCheckBox(tr("Wing and Plane Operating Points"));
			pSaveOppLayout->addWidget(label);
			pSaveOppLayout->addWidget(m_pctrlOpps);
			pSaveOppLayout->addWidget(m_pctrlWOpps);
		}
		pSaveOppBox->setLayout(pSaveOppLayout);
	}

	QGroupBox *pSaveTimerBox = new QGroupBox(tr("Autosave setting"));
	{
		QHBoxLayout *pSaveTimerLayout = new QHBoxLayout;
		{
			m_pctrlAutoSave = new QCheckBox("Autosave");
			QLabel *pctrlIntervalLabel = new QLabel(tr("Every"));
			m_pctrlInterval = new IntEdit(m_SaveInterval);
			QLabel *pctrlMinutes = new QLabel("mn");
			pSaveTimerLayout->addWidget(m_pctrlAutoSave);
			pSaveTimerLayout->addWidget(pctrlIntervalLabel);
			pSaveTimerLayout->addWidget(m_pctrlInterval);
			pSaveTimerLayout->addWidget(pctrlMinutes);

			connect(m_pctrlAutoSave, SIGNAL(clicked(bool)), m_pctrlInterval, SLOT(setEnabled(bool)));

		}
		pSaveTimerBox->setLayout(pSaveTimerLayout);
	}


	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		QPushButton *pOKButton = new QPushButton(tr("OK"));
		pOKButton->setAutoDefault(false);
		QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
		pCancelButton->setAutoDefault(false);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(pOKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(pCancelButton);
		pCommandButtons->addStretch(1);
		connect(pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
		connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(pLoadBox);
		pMainLayout->addWidget(pSaveTimerBox);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pSaveOppBox);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtons);
	}
	setLayout(pMainLayout);
}


void SaveOptionsDlg::initDialog(bool bAutoLoadLast, bool bOpps, bool bWOpps, bool bAutoSave, int saveInterval)
{
	m_bAutoLoadLast = bAutoLoadLast;
	m_bAutoSave = bAutoSave;
	m_SaveInterval = saveInterval;
	m_bOpps  = bOpps;
	m_bWOpps = bWOpps;
	m_pctrlOpps->setChecked(m_bOpps);
	m_pctrlWOpps->setChecked(m_bWOpps);

	m_pctrlAutoSave->setChecked(m_bAutoSave);
	m_pctrlInterval->setValue(m_SaveInterval);
	m_pctrlInterval->setEnabled(m_bAutoSave);

}



void SaveOptionsDlg::onOK()
{
	m_bAutoLoadLast = m_pctrlAutoLoadLast->isChecked();
	m_bOpps = m_pctrlOpps->isChecked();
	m_bWOpps = m_pctrlWOpps->isChecked();
	m_bAutoSave = m_pctrlAutoSave->isChecked();
	m_SaveInterval = m_pctrlInterval->value();
	accept();
}











