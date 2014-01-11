/****************************************************************************

	PlaneDlg Class
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

#ifndef PLANEDLG_H
#define PLANEDLG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>

#include "../../objects/Plane.h"
#include "../../misc/DoubleEdit.h"


/** The class to define and edit planes. SUes */

class PlaneDlg : public QDialog
{
	Q_OBJECT
	friend class QMiarex;
	friend class MainFrame;

public:
    PlaneDlg(QWidget *parent);
	void InitDialog();

private slots:
	void OnOK();
	void OnFin();
	void OnStab();
	void OnBodyCheck();
	void OnDefineWing();
	void OnDefineStab();
	void OnDefineFin();
	void OnDefineBody();
	void OnChanged();
	void OnDescriptionChanged();
	void OnImportWing();
	void OnPlaneName();
	void OnSymFin();
	void OnDoubleFin();
	void OnBiplane();
	void OnDefineWing2();
	void OnImportWing2();
	void OnImportBody();
	void OnInertia();

private:
	void SetupLayout();
	void ComputePlane(void);
	void SetResults();
	void ReadParams();
	void SetParams();
	void keyPressEvent(QKeyEvent *event);
	void reject();

private:

	Plane *m_pPlane;   /**< A pointer to the plane which is currently edited in this dialog window */
	bool m_bChanged;   /**< Set to true whenever the data in the window has been changed */
	bool m_bDescriptionChanged;
	bool m_bAcceptName;
	

private:
	QLabel *m_pctrlSurf1;
	QLabel *m_pctrlSurf2;
	QLabel *m_pctrlSurf3;
	QLabel *m_pctrlVolume;
	QLabel *m_pctrlLen1;
	QLabel *m_pctrlLen2;
	QLabel *m_pctrlLen3;
	QLabel *m_pctrlLen4;
	QLabel *m_pctrlLen5;
	QLabel *m_pctrlLen6;
	QLabel *m_pctrlLen7;
	QLabel *m_pctrlLen8;
	QLabel *m_pctrlLen9;
	QLabel *m_pctrlLen10;
	QLabel *m_pctrlLen11;
	QLabel *m_pctrlLen12;
	QLabel *m_pctrlLen13;
	QLabel *m_pctrlWingSpan;
	QLabel *m_pctrlWingSurface;
	QLabel *m_pctrlStabVolume;
	QLabel *m_pctrlFinSurface;
	QLabel *m_pctrlStabLeverArm;
	QLabel *m_pctrlStabSurface;
	QLabel *m_pctrlPlaneVolume;
	QLabel *m_pctrlVLMTotalPanels;
    DoubleEdit  *m_pctrlXBody;
    DoubleEdit  *m_pctrlZBody;
    DoubleEdit  *m_pctrlXLEFin;
    DoubleEdit  *m_pctrlYLEFin;
    DoubleEdit  *m_pctrlZLEFin;
    DoubleEdit  *m_pctrlZLEStab;
    DoubleEdit  *m_pctrlXLEStab;
    DoubleEdit  *m_pctrlXLEWing;
    DoubleEdit  *m_pctrlZLEWing;
    DoubleEdit  *m_pctrlXLEWing2;
    DoubleEdit  *m_pctrlZLEWing2;
    DoubleEdit  *m_pctrlStabTilt;
    DoubleEdit  *m_pctrlFinTilt;
    DoubleEdit  *m_pctrlWingTilt;
    DoubleEdit  *m_pctrlWingTilt2;
	QLineEdit *m_pctrlPlaneName;
	QTextEdit *m_pctrlPlaneDescription;
	QCheckBox *m_pctrlBiplane;
	QCheckBox *m_pctrlBody;
	QCheckBox *m_pctrlStabCheck;
	QCheckBox *m_pctrlFinCheck;
	QCheckBox *m_pctrlDoubleFin;
	QCheckBox	*m_pctrlSymFin;
	QPushButton *m_pctrlDefineWing;
	QPushButton *m_pctrlImportWing;
	QPushButton *m_pctrlDefineWing2;
	QPushButton *m_pctrlImportWing2;
	QPushButton	*m_pctrlDefineFin;
	QPushButton	*m_pctrlVTail;
	QPushButton	*m_pctrlDefineStab;
	QPushButton *m_pctrlDefineBody;
	QPushButton *m_pctrlImportBody;
	QPushButton *m_pctrlPlaneInertia;
	QPushButton *OKButton;
	QPushButton *CancelButton;

};

#endif // PLANEDLG_H
