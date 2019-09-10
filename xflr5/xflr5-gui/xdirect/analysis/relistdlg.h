/****************************************************************************

    ReListDlg Class
    Copyright (C) 2009-2016 Andre Deperrois

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


#ifndef RELISTDLG_H
#define RELISTDLG_H

#include <QDialog>

#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>

class FloatEditDelegate;


class ReListDlg : public QDialog
{
    Q_OBJECT
    friend class BatchDlg;
    friend class BatchThreadDlg;

public:
    ReListDlg(QWidget *pParent=nullptr);
    ~ReListDlg();

    void initDialog(QVector<double> ReList, QVector<double> MachList, QVector<double> NCritList);

private slots:
    void onDelete();
    void onInsert();
    void onOK();
    void onCellChanged(QModelIndex topLeft, QModelIndex botRight);

private:
    void fillReModel();
    void setupLayout();
    void sortData();
    void sortRe();
    void keyPressEvent(QKeyEvent *event);

private:
    QPushButton *m_pctrlInsert, *m_pctrlDelete;
    QPushButton *OKButton, *CancelButton;


    QTableView *m_pctrlReTable;
    QStandardItemModel *m_pReModel;
    FloatEditDelegate *m_pFloatDelegate;

    QVector<double> m_ReList;
    QVector<double> m_MachList;
    QVector<double> m_NCritList;

    int m_iSelection;
};

#endif // RELISTDLG_H
