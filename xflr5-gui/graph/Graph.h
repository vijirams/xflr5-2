/****************************************************************************

    Graph Classes
        Copyright (C) 2003-2016 Andre Deperrois adeperrois@xflr5.com

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


#ifndef GRAPH_H
#define GRAPH_H

#include <QFile>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <QList>
#include "Curve.h"

class Graph 
{
	friend class QGraph;
	friend class GraphWidget;
//	friend class OpPointWidget;
//	friend class QXDirect;

public:

	bool bAutoX();
	bool bAutoY();
	bool bAutoXMin();
	bool bAutoYMin();
	bool hasBorder();
	bool bInverted();
	bool isInDrawRect(int const &x, int const &y);
	bool isInDrawRect(QPoint const &pt);
	bool initializeGraph(int width=0, int height=0);

	double clientTox(int x);
	double clientToy(int y);

	double clientTox(double x);
	double clientToy(double y);

	int xToClient(double x);
	int yToClient(double y);

	void copySettings(Graph* pGraph, bool bScales=true);
	void deselectPoint();
	Curve * getCurvePoint(const int &xClt, const int &yClt, int &nSel);
	Curve * getClosestPoint(double const &x, double const &y, double &xSel, double &ySel, int &nSel);
	void resetLimits();
	void resetCurves();
	void scaleAxes(double zoom);
	void scaleXAxis(double zoom);
	void scaleYAxis(double zoom);
	void setAutoXMinUnit(bool bAuto);
	void setAutoYMinUnit(bool bAuto);
	void setAutoXUnit();
	void setAutoYUnit();
	void setAxisData(int s, int w, QColor clr);
	void setBkColor(QColor cr);
	void setBorderColor(QColor crBorder);
	void setBorder(bool bBorder);
	void setBorderStyle(int s);
	void setBorderWidth(int w);
	void setDrawRect(QRect Rect);
	void setMargin(int m);
	void setInverted(bool bInverted);
	void setType(int type);
	void setXTitle(QString str);
	void setYTitle(QString str);
	void setX0(double f);
	void setXMax(double f);
	void setXMin(double f);
	void setXMinorUnit(double f);
	void setXUnit(double f);
	void setY0(double f);
	void setYMax(double f);
	void setYMin(double f);
	void setYMinorUnit(double f);
	void setYUnit(double f);
	void setXMajGrid(bool const &state, QColor const &clr, int const &style, int const &width);
	void setYMajGrid(bool const &state, QColor const &clr, int const &style, int const &width);
	void setXMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit = -1.0);
	void setYMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit = -1.0);
	void setAuto(bool bAuto);
	void setAutoX(bool bAuto);
	void setAutoY(bool bAuto);
	void setAxisColor(QColor crColor);
	void setAxisStyle(int nStyle);
	void setAxisWidth(int Width);
	void setTitleColor(QColor crColor);
	void setLabelColor(QColor crColor);
	void setWindow(double x1, double x2, double y1, double y2);

	QColor axisColor();
	QColor titleColor();
	QColor labelColor();

	int margin();
	int axisStyle();
	int axisWidth();
	int xVariable();
	int yVariable();
	void setVariables(int const & X, int const & Y);
	void setXVariable(int const & X);
	void setYVariable(int const & Y);
	double xOrigin();
	double xMin();
	double xMax();
	double xUnit();
	double yOrigin();
	double yMin();
	double yMax();
	double yUnit();
	double xScale();
	double yScale();
	
	bool bXMajGrid();
	bool yMajGrid();
	bool bXMinGrid();
	bool bYMinGrid();
	bool selectPoint(QString const &CurveName, int sel);
	bool setXScale();
	bool setYScale();
	
	void setXMajGrid(bool const &bGrid);
	void setYMajGrid(bool const &bGrid);
	void setXMinGrid(bool const &bGrid);
	void setYMinGrid(bool const &bGrid);
	void bXMajGrid(bool &bstate, QColor &clr, int &style, int &width);
	void yMajGrid(bool &bstate, QColor &clr, int &style, int &width);
	void bXMinGrid(bool &bstate, bool &bAuto, QColor &clr, int &style, int &width, double &unit);
	void bYMinGrid(bool &bstate, bool &bAuto, QColor &clr, int &style, int &width, double &unit);
	QString xTitle(){return m_XTitle;}
	QString yTitle(){return m_YTitle;}

	QRect *clientRect();

	void setGraphDefaults(bool bDark=true);
	void setGraphName(QString GraphName);
	void graphName(QString &GraphName);

	QString graphName(){return m_GraphName;}
	Curve* curve(int nIndex);
	Curve* curve(QString CurveTitle);
	Curve* addCurve();
	Curve* addCurve(Curve *pCurve);
	void deleteCurve(int index);
	void deleteCurve(Curve *pCurve);
	void deleteCurve(QString CurveTitle);
	void deleteCurves();
	void resetXLimits();
	void resetYLimits();

	int curveCount() {return m_oaCurves.size();}
	QList<void*> *getCurves(){return &m_oaCurves;}


	QColor backgroundColor(){return m_BkColor;}
	QColor borderColor() {return m_BorderColor;}
	int borderStyle(){return m_BorderStyle;}
	int borderWidth(){return m_BorderWidth;}


	Graph();
	virtual ~Graph();

	static QColor m_CurveColors[10];

private:

	QString m_GraphName;        /** The graph's name, used for little else than to identify it in the settings file */

	int m_Type;

	QRect m_rCltRect;         //in screen coordinates

	bool m_bXMajGrid, m_bXMinGrid;
	bool m_bYMajGrid, m_bYMinGrid;

	bool m_bXAutoMinGrid, m_bYAutoMinGrid;

	bool m_bYInverted;
	bool m_bAutoX, m_bAutoY;
	bool m_bBorder;

	int m_AxisStyle;// axis style
	int m_AxisWidth;// axis width

	int m_XMajStyle, m_YMajStyle;
	int m_XMajWidth, m_YMajWidth;
	QColor m_XMajClr,   m_YMajClr;
	int m_XMinStyle, m_YMinStyle;
	int m_XMinWidth, m_YMinWidth;
	QColor m_XMinClr,   m_YMinClr;
	double m_XMinorUnit,m_YMinorUnit;

	QString m_XTitle;
	QString m_YTitle;
	QList<void*> m_oaCurves;

	QPoint m_ptoffset; //in screen coordinates, w.r.t. the client area

	int exp_x, exp_y;
	double xo, yo;
	double xunit, yunit;
	double xmin, ymin, xmax, ymax;
	double Cxmin, Cxmax, Cymin, Cymax;
	double m_scalex, m_scaley;
	double m_h, m_w; //graph width and height
	int m_iMargin;
	QColor m_AxisColor;


	QColor m_TitleColor;
	QColor m_LabelColor;

	QColor m_BkColor;
	QColor m_BorderColor;
	int m_BorderStyle;
	int m_BorderWidth;


	int m_X, m_Y; //index of X and Y variables
};

#endif
