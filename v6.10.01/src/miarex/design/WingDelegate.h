/****************************************************************************

	WingDelegate Class
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

#ifndef WINGDELEGATE_H
#define WINGDELEGATE_H

#include <QList>
#include <QItemDelegate>

#include "../../misc/DoubleEdit.h"
#include "../../objects/WingSection.h"

class WingDelegate : public QItemDelegate
{
	Q_OBJECT
	friend class WingDlg;
	friend class GL3dWingDlg;

public:
	WingDelegate(QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void SetPrecision(int*PrecisionTable);

private:
	void *m_pWingDlg;
	int *m_Precision; ///table of float precisions for each column
	QList<void*> *m_poaFoil;
	QList<WingSection*> *m_pWingSection;
};

#endif // QWingDelegate_H
