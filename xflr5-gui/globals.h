/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois adeperrois@xflr5.com

	All rights reserved.

*****************************************************************************/

/**
* @file
* This file contains the declaration of methods used throughout the program and not specific to one application.
*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QString>
#include <QColor>
#include <QFile>
#include <QPainter>
#include <complex>

#include <gui_params.h>
#include <engine_enums.h>
#include <objects2d/Foil.h>
#include <objects2d/Polar.h>

using namespace std;


int readValues(QString line, double &x, double &y, double &z);

void ExpFormat(double &f, int &exp);
void ReynoldsFormat(QString &str, double f);


float GLGetRed(double tau);
float GLGetGreen(double tau);
float GLGetBlue(double tau);


QColor randomColor(bool bLightColor);

void Trace(int n);
void Trace(QString msg);
void Trace(QString msg, bool b);
void Trace(QString msg, int n);
void Trace(QString msg, double f);


Qt::PenStyle getStyle(int s);



void GLLineStipple(int style);


XFLR5::enumPanelDistribution distributionType(QString strDist);
QString distributionType(XFLR5::enumPanelDistribution dist);

XFLR5::enumBodyLineType bodyPanelType(QString strPanelType);
QString bodyPanelType(XFLR5::enumBodyLineType panelType);


XFOIL::enumPolarType polarType(QString strPolarType);
QString polarType(XFOIL::enumPolarType polarType);

XFLR5::enumPolarType WPolarType(QString strPolarType);
QString WPolarType(XFLR5::enumPolarType polarType);


XFLR5::enumAnalysisMethod analysisMethod(QString strAnalysisMethod);
QString analysisMethod(XFLR5::enumAnalysisMethod analysisMethod);


XFLR5::enumBC boundaryCondition(QString strBC);
QString boundaryCondition(XFLR5::enumBC boundaryCondition);

void * readFoilFile(QFile &xFoilFile);
void * readPolarFile(QFile &plrFile, QList<Polar*> &polarList);

void drawFoil(QPainter &painter, Foil*pFoil, double const &alpha, double const &scalex, double const &scaley, QPointF const &Offset);
void drawMidLine(QPainter &painter, Foil*pFoil, double const &scalex, double const &scaley, QPointF const &Offset);
void drawPoints(QPainter &painter, Foil*pFoil, double alpha, double const &scalex, double const &scaley, QPointF const &Offset);
void drawPoint(QPainter &painter, int pointStyle, QPoint pt);

bool stringToBool(QString str);
QString referenceDimension(XFLR5::enumRefDimension refDimension);
XFLR5::enumRefDimension referenceDimension(QString strRefDimension);

XFLR5::enumWingType wingType(QString strWingType);
QString wingType(XFLR5::enumWingType wingType);

void setAutoWPolarName(void * ptrWPolar, void *ptrPlane);


void ReynoldsFormat(QString &str, double f);

QColor getColor(int r, int g, int b, int a=255);
QColor colour(OpPoint *pOpp);
QColor colour(Polar *pPolar);
QColor colour(Foil *pFoil);
void setRandomFoilColor(Foil *pFoil, bool bLightTheme);




void readString(QDataStream &ar, QString &strong);
void writeString(QDataStream &ar, QString const &strong);
void readColor(QDataStream &ar, int &r, int &g, int &b);
void writeColor(QDataStream &ar, int r, int g, int b);

void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeColor(QDataStream &ar, int r, int g, int b, int a);


bool serializeFoil(Foil*pFoil, QDataStream &ar, bool bIsStoring);
bool serializePolar(Polar *pPolar, QDataStream &ar, bool bIsStoring);


#endif // FUNCTIONS_H
 
