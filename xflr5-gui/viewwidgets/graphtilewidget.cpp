/****************************************************************************

	GraphTileWidget Class
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

#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QSplitter>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QContextMenuEvent>

#include "graphtilewidget.h"
#include <graph/graphdlg.h>
#include <globals/mainframe.h>
#include "miarex/Miarex.h"
#include "xdirect/XDirect.h"
#include "xinverse/XInverse.h"
#include <misc/options/displayoptions.h>
#include <misc/options/Units.h>


MainFrame* GraphTileWidget::s_pMainFrame = NULL;
Miarex* GraphTileWidget::s_pMiarex = NULL;
XDirect* GraphTileWidget::s_pXDirect = NULL;



GraphTileWidget::GraphTileWidget(QWidget *parent) : QWidget(parent)
{
//	setMouseTracking(true);
	m_pLegendWidget = NULL;
	m_pMainSplitter = NULL;

	m_nGraphWidgets = 0;
	m_iActiveGraphWidget = -1;

	m_xflr5App = XFLR5::NOAPP;
	m_MiarexView = XFLR5::WPOLARVIEW;

	m_iPOppIndex = m_iWPolarIndex = 0;
	m_iStabPolarIndex = m_iStabTimeIndex = 0;

	m_SingleGraphOrientation = Qt::Horizontal;
}


GraphTileWidget::~GraphTileWidget()
{

}


Graph *GraphTileWidget::graph(int iGraph)
{
	if(iGraph<0 || iGraph>=m_GraphWidget.count()) return NULL;

	return m_GraphWidget.at(iGraph)->graph();
}


GraphWidget *GraphTileWidget::graphWidget(int iGraph)
{
	if(iGraph<0 || iGraph>=m_GraphWidget.count()) return NULL;

	return m_GraphWidget.at(iGraph);
}



GraphWidget *GraphTileWidget::graphWidget(Graph *pGraph)
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		if(m_GraphWidget.at(igw)->graph()==pGraph) return m_GraphWidget[igw];
	}
	return NULL;
}


void GraphTileWidget::setGraphList(QList<Graph *> pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation)
{
	m_xflr5App = s_pMainFrame->xflr5App();
	m_nGraphWidgets = qMin(nGraphs,MAXGRAPHS);
	m_iActiveGraphWidget = iGraphWidget;

	for(int ig=0; ig<qMin(MAXGRAPHS, pGraphList.count()); ig++)
		m_GraphWidget.at(ig)->setGraph(pGraphList.at(ig));

	m_pLegendWidget->setGraph(pGraphList.at(0));
	m_SingleGraphOrientation = orientation;

	adjustLayout();

	update();
}




void GraphTileWidget::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::CrossCursor);
}


void GraphTileWidget::contextMenuEvent (QContextMenuEvent *event)
{
	QMenu * pGraphMenu = s_pMainFrame->m_pWPlrCtxMenu;

	//get the main menu
	if(m_xflr5App==XFLR5::MIAREX)
	{
		switch(m_MiarexView)
		{
			case XFLR5::WPOLARVIEW:
				pGraphMenu = s_pMainFrame->m_pWPlrCtxMenu;
				break;
			case XFLR5::WOPPVIEW:
				pGraphMenu = s_pMainFrame->m_pWOppCtxMenu;
				break;
			case XFLR5::WCPVIEW:
				pGraphMenu = s_pMainFrame->m_pWCpCtxMenu;
				break;
			case XFLR5::STABTIMEVIEW:
				pGraphMenu = s_pMainFrame->m_pWTimeCtxMenu;
				break;
			case XFLR5::STABPOLARVIEW:
				pGraphMenu = s_pMainFrame->m_pWPlrCtxMenu;
				break;
			default:
				break;
		}
	}
	else
	{
		if(s_pXDirect->bPolarView())
			pGraphMenu = s_pMainFrame->m_pOperPolarCtxMenu;
		else
			pGraphMenu = s_pMainFrame->m_pOperFoilCtxMenu;
	}

	//execute the menu
	pGraphMenu->exec(event->globalPos());
}


void GraphTileWidget::keyPressEvent(QKeyEvent *event)
{
//	bool bShift = false;
	bool bCtrl  = false;
//	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;
	switch (event->key())
	{
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		{
			if(bCtrl)
			{
				event->ignore();
				return;
			}
			else
			{
				int iGraph = event->text().toInt()-1;

				if(m_xflr5App==XFLR5::XFOILANALYSIS)
				{
					s_pXDirect->setView(XFLR5::ONEGRAPH);
				}
				else if(m_xflr5App==XFLR5::MIAREX)
				{
					if (s_pMiarex->m_iView==XFLR5::WOPPVIEW)   m_iPOppIndex = iGraph;
					if (s_pMiarex->m_iView==XFLR5::WPOLARVIEW) m_iWPolarIndex = iGraph;
					if (s_pMiarex->m_iView==XFLR5::WCPVIEW && iGraph>0)	return;
					if (s_pMiarex->m_iView==XFLR5::STABPOLARVIEW)
					{
						if(iGraph>1) return;
						else m_iStabPolarIndex = iGraph;
						s_pMiarex->m_bLongitudinal = (iGraph==0);
					}
					if (s_pMiarex->m_iView==XFLR5::STABTIMEVIEW)
					{
						if(iGraph>3) return;
						else m_iStabTimeIndex = iGraph;
					}
					s_pMiarex->setView(XFLR5::ONEGRAPH);
				}
				m_nGraphWidgets = 1;
				if(iGraph>=0 && iGraph<m_GraphWidget.count())
				{
					m_iActiveGraphWidget = iGraph;
					m_pLegendWidget->setGraph(m_GraphWidget.at(iGraph)->graph());
					adjustLayout();
				}

				s_pMainFrame->checkGraphActions();
				update();
				setFocus();
				return;
			}
		}
		case Qt::Key_T:
		{
			onTwoGraphs();
			return;
		}
		case Qt::Key_F:
		{
			onFourGraphs();
			return;
		}
		case Qt::Key_A:
		{
			onAllGraphs();
			return;
		}
		default:event->ignore();
	}
}



void GraphTileWidget::onResetCurves(Graph *pGraph)
{
	if(!pGraph) return;
	switch(m_xflr5App)
	{
		case XFLR5::XFOILANALYSIS:
		{
			s_pXDirect->updateView();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			break;
		}
		case XFLR5::MIAREX:
		{
			Miarex::s_bResetCurves = true;
			s_pMiarex->updateView();
			break;
		}
		default:
			break;
	}
}



void GraphTileWidget::onSingleGraph()
{
	if(!isVisible()) return;

	QAction *pAction = qobject_cast<QAction*>(sender());
	if (!pAction) return;
	int iGraph = pAction->data().toInt();

	if(iGraph>=m_GraphWidget.count())
	{
		s_pMainFrame->checkGraphActions();
		return;
	}

	if(!m_GraphWidget.at(iGraph)->graph())
	{
		s_pMainFrame->checkGraphActions();
		return;
	}

	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		m_nGraphWidgets = 1;
		m_iActiveGraphWidget = iGraph;
		m_pLegendWidget->setGraph(m_GraphWidget.at(iGraph)->graph());
		s_pXDirect->setView(XFLR5::ONEGRAPH);
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		if (s_pMiarex->m_iView==XFLR5::WCPVIEW && iGraph>0) return;
		if (s_pMiarex->m_iView==XFLR5::STABPOLARVIEW)
		{
			if(iGraph>1) return;
			m_iStabPolarIndex = iGraph;
			s_pMiarex->m_bLongitudinal = (iGraph==0);
		}
		if (s_pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			if(iGraph>3) return;
			m_iStabTimeIndex = iGraph;
		}
		if(m_MiarexView==XFLR5::WOPPVIEW)        m_iPOppIndex = iGraph;
		if(m_MiarexView==XFLR5::WPOLARVIEW)      m_iWPolarIndex = iGraph;
		m_nGraphWidgets = 1;
		m_iActiveGraphWidget = iGraph;

		m_pLegendWidget->setGraph(m_GraphWidget.at(iGraph)->graph());
		s_pMiarex->setView(XFLR5::ONEGRAPH);
	}

	adjustLayout();
	s_pMainFrame->checkGraphActions();
	update();
	setFocus();
}


void GraphTileWidget::onTwoGraphs()
{
	if(!isVisible()) return;

	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		s_pXDirect->setView(XFLR5::TWOGRAPHS);
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		if(s_pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			onSingleGraph();
			return;
		}
		s_pMiarex->setView(XFLR5::TWOGRAPHS);
	}

	m_nGraphWidgets = 2;
	m_iActiveGraphWidget = 0;
	m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());

	adjustLayout();
	s_pMainFrame->checkGraphActions();
	update();
	setFocus();
}


void GraphTileWidget::onFourGraphs()
{
	if(!isVisible()) return;


	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		m_nGraphWidgets = 4;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
		s_pXDirect->setView(XFLR5::FOURGRAPHS);
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		if (s_pMiarex->m_iView==XFLR5::STABPOLARVIEW)
		{
			onTwoGraphs(); //there are only two graphs to display
			return;
		}
		else if(s_pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			onSingleGraph();
			return;
		}
		m_nGraphWidgets = 4;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
		s_pMiarex->setView(XFLR5::FOURGRAPHS);
	}

	adjustLayout();
	s_pMainFrame->checkGraphActions();
	update();
	setFocus();
}


void GraphTileWidget::onAllGraphs()
{
	if(!isVisible()) return;

	if(m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		s_pXDirect->setView(XFLR5::ALLGRAPHS);
		m_nGraphWidgets = 6;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
	}
	else if(m_xflr5App==XFLR5::MIAREX)
	{
		if (s_pMiarex->m_iView==XFLR5::STABPOLARVIEW)
		{
			onTwoGraphs(); //there are only two graphs to display
			return;
		}
		else if (s_pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			onFourGraphs(); //there are only two graphs to display
			return;
		}
		else if(s_pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			onSingleGraph();
			return;
		}

		s_pMiarex->setView(XFLR5::ALLGRAPHS);

		m_nGraphWidgets = 6;
		m_iActiveGraphWidget = 0;
		m_pLegendWidget->setGraph(m_GraphWidget.at(0)->graph());
	}

	adjustLayout();
	s_pMainFrame->checkGraphActions();
	update();
	setFocus();
}



void GraphTileWidget::onCurGraphSettings()
{
	if(!isVisible()) return;
	if(activeGraphWidget())
	{
		activeGraphWidget()->onGraphSettings();
	}
	setFocus();

}


void GraphTileWidget::onResetCurGraphScales()
{
	if(!isVisible()) return;
	if(activeGraphWidget())
	{
		activeGraphWidget()->onResetGraphScales();
	}
	setFocus();
}


void GraphTileWidget::onExportCurGraph()
{
	if(!isVisible()) return;
	Graph *pGraph = activeGraph();
	if(!pGraph) return;
	s_pMainFrame->exportGraph(pGraph);
	setFocus();
}



/**
 * The user has requested an edition of the settings for all WOpp graphs
 * Launches the dialog box and updates the graphs
 */
void GraphTileWidget::onAllGraphSettings()
{
	if(!isVisible()) return;

	GraphDlg grDlg(this);
	grDlg.setActivePage(1);

	if(xflr5App()==XFLR5::MIAREX)
	{
		if(s_pMiarex->m_iView==XFLR5::WPOLARVIEW)    grDlg.setGraph(s_pMiarex->m_WPlrGraph[0]);
		else if(s_pMiarex->m_iView==XFLR5::WOPPVIEW) grDlg.setGraph(s_pMiarex->m_WingGraph[0]);

		if(grDlg.exec() == QDialog::Accepted)
		{
			if(s_pMiarex->m_iView==XFLR5::WPOLARVIEW)
				for(int ig=1; ig<s_pMiarex->m_WingGraph.size(); ig++) s_pMiarex->m_WingGraph[ig]->copySettings(s_pMiarex->m_WingGraph[0]);
			else if(s_pMiarex->m_iView==XFLR5::WOPPVIEW)
				for(int ig=1; ig<s_pMiarex->m_WPlrGraph.size(); ig++) s_pMiarex->m_WPlrGraph[ig]->copySettings(s_pMiarex->m_WPlrGraph[0]);
		}
	}
	else if (m_xflr5App==XFLR5::XFOILANALYSIS)
	{
		if(!s_pXDirect->bPolarView()) grDlg.setGraph(s_pXDirect->CpGraph());
		else                        grDlg.setGraph(s_pXDirect->PlrGraph(0));

		if(grDlg.exec() == QDialog::Accepted)
		{
			if(s_pXDirect->bPolarView())
				for(int ig=1; ig<MAXPOLARGRAPHS; ig++) s_pXDirect->PlrGraph(ig)->copySettings(s_pXDirect->PlrGraph(0));
		}
	}
	update();
	setFocus();
}





/**
 * Updates the display after the user has requested a reset of the scale of all WOpp graphs
 */
void GraphTileWidget::onAllGraphScales()
{
	if(!isVisible()) return;

	if(xflr5App()==XFLR5::MIAREX)
	{
		if(s_pMiarex->m_iView == XFLR5::WOPPVIEW)
		{
			for(int ig=0; ig<s_pMiarex->m_WingGraph.size(); ig++)
			{
				s_pMiarex->m_WingGraph[ig]->setAuto(true);
				s_pMiarex->m_WingGraph[ig]->resetXLimits();
				s_pMiarex->m_WingGraph[ig]->resetYLimits();
				s_pMiarex->m_WingGraph[ig]->setAutoX(false);
				if(s_pMiarex->m_pCurPlane)
				{
					s_pMiarex->m_WingGraph[ig]->setXMin( -s_pMiarex->m_pCurPlane->planformSpan()*Units::mtoUnit());
					s_pMiarex->m_WingGraph[ig]->setXMax(  s_pMiarex->m_pCurPlane->planformSpan()*Units::mtoUnit());
				}
			}
		}
		else if(s_pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			for(int ig=0; ig<s_pMiarex->m_TimeGraph.size(); ig++)
			{
				s_pMiarex->m_TimeGraph[ig]->setAuto(true);
				s_pMiarex->m_TimeGraph[ig]->resetXLimits();
				s_pMiarex->m_TimeGraph[ig]->resetYLimits();
			}
		}
		else if(s_pMiarex->m_iView==XFLR5::WPOLARVIEW)
		{
			for(int ig=0; ig<s_pMiarex->m_WPlrGraph.size(); ig++)
			{
				s_pMiarex->m_WPlrGraph[ig]->setAuto(true);
				s_pMiarex->m_WPlrGraph[ig]->resetXLimits();
				s_pMiarex->m_WPlrGraph[ig]->resetYLimits();
			}
		}
		else if(s_pMiarex->m_iView==XFLR5::WCPVIEW)
		{
			s_pMiarex->m_CpGraph.setAuto(true);
			s_pMiarex->m_CpGraph.resetXLimits();
			s_pMiarex->m_CpGraph.resetYLimits();
			s_pMiarex->m_CpGraph.setInverted(true);
		}
	}
	else if (xflr5App()==XFLR5::XFOILANALYSIS)
	{
		if(!s_pXDirect->bPolarView())
		{
			s_pXDirect->CpGraph()->setAuto(true);
			s_pXDirect->CpGraph()->resetXLimits();
			s_pXDirect->CpGraph()->resetYLimits();
		}
		else
		{
			for(int ig=0; ig<s_pXDirect->PlrGraphSize(); ig++)
			{
				s_pXDirect->PlrGraph(ig)->setAuto(true);
				s_pXDirect->PlrGraph(ig)->resetXLimits();
				s_pXDirect->PlrGraph(ig)->resetYLimits();
			}
		}
	}
	update();
	setFocus();
}




Graph *GraphTileWidget::activeGraph()
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		if(m_GraphWidget.at(igw)->isVisible()&& m_GraphWidget.at(igw)->hasFocus()) return m_GraphWidget.at(igw)->graph();
	}
	return NULL;
}





GraphWidget *GraphTileWidget::activeGraphWidget()
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		if(m_GraphWidget.at(igw)->isVisible()&& m_GraphWidget.at(igw)->hasFocus()) return m_GraphWidget.at(igw);
	}
	return NULL;
}


















