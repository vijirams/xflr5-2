/****************************************************************************

	GraphWidget Class
		Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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


#include <mainframe.h>
#include <Settings.h>
#include "graphwidget.h"
#include "graphtilewidget.h"
#include "GraphDlg.h"
#include <WPolar.h>
#include <QPen>
#include <QPainterPath>
#include <QPainter>
#include <QPaintEvent>
#include <QStatusBar>
#include <QtDebug>



void *GraphWidget::s_pMainFrame = NULL;

GraphWidget::GraphWidget(QWidget *pParent) : QWidget(pParent)
{
	setMouseTracking(true);

	m_pParent = pParent;

	m_TitlePosition.setX(0);
	m_TitlePosition.setY(0);
	m_bDrawLegend = false;
	m_GraphTitle = "";

	m_bResetCurves = true;
	m_bTransGraph = false;
	m_bXPressed = m_bYPressed = false;

	m_pGraph = NULL;
}

void GraphWidget::setGraph(QGraph *pGraph)
{
	m_pGraph = pGraph;
	if(m_pGraph) m_pGraph->setDrawRect(rect());
}

void GraphWidget::setTitles(QString &Title, QPoint &Place)
{
	m_GraphTitle = Title;
	m_TitlePosition = Place;
}



void GraphWidget::paintEvent(QPaintEvent *  event )
{
	QPainter painter(this);
	painter.save();

	QBrush BackBrush(Settings::backgroundColor());
	painter.fillRect(event->rect(), BackBrush);
	if(!m_pGraph)
	{
		painter.restore();
		return;
	}

	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBackground(BackBrush);

	QPen mypen;
	mypen.setColor(QColor(255,0,0));
	mypen.setWidth(1);
	mypen.setStyle(Qt::SolidLine);


	painter.setPen(mypen);
	painter.drawLine(39,78,200,302);
	m_pGraph->drawGraph(painter);

/*	QPen BorderPen;
	BorderPen.setColor(m_pGraph->borderColor());
	BorderPen.setWidth(7);
	painter.setPen(BorderPen);
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
*/
	painter.drawText(m_TitlePosition, m_GraphTitle);

	if(m_bDrawLegend) m_pGraph->drawLegend(painter, m_LegendOrigin, Settings::textFont(), Settings::textColor());

	painter.restore();
}



void GraphWidget::resizeEvent ( QResizeEvent * event )
{
	QRect r = rect();
	if(m_pGraph) m_pGraph->setDrawRect(r);

	m_LegendOrigin = QPoint(r.center().x(), r.top()+20);

	if(m_pGraph)
	{
		m_pGraph->initializeGraph();
		emit graphResized(m_pGraph);
	}
	event->accept();
}



void GraphWidget::showLegend(bool bShow)
{
	m_bDrawLegend = bShow;
}



void GraphWidget::setLegendPosition(QPoint pos)
{
	m_LegendOrigin = pos;
}



void GraphWidget::contextMenuEvent (QContextMenuEvent *event)
{
	event->ignore();
}


void GraphWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_R:
		{
			onResetGraphScales();
			event->accept();
			break;
		}
		case Qt::Key_V:
		{
			GraphDlg::setActivePage(0);
			onGraphSettings();
			event->accept();
			break;
		}
		case Qt::Key_G:
		{
			onGraphSettings();
			event->accept();
			break;
		}

		default:event->ignore();
	}
}




void GraphWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::CrossCursor);
	onGraphSettings();
}


void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
	bool bCtrl;
	QPoint point;
	double xu, yu, x1, y1, xmin, xmax, ymin, ymax;

	if(!m_pGraph) return;

//	emit isActive(m_pGraph);
//	qDebug()<<"emitting"<<m_pGraph;

	setFocus();

	point = event->pos();

	bCtrl = false;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

	if(!rect().contains(event->pos()))
	{
		m_bTransGraph = false;
		return;
	}

	if ((event->buttons() & Qt::LeftButton) && m_bTransGraph)
	{
		// we translate the curves inside the graph
		m_pGraph->setAuto(false);
		x1 =  m_pGraph->clientTox(m_LastPoint.x()) ;
		y1 =  m_pGraph->clientToy(m_LastPoint.y()) ;

		xu = m_pGraph->clientTox(point.x());
		yu = m_pGraph->clientToy(point.y());

		xmin = m_pGraph->xMin() - xu+x1;
		xmax = m_pGraph->xMax() - xu+x1;
		ymin = m_pGraph->yMin() - yu+y1;
		ymax = m_pGraph->yMax() - yu+y1;

		m_pGraph->setWindow(xmin, xmax, ymin, ymax);
		update();
	}

	else if ((event->buttons() & Qt::MidButton) && !bCtrl)
	//scaling
	{
		//zoom graph
		m_pGraph->setAuto(false);
		if(point.y()-m_LastPoint.y()<0) m_pGraph->scaleAxes(1.02);
		else                            m_pGraph->scaleAxes(1.0/1.02);

		update();
	}
	// we zoom the graph or the foil
	else if ((event->buttons() & Qt::MidButton) || event->modifiers().testFlag(Qt::AltModifier))
	{
		if(m_pGraph)
		{
			//zoom graph
			m_pGraph->setAuto(false);
			if(point.y()-m_LastPoint.y()<0) m_pGraph->scaleAxes(1.02);
			else                            m_pGraph->scaleAxes(1.0/1.02);
			update();
		}

		m_LastPoint = point;
	}
	else if(m_pGraph->isInDrawRect(point))
	{
		MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
		pMainFrame->statusBar()->showMessage(QString("X =%1, Y = %2").arg(m_pGraph->clientTox(event->x())).arg(m_pGraph->clientToy(event->y())));
	}

	m_LastPoint = point;
}


void GraphWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		QPoint point = event->pos();

		m_LastPoint.rx() = point.x();
		m_LastPoint.ry() = point.y();

		m_bTransGraph = true;
		setCursor(Qt::ClosedHandCursor);

		m_LastPoint = point;
	}
}


void GraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
	setCursor(Qt::CrossCursor);
	m_bTransGraph = false;

	event->accept();
}


void GraphWidget::wheelEvent (QWheelEvent *event)
{
	double zoomFactor=1.0;

	QPoint pt(event->x(), event->y()); //client coordinates

	if(event->delta()>0)
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
		else                          zoomFactor = 1.06;
	}
	else
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
		else                          zoomFactor = 1./1.06;
	}

	if(rect().contains(pt))
	{
		if (m_bXPressed)
		{
			//zoom x scale
			m_pGraph->setAutoX(false);
			m_pGraph->scaleXAxis(1./zoomFactor);
		}
		else if(m_bYPressed)
		{
			//zoom y scale
			m_pGraph->setAutoY(false);
			m_pGraph->scaleYAxis(1./zoomFactor);
		}
		else
		{
			//zoom both
			m_pGraph->setAuto(false);
			m_pGraph->scaleAxes(1./zoomFactor);
		}

		m_pGraph->setAutoXUnit();
		m_pGraph->setAutoYUnit();
		update();
	}
}

/**
 * The user has requested the reset of the active graph's scales to their default value
 */
void GraphWidget::onResetGraphScales()
{
	m_pGraph->setAuto(true);
	update();
}



/**
 * The user has requested an edition of the settings of the active graph
 */
void GraphWidget::onGraphSettings()
{
	GraphDlg grDlg(this);
	grDlg.setGraph(m_pGraph);

//	QAction *action = qobject_cast<QAction *>(sender());
//	qDebug()<< action->data().toString();
//	grDlg.setActivePage(0);

	if(grDlg.exec() == QDialog::Accepted)
	{
		switch(m_pGraph->graphType())
		{
			case  QGRAPH::INVERSEGRAPH:
			{
				break;
			}
			case QGRAPH::OPPGRAPH:
			{
				if(m_pGraph->yVariable() == 0 || m_pGraph->yVariable()>=2)
				{
					m_pGraph->setYTitle(tr("Cp"));
					m_pGraph->setInverted(true);
				}
				else
				{
					m_pGraph->setYTitle(tr("Q"));
					m_pGraph->setInverted(false);
				}
				m_pGraph->resetYLimits();
				break;
			}
			case QGRAPH::POLARGRAPH:
			{
				QString Title;
				Title = Polar::variableName(m_pGraph->xVariable());
				m_pGraph->setXTitle(Title);

				Title = Polar::variableName(m_pGraph->yVariable());
				m_pGraph->setYTitle(Title);

				if(grDlg.bVariableChanged())
				{
					m_pGraph->setAuto(true);
					m_pGraph->setAutoYMinUnit(true);
				}
				break;
			}
			case QGRAPH::POPPGRAPH:
			{
				if(grDlg.bVariableChanged())
				{
					m_pGraph->setAutoY(true);
					m_pGraph->setAutoYMinUnit(true);
				}
				break;

			}
			case QGRAPH::WPOLARGRAPH:
			{
				QString Title;

				Title  = WPolar::variableName(m_pGraph->xVariable());
				m_pGraph->setXTitle(Title);

				Title  = WPolar::variableName(m_pGraph->yVariable());
				m_pGraph->setYTitle(Title);

				if(grDlg.bVariableChanged())
				{
					m_pGraph->setAuto(true);
					m_pGraph->setAutoYMinUnit(true);
				}
				break;
			}
			case QGRAPH::CPGRAPH:
			{
				break;
			}
			case QGRAPH::STABTIMEGRAPH:
			{
				break;
			}
			case QGRAPH::OTHERGRAPH:
				return;
		}
		emit graphChanged(m_pGraph);
	}

	m_bResetCurves = true;
	update();
}






