/****************************************************************************

	LegendWidget Class
		Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QWidget>
#include <objects3d/PlaneOpp.h>
#include <QGraph.h>


class LegendWidget : public QWidget
{
	Q_OBJECT
public:
	LegendWidget(QWidget *pParent = 0);
	~LegendWidget();

	void keyPressEvent(QKeyEvent *event);
	void paintEvent(QPaintEvent *event);
	QSize sizeHint();
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void setGraph(QGraph*pGraph){m_pGraph = pGraph;}

public:
	void *m_pParent;             /**< a void pointer to the instance of the GraphTileWidget object. */

	static void *s_pMainFrame;   /**< a void pointer to the instance of the MainFrame object. */
	static void *s_pMiarex;      /**< a void pointer to the instance of the QMiarex object. */
	static void *s_pXDirect;     /**< a void pointer to the instance of the QXDirect object. */

	void setMiarexView(XFLR5::enumMiarexViews eMiarexView);

private:
	void drawWPolarLegend(QPainter &painter, QPointF place, int bottom);
	void drawPOppGraphLegend(QPainter &painter, QPointF place, double bottom);
	void drawStabTimeLegend(QPainter &painter, QGraph *pGraph, QPointF place, int bottom);
	void drawCpLegend(QPainter &painter, QGraph *pGraph, QPointF place, int bottom);
	void drawPolarLegend(QPainter &painter, QPointF place, int bottom);

private:
	QGraph *m_pGraph;

	XFLR5::enumMiarexViews m_MiarexView;
	QPointF m_LegendPosition;
	QPointF m_PointDown;
	bool m_bTrans;
};

#endif // LEGENDWIDGET_H
