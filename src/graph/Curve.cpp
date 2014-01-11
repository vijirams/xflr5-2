/****************************************************************************

    Curve Class
	Copyright (C) 2003-2014 Andre Deperrois adeperrois@xflr5.com

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


#include "Curve.h"
#include "math.h"
#include "Graph.h"


/**
 * The public constructor
 */
Curve::Curve()
{
    CurveColor = QColor(255,0,0,127);
	m_CurveName = "";
	m_bIsVisible = true;
	m_bShowPoints = false;
	CurveWidth = 1;
	CurveStyle = Qt::SolidLine;
	m_iSelected = -1;
}

/**
 * Appends a point to the end of the data
 * @param xn: x-coordinate
 * @param yn: y-coordinate
 * @return the new number of points in the curve
 */
int Curve::AppendPoint(double xn, double yn)
{
	x.append(xn);
	y.append(yn);
	return size();
}

/**
 * Copies the data and settings from an existing curve
 * @param pCurve: a pointer to the input curve
 */
void Curve::Duplicate(Curve *pCurve)
{
	if(!pCurve) return;

	CopyData(pCurve);

	CurveColor = pCurve->CurveColor;
	CurveStyle = pCurve->CurveStyle;
	CurveWidth = pCurve->CurveWidth;
	m_bIsVisible = pCurve->m_bIsVisible;
	m_bShowPoints = pCurve->m_bShowPoints;
	m_CurveName = pCurve->m_CurveName;
}

/**
 * Copies the data and settings from an existing curve
 * @param pCurve: a pointer to the input curve
 */
void Curve::CopyData(Curve *pCurve)
{
	if(!pCurve) return;
	clear();

	for (int i=0; i<pCurve->size() ;i++)
	{
		x.append(pCurve->x[i]);
		y.append(pCurve->y[i]);
	}
}




/**
 * Returns the index of the curve's point closest to the input coordinates
 * @param xs x coordinate
 * @param ys y coordinate
 * @param &dist distance to the return point
 * @return the index of the closest point
 */
int Curve::closestPoint(double xs, double ys, double &dist )
{
	Graph *pGraph = (Graph*)m_pParentGraph;
	static int ref;
	static double d2;
	ref = -1;
	dist = 1.e10;
	if (size()<1) return -1;
	for(int i=0; i<size(); i++)
	{
		d2 =   (xs-x[i])*(xs-x[i])/pGraph->GetXScale()/pGraph->GetXScale() 
			 + (ys-y[i])*(ys-y[i])/pGraph->GetYScale()/pGraph->GetYScale();
		if (d2<dist)
		{
			dist = d2;
			ref = i;
		}
	}
	return ref;
}



/**
 * Returns the index of the curve's point closest to the input coordinates
 * @overload overloaded function
 * @param xs x coordinate
 * @param ys y coordinate
 * @param &dist distance to the return point
 * @param &n the index of the closest point
 */
void Curve::closestPoint(double xs, double ys, double &dist, int &n)
{
	Graph *pGraph = (Graph*)m_pParentGraph;
	static double d2;
	dist = 1.e10;
	if (n<1) return;
	for(int i=0; i<n; i++)
	{
		d2 =   (xs-x[i])*(xs-x[i])/pGraph->GetXScale()/pGraph->GetXScale()
			 + (ys-y[i])*(ys-y[i])/pGraph->GetYScale()/pGraph->GetYScale();
		if (d2<dist)
		{
			dist = d2;
			n = i;
		}
	}
}


/**
 * Returns the index and the coordinates of the curve's point closest to the input coordinates
 * @param xs x input coordinate
 * @param ys y input coordinate
 * @param xSel x output coordinate
 * @param ySel y output coordinate
 * @param &dist distance to the return point
 * @param &nSel the index of the closest point
 */
void Curve::closestPoint(double const &xs, double const &ys, double &xSel, double &ySel, double &dist, int &nSel)
{
	static double d2;
	dist = 1.e40;

	for(int i=0; i<size(); i++)
	{
		d2 =   (xs-x[i])*(xs-x[i]) + (ys-y[i])*(ys-y[i]);
		if (d2<dist)
		{
			dist = d2;
			xSel = x[i];
			ySel = y[i];
			nSel = i;
		}
	}
}


/**
 * Returns the minimum x value of this curve
 *@return the x min value, or +99999999.0 if the curve has no points
 */
double Curve::xMin()
{
	double xMin = 99999999.0;
//	if(n==0) xmin = .0; 
//	else
		for(int i=0; i<size();i++)
			xMin = qMin(xMin, x[i]);
	return xMin;
}


/**
 * Returns the maximum x value of this curve
 *@return the x max value, or -99999999.0 if the curve has no points
 */
double Curve::xMax()
{
	double xMax = -99999999.0;
//	if(n==0) xmax = 1.0; 
//	else
	for(int i=0; i<size();i++)
			xMax = qMax(xMax, x[i]);
	return xMax;
}


/**
 * Returns the minimum y value of this curve
 *@return the y min value, or +99999999.0 if the curve has no points
 */
double Curve::yMin()
{
	double yMin = 99999999.0;
//	if(n==0) ymin = .0; 
//	else
	for(int i=0; i<size();i++)
			yMin = qMin(yMin, y[i]);
	return yMin;
}


/**
 * Returns the maximum y value of this curve
 *@return the y max value, or -99999999.0 if the curve has no points
 */
double Curve::yMax()
{
	double yMax = -99999999.0;
//	if(n==0) ymax = 1.0; 
//	else
		for(int i=0; i<size();i++)
			yMax = qMax(yMax, y[i]);
	return yMax;
}

