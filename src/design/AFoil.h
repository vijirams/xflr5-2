/****************************************************************************

	AFoil Class
	Copyright (C) 2009-2015 Andre Deperrois adeperrois@xflr5.com

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


/** @file
 *
 * This file implements the QAFoil class used as the interface for direct Foil design.
 *
*/

#ifndef QAFOIL_H
#define QAFOIL_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QCheckBox>
#include <QStandardItemModel>
#include <QRadioButton>
#include <QSettings>
#include <QList>

#include "../params.h"
#include "Direct2dDesign.h"
#include "FoilTableDelegate.h"
#include "../objects/Foil.h"
#include "../objects/SplineFoil.h"



/**
 * @brief the QAFoil class used as the interface for direct Foil design
 */
class QAFoil : public QWidget
{
	Q_OBJECT

	friend class MainFrame;
	friend class FoilTableDelegate;
	friend class TwoDWidget;
	friend class GridSettingsDlg;
	friend class AFoilTableDlg;
	friend class FlapDlg;
	friend class NacaFoilDlg;
	friend class InterpolateFoilsDlg;
	friend class SplineCtrlsDlg;
	friend class TwoDPanelDlg;
	friend class FoilGeomDlg;
	friend class TEGapDlg;
	friend class LEDlg;
	friend class LECircleDlg;
	friend class FoilCoordDlg;
	friend class CAddDlg;

public:
	QAFoil(QWidget *parent = NULL);
	~QAFoil();

	void setupLayout();
	void initDialog(void *p2DWidget, QList<void*> *poaFoil, void *pXFoil);

signals:
	void projectModified();

private slots:
	void onAFoilLECircle();
	void onExportSplinesToFile();
	void onRenameFoil();
	void onFoilStyle();
	void onDeleteCurFoil();
	void onDuplicate();
	void onExportCurFoil();
	void onFoilClicked(const QModelIndex& index);
	void onShowAllFoils();
	void onHideAllFoils();
	void onHideCurrentFoil();
	void onShowCurrentFoil();
	void onShowLegend();
	void onStoreSplines();
	void onUndo();
	void onRedo();
	void onSplineControls();
	void onNewSplines();
	void onAFoilSetFlap();
	void onAFoilDerotateFoil();
	void onAFoilNormalizeFoil();
	void onAFoilCadd();
	void onAFoilPanels();
	void onAFoilFoilCoordinates();
	void onAFoilFoilGeom();
	void onAFoilSetTEGap();
	void onAFoilSetLERadius();
	void onAFoilInterpolateFoils();
	void onAFoilNacaFoils();
	void onFoilTableCtxMenu(const QPoint &);
	void onAFoilTableColumns();
	void onResetColumnWidths();

private:
	Foil* addNewFoil(Foil *pFoil);
	void setControls();
	void clearStack(int pos=0);
	void fillFoilTable();
	void fillTableRow(int row);
	void showFoil(Foil* pFoil, bool bShow=true);
	void setAFoilParams();
	void selectFoil(Foil* pFoil = NULL);
	void paintSplines(QPainter &painter);
	void paintFoils(QPainter &painter);
	void setTableFont();

	void loadSettings(QSettings *pSettings);
	void SaveSettings(QSettings *pSettings);

	void takePicture();
	void setPicture();

	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);


private:

	Direct2dDesign *m_p2DWidget;

	QTableView *m_pctrlFoilTable;
	QStandardItemModel *m_pFoilModel;
	FoilTableDelegate *m_pFoilDelegate;

	static void *s_pMainFrame;  /**< a static pointer to the instance of the application's MainFrame object >*/

	int *m_precision;           /**< the array of digit numbers for each column of the Foil table >*/


	bool m_bLECircle;           /**< true if the leading edge circle should be displayed >*/
	bool m_bStored;             /**< true if the current Picture has been stored on the Undo stack >*/
	bool m_bShowLegend;         /**< true if the foil legend should be displayed in the 2d display area >*/

	double m_LERad;             /**< the radius of the leading edge circle to draw >*/

	QList<void *> *m_poaFoil;   /**< a pointer to the array of Foil objects >*/
	void *m_pXFoil;             /**< a void pointer to the XFoil object >*/

	SplineFoil *m_pSF;          /**< a pointer to the SplineFoil object >*/

	Foil *m_pBufferFoil;        /**< a pointer to the active buffer foil >*/


	int m_StackPos;                   /**< the current position on the Undo stack >*/
	QList<SplineFoil> m_UndoStack;    /**< the stack of incremental modifications to the SplineFoil;
										 we can't use the QStack though, because we need to access
										 any point in the case of multiple undo operations >*/

};

#endif // QAFOIL_H



