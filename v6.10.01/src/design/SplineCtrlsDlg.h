/****************************************************************************

	SplineCtrlsDlg
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

#ifndef SPLINECTRLSDLG_H
#define SPLINECTRLSDLG_H

#include "../misc/DoubleEdit.h"
#include "../objects/SplineFoil.h"
#include "../misc/FloatEditDelegate.h"


#include <QDialog>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QCheckBox>

class SplineCtrlsDlg : public QDialog
{
	Q_OBJECT
	friend class QAFoil;

public:
	SplineCtrlsDlg(QWidget *pParent);
	~SplineCtrlsDlg();

	void InitDialog();

private slots:
	void OnOK();
	void OnUpdate();

private:
	void keyPressEvent(QKeyEvent *event);
	void showEvent(QShowEvent *event);

	void FillPointLists();
	void ReadData();
	void SetControls();
	void SetupLayout();
	void UpdateSplines();


	DoubleEdit	*m_pctrlOutExtrados;
	DoubleEdit	*m_pctrlOutIntrados;
	QComboBox	*m_pctrlDegExtrados;
	QComboBox	*m_pctrlDegIntrados;
	QPushButton *OKButton, *CancelButton;
	QCheckBox *m_pctrlSymetric;
	QSlider *m_pctrlPtWeight;

	QTableView *m_pctrlUpperList, *m_pctrlLowerList;
	QStandardItemModel *m_pUpperListModel,*m_pLowerListModel;
	FloatEditDelegate *m_pUpperFloatDelegate, *m_pLowerFloatDelegate;

	int *m_precision;

protected:
	SplineFoil *m_pSF;

	static void *s_pAFoil;
};

#endif // SPLINECTRLSDLG_H
