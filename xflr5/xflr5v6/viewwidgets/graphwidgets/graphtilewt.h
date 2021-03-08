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


#ifndef GRAPHTILEWIDGET_H
#define GRAPHTILEWIDGET_H

#include <QWidget>
#include <QSplitter>

#include <graph/graph.h>


#include <globals/gui_enums.h>
#include <xflanalysis/analysis3d_enums.h>

class LegendWt;
class GraphWt;
class Miarex;
class XDirect;
class Graph;

class GraphTileWidget : public QWidget
{
    friend class XDirectTileWidget;
    friend class MiarexTileWidget;

    Q_OBJECT
public:
    GraphTileWidget(QWidget *parent = nullptr);
    virtual ~GraphTileWidget();

    Graph *graph(int iGraph);
    GraphWt *graphWidget(int iGraph);

    XFLR5::enumApp xflr5App() const {return m_xflr5App;}

    int graphWidgetCount() const {return m_GraphWidget.count();}
    int activeGraphIndex() const {return m_iActiveGraphWidget;}

    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent (QContextMenuEvent *event);
    void showEvent(QShowEvent *event);


    virtual void setGraphList(QVector<Graph*>pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation =Qt::Horizontal);
    virtual void connectSignals() = 0;


    GraphWt *graphWidget(Graph *pGraph);
    Graph *activeGraph();
    GraphWt *activeGraphWidget();

private:
    virtual void adjustLayout() = 0;
    virtual void setupMainLayout() = 0;


public slots:
    void onResetCurves(Graph *pGraph = nullptr);

    void onSingleGraph();
    void onAllGraphSettings();
    void onAllGraphScales();


    void onTwoGraphs();
    void onFourGraphs();
    void onAllGraphs();
    void onCurGraphSettings();
    void onResetCurGraphScales();
    void onExportCurGraph();


public:
    static MainFrame *s_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
    static Miarex *s_pMiarex;      /**< A void pointer to the instance of the QMiarex object. */
    static XDirect *s_pXDirect;     /**< A void pointer to the instance of the QXDirect object. */


private:
    QVector<GraphWt*>m_GraphWidget;
    LegendWt *m_pLegendWidget;
    QSplitter *m_pMainSplitter;

    int m_nGraphWidgets;
    int m_iActiveGraphWidget;

    int m_iPOppIndex, m_iWPolarIndex, m_iStabPolarIndex, m_iStabTimeIndex;


    XFLR5::enumApp m_xflr5App;
    XFLR5::enumMiarexViews m_MiarexView;

    Qt::Orientation m_SingleGraphOrientation;
};

#endif // GRAPHTILEWIDGET_H
