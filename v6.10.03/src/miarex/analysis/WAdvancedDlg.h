/****************************************************************************

	WAdvancedDlg Class
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

#ifndef WADVANCEDDLG_H
#define WADVANCEDDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include "../../misc/DoubleEdit.h"
#include "../../misc/IntEdit.h"



class WAdvancedDlg : public QDialog
{
	Q_OBJECT
	friend class QMiarex;
	friend class MainFrame;

public:
	WAdvancedDlg(QWidget *pParent);
	void InitDialog();

private slots:
	void OnOK();
	void OnResetDefaults();

private:
	void keyPressEvent(QKeyEvent *event);
	void ReadParams();
	void SetParams();
	void SetupLayout();

	QLabel *m_pctrlLength, *m_pctrlLength2;
	QPushButton *OKButton, *CancelButton;
	QCheckBox *m_pctrlLogFile;
	QCheckBox *m_pctrlKeepOutOpps;
	QRadioButton *m_pctrlDirichlet, *m_pCtrlNeumann;
	DoubleEdit *m_pctrlRelax;
	DoubleEdit *m_pctrlAlphaPrec;
	DoubleEdit *m_pctrlMinPanelSize;
	IntEdit *m_pctrlNStation;
	IntEdit *m_pctrlIterMax;
	DoubleEdit *m_pctrlCoreSize;
	DoubleEdit *m_pctrlVortexPos;
	DoubleEdit *m_pctrlControlPos;

	bool m_bLogFile;
	bool m_bDirichlet;
	bool m_bTrefftz;
	bool m_bKeepOutOpps;

	int m_Iter;
	int m_NLLTStation;
	int m_WakeInterNodes;
	int m_MaxWakeIter;
	int m_InducedDragPoint;

	double m_ControlPos, m_VortexPos;
	double m_Relax, m_AlphaPrec;
	double m_CoreSize;
	double m_MinPanelSize;

};

#endif // WADVANCEDDLG_H
