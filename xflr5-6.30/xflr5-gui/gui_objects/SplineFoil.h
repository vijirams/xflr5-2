/****************************************************************************

    Spline Foil Class
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





/**
 *@file This class defines the splined foil object used in foil design.
 */


#ifndef SPLINEFOIL_H
#define SPLINEFOIL_H


#include "Spline5.h"


/**
*@class SplineFoil
*@brief  The class which defines the splined foil object.

The foil is contructed based on one spline for the upper surface and one spline for the lower surface.
*/
class SplineFoil
{
	friend class QAFoil;
	friend class SplineCtrlsDlg;
	friend class FoilTableDelegate;


public:
	SplineFoil();
	SplineFoil(SplineFoil *pSF);

	bool isVisible(){return m_bVisible;}
	bool isSymetric(){return m_bSymetric;}
	bool showOutPoints(){return m_bOutPoints;}
	bool showCenterLine(){return m_bCenterLine;}
	int splineFoilWidth(){return m_FoilWidth;}
	int splineFoilStyle(){return m_FoilStyle;}
	int splinePointStyle(){return m_PointStyle;}
	QColor splineFoilColor(){return m_FoilColor;}
	QString splineFoilName(){return m_strFoilName;}

	Spline *extrados(){return &m_Extrados;}
	Spline *intrados(){return &m_Intrados;}

	bool isModified(){return m_bModified;}
	void setModified(bool bModified){m_bModified = bModified;}

	void setVisible(bool bVisible){m_bVisible = bVisible;}

	void compMidLine();

	void initSplineFoil();

	bool serialize(QDataStream &ar, bool bIsStoring);
	bool serializeXFL(QDataStream &ar, bool bIsStoring);

	void copy(SplineFoil* pSF);
	void drawCtrlPoints(QPainter &painter, double scalex, double scaley, QPointF Offset);
	void drawMidLine(QPainter &painter, double scalex, double scaley, QPointF Offset);
	void drawFoil(QPainter &painter, double scalex, double scaley, QPointF Offset);
	void drawOutPoints(QPainter &painter, double scalex, double scaley, QPointF Offset);
	void exportToBuffer(void *pFoilPtr);
	void exportToFile(QTextStream &out);
	void updateSplineFoil();
	void setCurveParams(int style, int width, QColor color);

	double camber() {return m_fCamber;}
	double xCamber() {return m_fxCambMax;}
	double thickness() {return m_fThickness;}
	double xThickness() {return m_fxThickMax;}

private:
	bool m_bModified;                /**< false if the SplineFoil has been serialized in its current dtate, false otherwise */
	bool m_bVisible;                 /**< true if this SplineFoil object is visible */
	bool m_bOutPoints;               /**< true if the ouput line points should be displayed */
	bool m_bCenterLine;              /**< true if the SplineFoil's mean camber line is to be displayed */
	bool m_bSymetric;                /**< true if the SplineFoil is symetric. In which case the lower surface is set as symetric of the upper surface. */

	int m_OutPoints;                 /**< the number of output points with which to draw the SplineFoil. */

	int m_PointStyle;                /**< the index of the style for the SplineFoil's points*/
	int m_FoilStyle;                 /**< the index of the style with which to draw the SplineFoil */
	int m_FoilWidth;                 /**< the width with which to draw the SplineFoil */
	QColor m_FoilColor;              /**< the color with which to draw the SplineFoil */

	double m_fCamber;                /**< the SplineFoil's max camber */
	double m_fThickness;             /**< the SplineFoil's max thickness */
	double m_fxCambMax;              /**< the x-position of the SplineFoil's max camber point */
	double m_fxThickMax;             /**< the x-position of the SplineFoil's max thickness point */
	QString m_strFoilName;           /**< the SplineFoil's name */
	Spline5 m_Extrados;               /**< the spline which defines the upper surface */
	Spline5 m_Intrados;               /**< the spline which defines the lower surface */
	CVector m_rpMid[MIDPOINTCOUNT];  /**< the points on the SplineFoil's mid camber line @todo replace with a QVarLengthArray */
};
#endif
