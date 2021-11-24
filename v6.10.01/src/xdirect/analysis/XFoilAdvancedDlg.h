/****************************************************************************

	XFoilAdvancedDlg Class
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



#ifndef XFOILADVANCEDDLG_H
#define XFOILADVANCEDDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>

#include "../../misc/DoubleEdit.h"
#include "../../misc/IntEdit.h"


class XFoilAdvancedDlg : public QDialog
{
	Q_OBJECT

	friend class QXDirect;

public:
	XFoilAdvancedDlg(QWidget *pParent=NULL);
	void InitDialog();


private slots:
	void OnOK();
	void OnDefaults();

private:
	void keyPressEvent(QKeyEvent *event);
	void SetupLayout();
	QCheckBox *m_pctrlInitBL, *m_pctrlFullReport;
	IntEdit *m_pctrlIterLimit, *m_pctrlTimerInterval;
	DoubleEdit * m_pctrlVAccel;
	QPushButton *OKButton, *CancelButton, *m_pctrlDefaults;

	QCheckBox *m_pctrlKeepErrorsOpen;

	int m_IterLimit; /** @todo replace with a static variable in XFoilTask */
	double m_VAccel;
	bool m_bInitBL;
	bool m_bFullReport;

};

#endif // XFOILADVANCEDDLG_H
