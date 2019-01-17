/****************************************************************************

    WingWidget Class
        Copyright (C) 2015 Andre Deperrois

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

#ifndef WINGWIDGET_H
#define WINGWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QPixmap>


class Plane;
class PlaneOpp;
class Graph;

class WingWidget : public QWidget
{
	Q_OBJECT
public:
	WingWidget(QWidget *pParent = 0);
	~WingWidget();


	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);
	void wheelEvent (QWheelEvent *event);

	void setWingScale();
	void setWingGraph(Graph *pGraph);

private:
	void paintXCmRef(QPainter &painter, QPointF ORef, double scale);
	void paintXCP(QPainter &painter, QPointF ORef, double scale);
	void paintXTr(QPainter &painter, QPointF ORef, double scale);
	void paintWing(QPainter &painter, QPointF ORef, double scale);


signals:

public slots:
	void onResetWingScale();

public:
	static void *s_pMainFrame;
	static void *s_pMiarex;

private:

	bool m_bTrans;

	double m_WingScale;
	QPointF m_ptOffset;              /**< client offset position for wing display */
	QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
	Graph * m_pGraph;
	QPixmap m_PixText;
};

#endif // WINGWIDGET_H
