/****************************************************************************

	UnitsDlg Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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

#include "../globals.h"
#include "Units.h"
#include <QGridLayout>
#include <QVBoxLayout>

double Units::s_mtoUnit  = 1.0;
double Units::s_mstoUnit = 1.0;
double Units::s_m2toUnit = 1.0;
double Units::s_kgtoUnit = 1.0;
double Units::s_NtoUnit  = 1.0;
double Units::s_NmtoUnit = 1.0;
int Units::s_LengthUnit = 3;
int Units::s_SpeedUnit  = 0;
int Units::s_AreaUnit   = 3;
int Units::s_WeightUnit = 1;
int Units::s_ForceUnit  = 0;
int Units::s_MomentUnit = 0;


Units::Units(QWidget *parent): QDialog(parent)
{
	m_bLengthOnly = false;
	m_Question = tr("Select units for this project :");
	setWindowTitle(tr("Units Dialog"));
	SetupLayout();
}


void Units::SetupLayout()
{
	QGridLayout *UnitsLayout = new QGridLayout;
	{
		QLabel *lab1 = new QLabel(tr("Length"));
		QLabel *lab2 = new QLabel(tr("Area"));
		QLabel *lab3 = new QLabel(tr("Speed"));
		QLabel *lab4 = new QLabel(tr("Mass"));
		QLabel *lab5 = new QLabel(tr("Force"));
		QLabel *lab6 = new QLabel(tr("Moment"));
		UnitsLayout->addWidget(lab1, 1,1);
		UnitsLayout->addWidget(lab2, 2,1);
		UnitsLayout->addWidget(lab3, 3,1);
		UnitsLayout->addWidget(lab4, 4,1);
		UnitsLayout->addWidget(lab5, 5,1);
		UnitsLayout->addWidget(lab6, 6,1);

		m_pctrlQuestion = new QLabel(tr("Define the project units"));

		m_pctrlLengthFactor = new QLabel(" ");
		m_pctrlSurfaceFactor = new QLabel(" ");
		m_pctrlWeightFactor = new QLabel(" ");
		m_pctrlSpeedFactor = new QLabel(" ");
		m_pctrlForceFactor = new QLabel(" ");
		m_pctrlMomentFactor = new QLabel(" ");
		m_pctrlLengthFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSurfaceFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlWeightFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSpeedFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlForceFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlMomentFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);


		UnitsLayout->addWidget(m_pctrlLengthFactor, 1,2);
		UnitsLayout->addWidget(m_pctrlSurfaceFactor, 2,2);
		UnitsLayout->addWidget(m_pctrlSpeedFactor, 3,2);
		UnitsLayout->addWidget(m_pctrlWeightFactor, 4,2);
		UnitsLayout->addWidget(m_pctrlForceFactor, 5,2);
		UnitsLayout->addWidget(m_pctrlMomentFactor, 6,2);

		m_pctrlLength  = new QComboBox;
		m_pctrlSurface = new QComboBox;
		m_pctrlSpeed   = new QComboBox;
		m_pctrlWeight  = new QComboBox;
		m_pctrlForce   = new QComboBox;
		m_pctrlMoment  = new QComboBox;
		UnitsLayout->addWidget(m_pctrlLength,  1,3);
		UnitsLayout->addWidget(m_pctrlSurface, 2,3);
		UnitsLayout->addWidget(m_pctrlSpeed,   3,3);
		UnitsLayout->addWidget(m_pctrlWeight,  4,3);
		UnitsLayout->addWidget(m_pctrlForce,   5,3);
		UnitsLayout->addWidget(m_pctrlMoment,  6,3);


		m_pctrlLengthInvFactor = new QLabel(" ");
		m_pctrlSurfaceInvFactor = new QLabel(" ");
		m_pctrlWeightInvFactor = new QLabel(" ");
		m_pctrlSpeedInvFactor = new QLabel(" ");
		m_pctrlForceInvFactor = new QLabel(" ");
		m_pctrlMomentInvFactor = new QLabel(" ");
		m_pctrlLengthInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSurfaceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlWeightInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlSpeedInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlForceInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		m_pctrlMomentInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		UnitsLayout->addWidget(m_pctrlLengthInvFactor, 1,4);
		UnitsLayout->addWidget(m_pctrlSurfaceInvFactor, 2,4);
		UnitsLayout->addWidget(m_pctrlSpeedInvFactor, 3,4);
		UnitsLayout->addWidget(m_pctrlWeightInvFactor, 4,4);
		UnitsLayout->addWidget(m_pctrlForceInvFactor, 5,4);
		UnitsLayout->addWidget(m_pctrlMomentInvFactor, 6,4);
		UnitsLayout->setColumnStretch(4,2);
		UnitsLayout->setColumnMinimumWidth(4,220);
	}

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		OKButton      = new QPushButton(tr("OK"));
		CancelButton  = new QPushButton(tr("Cancel"));
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
	}

	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		MainLayout->addWidget(m_pctrlQuestion);
		MainLayout->addLayout(UnitsLayout);
		MainLayout->addStretch(1);
		MainLayout->addSpacing(20);
		MainLayout->addLayout(CommandButtons);
		MainLayout->addStretch(1);
	}

	setLayout(MainLayout);

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlLength, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
	connect(m_pctrlSurface, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
	connect(m_pctrlSpeed, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
	connect(m_pctrlWeight, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
	connect(m_pctrlForce, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
	connect(m_pctrlMoment, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
}


void Units::InitDialog()
{
	QStringList list;
	list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
	m_pctrlLength->clear();
	m_pctrlLength->addItems(list);		//5

	m_pctrlSurface->clear();
	m_pctrlSurface->addItem(QString::fromUtf8("mm²"));		//0
	m_pctrlSurface->addItem(QString::fromUtf8("cm²"));		//1
	m_pctrlSurface->addItem(QString::fromUtf8("dm²"));		//2
	m_pctrlSurface->addItem(QString::fromUtf8("m²"));		//3
	m_pctrlSurface->addItem(QString::fromUtf8("in²"));		//4
	m_pctrlSurface->addItem(QString::fromUtf8("ft²"));		//5

	m_pctrlSpeed->clear();
	m_pctrlSpeed->addItem("m/s");		//0
	m_pctrlSpeed->addItem("km/h");		//1
	m_pctrlSpeed->addItem("ft/s");		//2
	m_pctrlSpeed->addItem("kt (int.)");	//3
	m_pctrlSpeed->addItem("mph");		//4

	m_pctrlWeight->clear();
	m_pctrlWeight->addItem("g");		//0
	m_pctrlWeight->addItem("kg");		//1
	m_pctrlWeight->addItem("oz");		//2
	m_pctrlWeight->addItem("lb");		//3

	m_pctrlForce->clear();
	m_pctrlForce->addItem("N");		//0
	m_pctrlForce->addItem("lbf");		//1

	m_pctrlMoment->clear();
	m_pctrlMoment->addItem("N.m");	//0
	m_pctrlMoment->addItem("lbf.in");	//1
	m_pctrlMoment->addItem("lbf.ft");	//2

	m_pctrlLength->setCurrentIndex(s_LengthUnit);
	m_pctrlWeight->setCurrentIndex(s_WeightUnit);
	m_pctrlSurface->setCurrentIndex(s_AreaUnit);
	m_pctrlSpeed->setCurrentIndex(s_SpeedUnit);
	m_pctrlForce->setCurrentIndex(s_ForceUnit);
	m_pctrlMoment->setCurrentIndex(s_MomentUnit);

	m_pctrlLength->setFocus();
	OnSelChanged(" ");

	if(m_bLengthOnly)
	{
		m_pctrlSpeed->setEnabled(false);
		m_pctrlSurface->setEnabled(false);
		m_pctrlWeight->setEnabled(false);
		m_pctrlForce->setEnabled(false);
		m_pctrlMoment->setEnabled(false);
	}
	m_pctrlQuestion->setText(m_Question);
}


void Units::OnSelChanged(const QString &)
{
	s_LengthUnit  = m_pctrlLength->currentIndex();
	s_AreaUnit    = m_pctrlSurface->currentIndex();
	s_WeightUnit  = m_pctrlWeight->currentIndex();
	s_SpeedUnit   = m_pctrlSpeed->currentIndex();
	s_ForceUnit   = m_pctrlForce->currentIndex();
	s_MomentUnit  = m_pctrlMoment->currentIndex();

	SetUnitConversionFactors();

	QString str, strange;

	getLengthUnitLabel(str);
	strange= QString("     1 m = %1").arg(s_mtoUnit,15,'f',5);
	m_pctrlLengthFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 m").arg(1./s_mtoUnit,15,'f',5);
	m_pctrlLengthInvFactor->setText(strange);


	getAreaUnitLabel(str);
	strange= QString(QString::fromUtf8("     1 m² = %1")).arg(s_m2toUnit,15,'f',5);
	m_pctrlSurfaceFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 m").arg(1./s_m2toUnit,15,'f',5);
	strange += QString::fromUtf8("²");
	m_pctrlSurfaceInvFactor->setText(strange);

	getWeightUnitLabel(str);
	strange= QString("     1 kg = %1").arg(s_kgtoUnit,15,'f',5);
	m_pctrlWeightFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 kg").arg(1./s_kgtoUnit,15,'f',5);
	m_pctrlWeightInvFactor->setText(strange);

	getSpeedUnitLabel(str);
	strange= QString("     1 m/s = %1").arg(s_mstoUnit,15,'f',5);
	m_pctrlSpeedFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 m/s").arg(1./s_mstoUnit,15,'f',5);
	m_pctrlSpeedInvFactor->setText(strange);

	getForceUnitLabel(str);
	strange= QString("     1 N = %1").arg(s_NtoUnit,15,'f',5);
	m_pctrlForceFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 N").arg(1./s_NtoUnit,15,'f',5);
	m_pctrlForceInvFactor->setText(strange);

	getMomentUnitLabel(str);
	strange= QString("     1 N.m = %1").arg(s_NmtoUnit,15,'f',5);
	m_pctrlMomentFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 N.m").arg(1./s_NmtoUnit,15,'f',5);
	m_pctrlMomentInvFactor->setText(strange);

}




/**
 * Returns the name of the user-selected area unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the area unit
 *@param unit the index of the area unit
 */
void Units::getAreaUnitLabel(QString &str)
{
	switch(s_AreaUnit)
	{
		case 0:
		{
			str="mm"+QString::fromUtf8("²");
			break;
		}
		case 1:
		{
			str="cm"+QString::fromUtf8("²");
			break;
		}
		case 2:
		{
			str="dm"+QString::fromUtf8("²");
			break;
		}
		case 3:
		{
			str="m"+QString::fromUtf8("²");
			break;
		}
		case 4:
		{
			str="in"+QString::fromUtf8("²");
			break;
		}
		case 5:
		{
			str="ft"+QString::fromUtf8("²");
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}



/**
 * Returns the name of the user-selected length unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the length unit
 *@param unit the index of the length unit
 */
void Units::getLengthUnitLabel(QString &str)
{
	switch(s_LengthUnit)
	{
		case 0:
		{
			str="mm";
			break;
		}
		case 1:
		{
			str="cm";
			break;
		}
		case 2:
		{
			str="dm";
			break;
		}
		case 3:
		{
			str="m";
			break;
		}
		case 4:
		{
			str="in";
			break;
		}
		case 5:
		{
			str="ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the user-selected force unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the force unit
 *@param unit the index of the force unit
 */
void Units::getForceUnitLabel(QString &str)
{
	switch(s_ForceUnit)
	{
		case 0:{
			str="N";
			break;
		}
		case 1:{
			str="lbf";
			break;
		}

		default:{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the user-selected moment unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the moment unit
 *@param unit the index of the moment unit
 */
void Units::getMomentUnitLabel(QString &str)
{
	switch(s_MomentUnit)
	{
		case 0:
		{
			str="N.m";
			break;
		}
		case 1:
		{
			str="lbf.in";
			break;
		}
		case 2:
		{
			str="lbf.ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}



/**
 * Returns the name of the user-selected speed unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the speed unit
 *@param unit the index of the speed unit
 */
void Units::getSpeedUnitLabel(QString &str)
{
	switch(s_SpeedUnit){
		case 0:{
			str="m/s";
			break;
		}
		case 1:{
			str="km/h";
			break;
		}
		case 2:{
			str="ft/s";
			break;
		}
		case 3:{
			str="kt";
			break;
		}
		case 4:{
			str="mph";
			break;
		}
		default:{
			str=" ";
			break;
		}
	}
}


/**
 * Returns the name of the user-selected mass unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the mass unit
 *@param unit the index of the mass unit
 */
void Units::getWeightUnitLabel(QString &str)
{
	switch(s_WeightUnit)
	{
		case 0:{
			str="g";
			break;
		}
		case 1:{
			str="kg";
			break;
		}
		case 2:{
			str="oz";
			break;
		}
		case 3:{
			str="lb";
			break;
		}
		default:{
			str=" ";
			break;
		}
	}
}



/**
* Initializes the conversion factors for all user-defined units
*/
void Units::SetUnitConversionFactors()
{
	switch(s_LengthUnit)
	{
		case 0:
		{//mdm
			s_mtoUnit  = 1000.0;
			break;
		}
		case 1:{//cm
			s_mtoUnit  = 100.0;
			break;
		}
		case 2:{//dm
			s_mtoUnit  = 10.0;
			break;
		}
		case 3:{//m
			s_mtoUnit  = 1.0;
			break;
		}
		case 4:{//in
			s_mtoUnit  = 1000.0/25.4;
			break;
		}
		case 5:{///ft
			s_mtoUnit  = 1000.0/25.4/12.0;
			break;
		}
		default:{//m
			s_mtoUnit  = 1.0;
			break;
		}
	}
	switch(s_AreaUnit)
	{
		case 0:{//mm²
			s_m2toUnit = 1000000.0;
			break;
		}
		case 1:{//cm²
			s_m2toUnit = 10000.0;
			break;
		}
		case 2:{//dm²
			s_m2toUnit = 100.0;
			break;
		}
		case 3:{//m²
			s_m2toUnit = 1.0;
			break;
		}
		case 4:{//in²
			s_m2toUnit = 1./0.254/0.254*100.0;
			break;
		}
		case 5:{//ft²
			s_m2toUnit = 1./0.254/0.254/144.0*100.0;
			break;
		}
		default:{
			s_m2toUnit = 1.0;
			break;
		}
	}

	switch(s_WeightUnit){
		case 0:{///g
			s_kgtoUnit = 1000.0;
			break;
		}
		case 1:{//kg
			s_kgtoUnit = 1.0;

			break;
		}
		case 2:{//oz
			s_kgtoUnit = 1./ 2.83495e-2;
			break;
		}
		case 3:{//lb
			s_kgtoUnit = 1.0/0.45359237;
			break;
		}
		default:{
			s_kgtoUnit = 1.0;
			break;
		}
	}
	switch(s_SpeedUnit){
		case 0:{// m/s
			s_mstoUnit = 1.0;
			break;
		}
		case 1:{// km/h
			s_mstoUnit = 3600.0/1000.0;
			break;
		}
		case 2:{// ft/s
			s_mstoUnit = 100.0/2.54/12.0;
			break;
		}
		case 3:{// kt (int.)
			s_mstoUnit = 1.0/0.514444;
			break;
		}
		case 4:{// mph
			s_mstoUnit = 3600.0/1609.344;
			break;
		}
		default:{
			s_mstoUnit = 1.0;
			break;
		}
	}

	switch(s_ForceUnit){
		case 0:{//N
			s_NtoUnit = 1.0;
			break;
		}
		case 1:{//lbf
			s_NtoUnit = 1.0/4.44822;
			break;
		}
		default:{
			s_NtoUnit = 1.0;
			break;
		}
	}
	switch(s_MomentUnit)
	{
		case 0:{//N.m
			s_NmtoUnit = 1.0;
			break;
		}
		case 1:{//lbf.in
			s_NmtoUnit = 1.0/4.44822/0.0254;
			break;
		}
		case 2:{//lbf.0t
			s_NmtoUnit = 1.0/4.44822/12.0/0.0254;
			break;
		}
		default:{
			s_NmtoUnit = 1.0;
			break;
		}
	}
}



QString Units::lengthUnitLabel()
{
	QString str;
	getLengthUnitLabel(str);
	return str;
}

QString Units::speedUnitLabel()
{
	QString str;
	getSpeedUnitLabel(str);
	return str;
}

QString Units::weightUnitLabel()
{
	QString str;
	getWeightUnitLabel(str);
	return str;
}


QString Units::areaUnitLabel()
{
	QString str;
	getAreaUnitLabel(str);
	return str;
}


QString Units::momentUnitLabel()
{
	QString str;
	getMomentUnitLabel(str);
	return str;
}


QString Units::forceUnitLabel()
{
	QString str;
	getForceUnitLabel(str);
	return str;
}


