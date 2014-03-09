
/****************************************************************************

	XFoilTask Class
	   Copyright (C) 2011 Andre Deperrois adeperrois@xflr5.com

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


#include "BatchThreadDlg.h"
#include "XFoilTask.h"
#include <QThread>
#include <QCoreApplication>
#include <QApplication>
#include <QTimerEvent>
#include <QtDebug>

int XFoilTask::s_IterLim=100;
bool XFoilTask::s_bAutoInitBL = true;
bool XFoilTask::s_bCancel = false;
bool XFoilTask::s_bSkipOpp = false;
bool XFoilTask::s_bSkipPolar = false;

/**
* The public constructor
*/
XFoilTask::XFoilTask(QWidget *pParent)
{
	m_pParent = pParent;
	m_Id = -1;
	m_pFoil  = NULL;
	m_pPolar = NULL;
	m_bIsFinished = true;

	m_AlphaMin = m_AlphaMax = m_AlphaInc = 0.0;
	m_ClMin    = m_ClMax    = m_ClInc    = 0.0;
	m_ReMin    = m_ReMax    = m_ReInc    = 0.0;

	m_bAlpha = true;
	m_bFromZero = false;
	m_bInitBL = true;

	m_OutMessage.clear();
	m_OutStream.setDevice(NULL);

	m_bErrors = false;

	m_pGraph = NULL;
	m_pCurve0 = m_pCurve1 = NULL;

	setAutoDelete(false);
}


/**
* Implements the run method of the QRunnable virtual base method
*
* Asssumes that XFoil has been initialized with Foil and Polar
*/
void XFoilTask::run()
{
	if(s_bCancel || !m_pPolar || !m_pFoil)
	{
		m_bIsFinished = true;
		return;
	}

	if(m_pPolar->m_PolarType!=FIXEDAOAPOLAR) AlphaSequence();
	else                                     ReSequence();

	m_bIsFinished = true;

	// For multithreaded analysis, post an event to notify parent window.
	// Could emit a signal, but signals are converted to events anyway when sent across threads
	QTimerEvent *event = new QTimerEvent(m_Id);
	QApplication::postEvent(m_pParent, event);
	//will be truly finished whent this message has been received by the parent batch analysis
}


/**
* Initializes the XFoil calculation
* @param pFoil a pointer to the instance of the Foil object for which the calculation is run
* @param pPolar a pointer to the instance of the Polar object for which the calculation is run
* @return true if the initialization of the Foil in XFoil has been sucessful, false otherwise
*/
bool XFoilTask::InitializeTask(Foil *pFoil, Polar *pPolar, bool bViscous, bool bInitBL, bool bFromZero)
{
	s_bCancel = false;
	s_bSkipOpp = s_bSkipPolar = false;

	XFoil::s_bCancel = false;
	m_bErrors = false;
	m_pFoil = pFoil;
	m_pPolar = pPolar;

	m_bInitBL = bInitBL;
	m_bFromZero = bFromZero;

	m_bIsFinished = false;

	if(!XFoilInstance.InitXFoilGeometry(m_pFoil))  return false;
	if(!XFoilInstance.InitXFoilAnalysis(m_pPolar, bViscous)) return false;

	return true;
}

/** 
 * Sets the range of aoa or Cl parameters to analyze 
 * @param bAlpha true if the input parameter is a range of aoa, false if a range of lift coefficients
 * @param SpMin the minimum value of the range to analyze
 * @param SpMax the maximum value of the range to analyze
 * @param SpInc the increment value for the parameter
 */
void XFoilTask::setSequence(double bAlpha, double SpMin, double SpMax, double SpInc)
{
	m_bAlpha = bAlpha;
	if(bAlpha)
	{
		m_AlphaMin = SpMin;
		m_AlphaMax = SpMax;
		m_AlphaInc = SpInc;
	}
	else
	{
		m_ClMin = SpMin;
		m_ClMax = SpMax;
		m_ClInc = SpInc;
	}
}

/** 
 * Maps the range of Reynolds numbers to analyze to the member variables
 * @param ReMin the minimum value of the range to analyze
 * @param ReMax the maximum value of the range to analyze
 * @param ReInc the increment value for the Reynolds number
 */
void XFoilTask::setReRange(double ReMin, double ReMax, double ReInc)
{
	m_ReMin = ReMin;
	m_ReMax = ReMax;
	m_ReInc = ReInc;
}

/**
 * Maps the pointers to the graphs and curves for the output to the member variables.
 * If the pointers are NULL, no graphic output will be provided.
 * @param pGraph a pointer to the Graph object
 * @param pCurve0 a pointer to the rms curve
 * @param pCurve1 a pointer to the max curve
 */
void XFoilTask::setGraphPointers(Graph *pGraph, Curve *pCurve0, Curve* pCurve1)
{
	m_pGraph = pGraph;
	m_pCurve0 = pCurve0;
	m_pCurve1 = pCurve1;
}


/** 
* Performs a sequence of XFoil calculations for a range of aoa or lift coefficients
* @return true if the calculation was successful
*/
bool XFoilTask::AlphaSequence()
{
	QString str;

	double alphadeg;
	int ia, iSeries, total, MaxSeries;
	double SpMin, SpMax, SpInc;

	MaxSeries = 1;
	if(m_bAlpha)
	{
		SpMin = m_AlphaMin;
		SpMax = m_AlphaMax;
		SpInc = qAbs(m_AlphaInc);
		if (m_bFromZero && SpMin*SpMax<0)
		{
			MaxSeries = 2;
			SpMin = 0.0;
			SpMax = SpMax;
		}
	}
	else
	{
		SpMin = m_ClMin;
		SpMax = m_ClMax;
		SpInc = qAbs(m_ClInc);
	}

	if(SpMin > SpMax) SpInc = -qAbs(SpInc);


	for (iSeries=0; iSeries<MaxSeries; iSeries++)
	{
		if(s_bCancel) break;

		qApp->processEvents();

		total = (int)qAbs((SpMax*1.0001-SpMin)/SpInc);//*1.0001 to make sure upper limit is included


		if(m_bInitBL)
		{
			XFoilInstance.lblini = false;
			XFoilInstance.lipan = false;
		}

		for (ia=0; ia<=total; ia++)
		{
			if(s_bCancel) break;
			if(s_bSkipPolar)
			{

				XFoilInstance.lblini = false;
				XFoilInstance.lipan = false;
				s_bSkipPolar = false;
				traceLog("    .......skipping polar \n");
				return false;
			}

			if(m_bAlpha)
			{
				alphadeg = SpMin+ia*SpInc;

				XFoilInstance.alfa = alphadeg * PI/180.0;
				XFoilInstance.lalfa = true;
				XFoilInstance.qinf = 1.0;
				str = QString("Alpha = %1").arg(alphadeg,9,'f',3);
				traceLog(str);


				// here we go !
				if (!XFoilInstance.specal())
				{
					str = QObject::tr("Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ");
					traceLog(str);
					m_bErrors = true;
					return false;
				}
			}
			else
			{
				XFoilInstance.lalfa = false;
				XFoilInstance.alfa = 0.0;
				XFoilInstance.qinf = 1.0;
				XFoilInstance.clspec = SpMin+ia*SpInc;
				str = QString(QObject::tr("Cl = %1")).arg(XFoilInstance.clspec,9,'f',3);
				traceLog(str);
				if(!XFoilInstance.speccl())
				{
					str = QObject::tr("Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ");
					traceLog(str);
					m_bErrors = true;
					return false;
				}
			}

			XFoilInstance.lwake = false;
			XFoilInstance.lvconv = false;

			m_Iterations = 0;

			while(!Iterate()){}

			if(XFoilInstance.lvconv)
			{
				str = QString(QObject::tr("   ...converged after %1 iterations\n")).arg(m_Iterations);
				traceLog(str);
//				m_pPolar->AddXFoilData(&XFoilInstance);
			}
			else
			{
				str = QString(QObject::tr("   ...unconverged after %1 iterations\n")).arg(m_Iterations);
				traceLog(str);
				m_bErrors = true;
			}

			OpPoint::addOpPoint(m_pFoil, m_pPolar, &XFoilInstance);

			if(m_pGraph)
			{
				m_pGraph->ResetYLimits();
				if(m_pCurve0) m_pCurve0->clear();
				if(m_pCurve1) m_pCurve1->clear();

			}

		}// end Alpha or Cl loop
		SpMin = 0.0;
		SpMax = m_AlphaMin;
		SpInc = -SpInc;
	}
//		strong+="\n";
	return true;
}




/** 
* Performs a sequence of XFoil calculations for a range of Reynolds numbers.
* @return true if the calculation was successful
*/
bool XFoilTask::ReSequence()
{
	QString str;
	int ia;
	double Re;

	if(m_ReMax< m_ReMin) m_ReInc = -qAbs(m_ReInc);

	int total=int((m_ReMax*1.0001-m_ReMin)/m_ReInc);//*1.0001 to make sure upper limit is included

	total = abs(total);

	QString strange;

	for (ia=0; ia<=total; ia++)
	{
		if(s_bCancel) break;
		if(s_bSkipPolar)
		{
			XFoilInstance.lblini = false;
			XFoilInstance.lipan = false;
			s_bSkipPolar = false;
			traceLog("    .......skipping polar \n");
			return false;
		}

		qApp->processEvents();

		Re = m_ReMin+ia*m_ReInc;
		strange =QString("Re = %1 ........ ").arg(Re,0,'f',0);
		traceLog(strange);
		XFoilInstance.reinf1 = Re;
		XFoilInstance.lalfa = true;
		XFoilInstance.qinf = 1.0;

		// here we go !
		if (!XFoilInstance.specal())
		{
			QString str;
			str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ";
			traceLog(str);
			m_bErrors = true;
			return false;
		}

		XFoilInstance.lwake = false;
		XFoilInstance.lvconv = false;

		while(!Iterate()){}
		if(XFoilInstance.lvconv)
		{
			str = QString(QObject::tr("   ...converged after %1 iterations\n")).arg(m_Iterations);
			traceLog(str);
//			m_pPolar->AddXFoilData(&XFoilInstance);
		}
		else
		{
			str = QString(QObject::tr("   ...unconverged after %1 iterations\n")).arg(m_Iterations);
			traceLog(str);
			m_bErrors = true;
		}

		qApp->processEvents();

		m_Iterations = 0;

		OpPoint::addOpPoint(m_pFoil, m_pPolar, &XFoilInstance);

		if(m_pGraph)
		{
			m_pGraph->ResetYLimits();
			if(m_pCurve0) m_pCurve0->clear();
			if(m_pCurve1) m_pCurve1->clear();
		}
	}
	return true;
}



/**
* Manages the viscous iterations of the XFoil calculation.
* @return true if the analysis has been successful.
*/
bool XFoilTask::Iterate()
{
	if(!XFoilInstance.viscal())
	{
		XFoilInstance.lvconv = false;
//		QString str =QObject::tr("CpCalc: local speed too large\n Compressibility corrections invalid");
		return false;
	}

	while(m_Iterations<s_IterLim && !XFoilInstance.lvconv && !s_bCancel)
	{
		if(XFoilInstance.ViscousIter())
		{
			if(m_pCurve0) m_pCurve0->AppendPoint((double)m_Iterations, XFoilInstance.rmsbl);
			if(m_pCurve1) m_pCurve1->AppendPoint((double)m_Iterations, XFoilInstance.rmxbl);
			m_Iterations++;
		}
		else m_Iterations = s_IterLim;

		if(s_bSkipOpp || s_bSkipPolar)
		{
			XFoilInstance.lblini = false;
			XFoilInstance.lipan = false;
			s_bSkipOpp = false;
			return true;
		}
	}

	if(s_bCancel)  return true;// to exit loop


	if(!XFoilInstance.ViscalEnd())
	{
		XFoilInstance.lvconv = false;//point is unconverged

		XFoilInstance.lblini = false;
		XFoilInstance.lipan  = false;
		m_bErrors = true;
		return true;// to exit loop
	}

	if(m_Iterations>=s_IterLim && !XFoilInstance.lvconv)
	{
		if(s_bAutoInitBL)
		{
			XFoilInstance.lblini = false;
			XFoilInstance.lipan = false;
		}
		XFoilInstance.fcpmin();// Is it of any use ?
		return true;
	}
	if(!XFoilInstance.lvconv)
	{
		m_bErrors = true;
		XFoilInstance.fcpmin();// Is it of any use ?
		return false;
	}
	else
	{
		//converged at last
		XFoilInstance.fcpmin();// Is it of any use ?
		return true;
	}
	return false;
}



/** 
 * Sends the analysis messages to the specified text output stream 
 * @param str the message to output.
 */
void XFoilTask::traceLog(QString str)
{
	if(m_OutStream.device())
	{
		m_OutStream << str;
		m_OutMessage += str;
		qApp->processEvents();
	}
}

