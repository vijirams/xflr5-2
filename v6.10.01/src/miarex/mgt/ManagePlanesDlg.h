/****************************************************************************

	ManageUFOsDlg Class
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


#ifndef MANAGEPLANESDLG_H
#define MANAGEPLANESSDLG_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include "PlaneTableDelegate.h"
#include "../../objects/Plane.h"



class ManagePlanesDlg : public QDialog
{
	Q_OBJECT
	friend class QMiarex;
	friend class MainFrame;

public:
	ManagePlanesDlg(QWidget *pParent);
	void InitDialog(QString &UFOName);

private slots:
	void OnDelete();
	void OnRename();
	void OnUFOClicked(QModelIndex index);
	void OnDoubleClickTable(const QModelIndex &index);
	void OnDescriptionChanged();

private:
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void FillUFOTable();
	void FillPlaneRow(int row);

	void SetupLayout();
	void SelectUFO();

private:
	QPushButton *CloseButton;
	QPushButton *m_pctrlRename, *m_pctrlDelete;
	QTextEdit *m_pctrlDescription;
	QTableView *m_pctrlUFOTable;
	QStandardItemModel *m_pUFOModel;
	PlaneTableDelegate *m_pUFODelegate;

	Plane *m_pPlane;
	bool m_bChanged;
};

#endif // MANAGEUFOSDLG_H
