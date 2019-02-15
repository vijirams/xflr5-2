/****************************************************************************

	Objects2D    Copyright (C) 2016-2016 Andre Deperrois 

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



#include "objects2d.h"
#include <xdirect/XDirect.h>
#include <globals/globals.h>
#include <misc/options/displayoptions.h>
#include <QtDebug>

#define PI 3.141592654

QList<Foil*>    Objects2d::s_oaFoil;
QList<Polar*>   Objects2d::s_oaPolar;
QList<OpPoint*> Objects2d::s_oaOpp;



Objects2d::Objects2d()
{

}



/**
 * Deletes the Foil object removes it from the array, and returns a pointer to the next Foil in the array
 * @param pFoil a pointer to the Foil to delete
 * @return a pointer to the next or previous Foil in the array, or NULL if none
 */
Foil * Objects2d::deleteFoil(Foil *pFoil)
{
	if(!pFoil || !pFoil->foilName().length()) return nullptr;
	Foil *pOldFoil;
	OpPoint * pOpPoint;
	Polar* pPolar;
	int j;


	for (j=s_oaOpp.size()-1; j>=0; j--)
	{
		pOpPoint = s_oaOpp[j];
		if(pOpPoint->foilName() == pFoil->foilName())
		{
			if(pOpPoint==XDirect::curOpp()) XDirect::setCurOpp(NULL);
			s_oaOpp.removeAt(j);
			delete pOpPoint;
		}
	}


	for (j=s_oaPolar.size()-1; j>=0; j--)
	{
        pPolar = s_oaPolar.at(j);
		if(pPolar->foilName() == pFoil->foilName())
		{
			if(pPolar==XDirect::curPolar()) XDirect::setCurPolar(NULL);
			s_oaPolar.removeAt(j);
			delete pPolar;
		}
	}


	Foil *pNewCurFoil= nullptr;

	for (j=0; j<s_oaFoil.size(); j++)
	{
        pOldFoil = s_oaFoil.at(j);
		if (pOldFoil == pFoil)
		{
            if(j<s_oaFoil.count()-1) pNewCurFoil = s_oaFoil.at(j+1);
            else if(j>0)             pNewCurFoil = s_oaFoil.at(j-1);
			else                     pNewCurFoil = nullptr;

			s_oaFoil.removeAt(j);
			delete pOldFoil;
			break;
		}
	}
	return pNewCurFoil;
}


/**
 * Returns a pointer to the foil with the corresponding nam or NULL if not found.
 * @param strFoilName the name of the Foil to search for in the array
 * @return a pointer to the foil with the corresponding nam or NULL if not found.
 */
Foil* Objects2d::foil(QString strFoilName)
{
	if(!strFoilName.length()) return nullptr;
	Foil* pFoil;
	for (int i=0; i<s_oaFoil.size(); i++)
	{
        pFoil = s_oaFoil.at(i);
		if (pFoil->foilName() == strFoilName)
		{
			return pFoil;
		}
	}

	return nullptr;
}

/**
 * The Foil does not exist yet in the array.
 * Delete any former Foil with the same name, including its children objects.
 * Insert the Foil in sorted order in the array.
 * Questions have been answered previously : which-name, overwrite-or-not-overwrite, etc. Just do it.
*/
void Objects2d::insertThisFoil(Foil *pFoil)
{
	Foil*pOldFoil;
	QString oldFoilName = pFoil->foilName();

	//check that this Foil does not exist in the array
	for(int iFoil=0; iFoil<s_oaFoil.count(); iFoil++)
	{
		if(pFoil == s_oaFoil.at(iFoil))
		{
//			Trace("This foil "+m_FoilName+" aready exists and has not been inserted");
			return;
		}
	}

	//check if it's an overwrite
	for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
	{
		pOldFoil = s_oaFoil.at(iFoil);
		if(pOldFoil->foilName()==oldFoilName && pOldFoil!=pFoil)
		{
			//copy the old foil's style
			pFoil->m_red          = pOldFoil->m_red;
			pFoil->m_green        = pOldFoil->m_green;
			pFoil->m_blue         = pOldFoil->m_blue;
			pFoil->m_alphaChannel = pOldFoil->m_alphaChannel;
			pFoil->m_FoilStyle = pOldFoil->foilLineStyle();
			pFoil->m_FoilWidth = pOldFoil->foilLineWidth();

			//we overwrite the old foil and delete its children objects
			deleteFoil(pOldFoil);
			//continue loop, shouldn't find any other Foil with the same name, but cleans up former errors eventually
		}
	}

	// no existing former foil with the same name, straightforward insert
	for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
	{
        pOldFoil = s_oaFoil.at(iFoil);
		if(pFoil->foilName().compare(pOldFoil->foilName(), Qt::CaseInsensitive)<0)
		{
			//then insert before
			s_oaFoil.insert(iFoil, pFoil);
			return;
		}
	}

	//not inserted, append
	s_oaFoil.append(pFoil);
}




/**
* Creates a new instance of an OpPoint.
* Loads the data from the XFoil object in that OpPoint
* Adds the data to the active Polar object
* @param pPolarPtr a point to the parent Polar object to which the OpPoint should be attached.
* @return a pointer to the OpPoint which has been created, or NULL if it wasn't stored.
*/
OpPoint* Objects2d::addOpPoint(Foil *pFoil, Polar *pPolar, XFoil *pXFoil, bool bStoreOpp)
{
    if(!pFoil || !pXFoil) return nullptr;

    if(!pPolar) pPolar = XDirect::curPolar();

	OpPoint *pNewPoint = new OpPoint();
	QColor clr = randomColor(!Settings::isLightTheme());
	pNewPoint->setColor(clr.red(), clr.green(), clr.black(), clr.alpha());

    if(pNewPoint ==nullptr)
	{
		return nullptr;
	}
	else
	{
		if(!pXFoil->lvconv)
		{
			delete pNewPoint;
			return nullptr;
		}
		pNewPoint->m_Alpha = pXFoil->alfa * 180/PI;
		pNewPoint->foilName() = pFoil->foilName();
		pNewPoint->polarName()  = pPolar->polarName();

		addXFoilData(pNewPoint, pXFoil, pFoil);
	}

    if(bStoreOpp)
	{
		//insert the OpPoint in the Operating points array
		Objects2d::insertOpPoint(pNewPoint);
	}

	// Now insert OpPoint in the current Polar object

	if(pXFoil->lvconv && pPolar)
	{
		if(pPolar->polarType()==XFLR5::FIXEDLIFTPOLAR || pPolar->polarType()==XFLR5::RUBBERCHORDPOLAR)
		{
			if(pNewPoint && pNewPoint->Reynolds()<1.00e8)
			{
				pPolar->addOpPointData(pNewPoint);
			}
		}
		else
		{
			pPolar->addOpPointData(pNewPoint);
		}
	}

	if(!bStoreOpp)
	{
		delete pNewPoint;
		pNewPoint = nullptr;
	}

	return pNewPoint;
}




/**
 * Deletes the Foil and its children Polar and OpPoint objects
 * @return a pointer to the next or previous Foil in the array
 */
Foil * Objects2d::deleteThisFoil(Foil *pFoil)
{
	Foil *pOldFoil=nullptr;
	Polar* pOldPolar=nullptr;
	OpPoint *pOpPoint=nullptr;

	//delete any OpPoints with this FoilName
	for (int jOpp=s_oaOpp.size()-1; jOpp>=0; jOpp--)
	{
		pOpPoint = s_oaOpp[jOpp];
		if(pOpPoint->foilName() == pFoil->foilName())
		{
			if(pOpPoint==XDirect::curOpp()) XDirect::setCurOpp(NULL);
			s_oaOpp.removeAt(jOpp);
			delete pOpPoint;
		}
	}


	//delete any Polars  with this FoilName
	for (int jPlr=s_oaPolar.size()-1; jPlr>=0; jPlr--)
	{
        pOldPolar = s_oaPolar.at(jPlr);
		if(pOldPolar->foilName() == pFoil->foilName())
		{
			if(pOldPolar==XDirect::curPolar()) XDirect::setCurPolar(NULL);
			s_oaPolar.removeAt(jPlr);
			delete pOldPolar;
		}
	}


	//delete the Foil
	Foil *pNewCurFoil= nullptr;

	for (int jFoil=0; jFoil<s_oaFoil.size(); jFoil++)
	{
		pOldFoil = s_oaFoil.at(jFoil);
		if (pOldFoil == pFoil)
		{
            if(jFoil<s_oaFoil.count()-1) pNewCurFoil = s_oaFoil.at(jFoil+1);
            else if(jFoil>0)             pNewCurFoil = s_oaFoil.at(jFoil-1);
			else                         pNewCurFoil = nullptr;

			s_oaFoil.removeAt(jFoil);
			delete pOldFoil;
			break;
		}
	}
	return pNewCurFoil;
}




bool Objects2d::deleteOpp(OpPoint *pOpp)
{
	OpPoint* pOldOpp;

	if(!pOpp) return false;
	if(pOpp == XDirect::curOpp()) XDirect::setCurOpp(NULL);

	for (int iOpp=0; iOpp<s_oaOpp.size(); iOpp++)
	{
        pOldOpp = s_oaOpp.at(iOpp);
		if (pOpp == pOldOpp)
		{
			s_oaOpp.removeAt(iOpp);
			delete pOpp;
			return true;
		}
	}
	return false;
}



void Objects2d::deletePolar(Polar *pPolar)
{
	Polar* pOldPolar;

	if(!pPolar) return;
	if(pPolar == XDirect::curPolar()) XDirect::setCurPolar(NULL);

	for (int iPolar=0; iPolar<s_oaPolar.size(); iPolar++)
	{
        pOldPolar =s_oaPolar.at(iPolar);
		if (pPolar == pOldPolar)
		{
			s_oaPolar.removeAt(iPolar);
			delete pOldPolar;
			break;
		}
	}
}





/**
 * The foil exists in the array.
 * Rename it with the specified new name, and rename its children Polar and OpPoint objects
 * Overwrites any former foil with the same name.
 * Questions have been answered previously : which-name, overwrite-or-not-overwrite, etc. Just do it.
 * @param newFoilName
 */
void Objects2d::renameThisFoil(Foil *pFoil, QString newFoilName)
{
	Foil *pOldFoil=nullptr;
	Polar* pOldPolar=nullptr;
	OpPoint *pOpPoint=nullptr;
	QString oldFoilName = pFoil->foilName();

	//check that this Foil exists in the array
	bool bFound = false;
	for(int iFoil=0; iFoil<s_oaFoil.count(); iFoil++)
	{
		if(pFoil == s_oaFoil.at(iFoil))
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
//		Trace("The foil "+m_FoilName+" could not be found in the array and has not been renamed");
		return;
	}

	//rename it
	pFoil->setFoilName(newFoilName);

	//delete any former Foil with the new name
	for(int iFoil=0; iFoil<s_oaFoil.count(); iFoil++)
	{
        pOldFoil = s_oaFoil.at(iFoil);
		if(pOldFoil->foilName() == oldFoilName)
		{
			deleteThisFoil(pOldFoil);
			//continue loop to purge old duplicates, who knows.
		}
	}

	//ready to rename

	//rename its children objects
	for (int iPolar=0; iPolar<s_oaPolar.size(); iPolar++)
	{
        pOldPolar = s_oaPolar.at(iPolar);
		if(pOldPolar->foilName() == oldFoilName)
		{
			pOldPolar->setFoilName(newFoilName);
		}
	}

	for (int iOpp=0; iOpp<s_oaOpp.size(); iOpp++)
	{
		pOpPoint = s_oaOpp.at(iOpp);
		if(pOpPoint->foilName() == oldFoilName)
		{
			pOpPoint->setFoilName(newFoilName);
		}
	}


	//remove the Foil from its current position in the array
	for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
	{
        if(pFoil == s_oaFoil.at(iFoil))
		{
			s_oaFoil.removeAt(iFoil);
			break;
		}
	}


	//re-insert and we're done
	for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
	{
        pOldFoil = s_oaFoil.at(iFoil);
		if(pFoil->foilName().compare(pOldFoil->foilName(), Qt::CaseInsensitive)<0)
		{
			//then insert before
			s_oaFoil.insert(iFoil, pFoil);
			return;
		}
	}

	//Not inserted, append
	s_oaFoil.append(pFoil);
}




OpPoint *Objects2d::getOpp(Foil *pFoil, Polar *pPolar, double Alpha)
{
	OpPoint* pOpPoint;
	if(!pPolar) return nullptr;

	for (int i=0; i<s_oaOpp.size(); i++)
	{
		if(!pPolar) return nullptr;
        pOpPoint = s_oaOpp.at(i);
		//since alphas are calculated at 1/100th
		if (pOpPoint->foilName() == pFoil->foilName())
		{
			if (pOpPoint->polarName() == pPolar->polarName())
			{
				if(pPolar->polarType() != XFLR5::FIXEDAOAPOLAR)
				{
					if(qAbs(pOpPoint->aoa() - Alpha) <0.001)
					{
						return pOpPoint;
					}
				}
				else
				{
					if(qAbs(pOpPoint->Reynolds() - Alpha) <0.1)
					{
						return pOpPoint;
					}
				}
			}
		}
	}
	return nullptr;// if no OpPoint has a matching alpha
}


/**
 * Inserts a new OpPoint in the array. The OpPoints are sorted by FoilName first, then by Re number, then by aoa.
 * If an OpPoint already exists with the same combination of (FoilName, Re, aoa), it is overwritten.
 * @param pNewPoint
 */
void Objects2d::insertOpPoint(OpPoint *pNewPoint)
{
	if(!pNewPoint) return;

	OpPoint* pOpPoint;
	Polar *pPolar = getPolar(pNewPoint->foilName(), pNewPoint->polarName());

	if(!pPolar)
	{
		delete pNewPoint;
		return;
	}

	// first add the OpPoint to the OpPoint Array for the current FoilName
	for (int i=0; i<s_oaOpp.size(); i++)
	{
        pOpPoint = s_oaOpp.at(i);
		if (pNewPoint->foilName().compare(pOpPoint->foilName())<0)
		{
			//insert point
			s_oaOpp.insert(i, pNewPoint);
			return;
		}
		else if (pNewPoint->foilName() == pOpPoint->foilName())
		{
			if (pNewPoint->Reynolds() < pOpPoint->Reynolds())
			{
				//insert point
				s_oaOpp.insert(i, pNewPoint);
				return;
			}
			else if (fabs(pNewPoint->Reynolds()-pOpPoint->Reynolds())<1.0)
			{
				if(fabs(pNewPoint->aoa() - pOpPoint->aoa())<0.005 &&
				   fabs(pNewPoint->ACrit-pOpPoint->ACrit)<0.1 &&
				   fabs(pNewPoint->Xtr1-pOpPoint->Xtr1)<0.001 &&
				   fabs(pNewPoint->Xtr2-pOpPoint->Xtr2)<0.001)
				{

					//replace existing point
                    XDirect::setCurOpp(NULL);
                    s_oaOpp.removeAt(i);
                    delete pOpPoint;
                    s_oaOpp.insert(i, pNewPoint);
					return;
				}
				else if (pNewPoint->m_Alpha > pOpPoint->aoa())
				{
					//insert point
					s_oaOpp.insert(i, pNewPoint);
					return;
				}
			}
		}
	}

	s_oaOpp.append(pNewPoint);
}


/**
 * Inserts a polar in the array, using the foil name, the polar type, the Re number and the a.o.a. as sorting keys.
 * If a Polar with identical foilname and polar name exists, deletes the old and replaces it.
 * Note : sorting of Type 1 polars is necessary for the interpolation process in Plane analysis.
 * @param pPolar a pointer to the Polar object to insert in the array;
 */
void Objects2d::addPolar(Polar *pPolar)
{
	if(!pPolar) return;

	bool bExists   = false;
	bool bInserted = false;
	Polar *pOldPlr = nullptr;
	int ip,j;

	for (ip=0; ip<s_oaPolar.size(); ip++)
	{
        pOldPlr = s_oaPolar.at(ip);
		if (pOldPlr->polarName().compare(pPolar->polarName())==0 &&
			pOldPlr->foilName().compare(pPolar->foilName())==0)
		{
			bExists = true;
			s_oaPolar.removeAt(ip);
			delete pOldPlr;
			s_oaPolar.insert(ip, pPolar);
			break;
		}
	}

	if(!bExists)
	{
		for (j=0; j<s_oaPolar.size(); j++)
		{
            pOldPlr = s_oaPolar.at(j);

			//first index is the parent foil name
			if (pPolar->foilName().compare(pOldPlr->foilName(), Qt::CaseInsensitive)<0)
			{
				s_oaPolar.insert(j, pPolar);
				bInserted = true;
				break;
			}
			else if (pPolar->foilName() == pOldPlr->foilName())
			{
				//second index is the polar type
				if(pPolar->polarType() < pOldPlr->polarType())
				{
					s_oaPolar.insert(j, pPolar);
					bInserted = true;
					break;
				}
				else if(pPolar->polarType() == pOldPlr->polarType())
				{
					if (pPolar->polarType() != XFLR5::FIXEDAOAPOLAR)
					{
						//sort by re Nbr
						if(pPolar->Reynolds() < pOldPlr->Reynolds())
						{
							s_oaPolar.insert(j, pPolar);
							bInserted = true;
							break;
						}
					}
					else
					{
						//Type 4, sort by Alphas
						if(pPolar->m_ASpec < pOldPlr->m_ASpec)
						{
							s_oaPolar.insert(j, pPolar);
							bInserted = true;
							break;
						}
					}
				}
			}
		}
		if(!bInserted)
		{
			s_oaPolar.append(pPolar);
			bInserted = true;
		}
	}
}




Polar *Objects2d::getPolar(Foil *pFoil, QString PolarName)
{
	if (!PolarName.length()) return nullptr;


	Polar *pPolar;
	for (int i=0; i<s_oaPolar.size(); i++)
	{
        pPolar =  s_oaPolar.at(i);
		if (pPolar->foilName() == pFoil->foilName() &&  pPolar->polarName() == PolarName)
		{
			return pPolar;
		}
	}
	return nullptr;
}


Polar *Objects2d::getPolar(QString m_FoilName, QString PolarName)
{
	if (!PolarName.length())
	{
		return nullptr;
	}

	Polar *pPolar;
	for (int i=0; i<s_oaPolar.size(); i++)
	{
        pPolar =  s_oaPolar.at(i);
		if (pPolar->foilName() == m_FoilName &&  pPolar->polarName() == PolarName)
		{
			return pPolar;
		}
	}
	return nullptr;
}





/**
 * Creates a polar object for a given set of specified input data
 * @param pFoil a pointer to the Foil object to which the Polar will be attached
 * @param Re  the value of the Reynolds number
 * @param Mach  the value of the Mach number
 * @param NCrit the value of the transition criterion
 * @return a pointer to the Polar object which has been created
 */
Polar * Objects2d::createPolar(Foil *pFoil, double Re, double Mach, double NCrit, double XtrTop, double XtrBot, XFLR5::enumPolarType polarType)
{
	if(!pFoil) return nullptr;

	Polar *pNewPolar = new Polar;
    pNewPolar->setFoilName(pFoil->foilName());
    pNewPolar->setVisible(true);
	pNewPolar->polarType() = polarType;
	pNewPolar->Mach()      = Mach;
	pNewPolar->NCrit()     = NCrit;
	pNewPolar->XtrTop()    = XtrTop;
	pNewPolar->XtrBot()    = XtrBot;

	switch (pNewPolar->polarType())
	{
	case XFLR5::FIXEDSPEEDPOLAR:
		pNewPolar->MaType() = 1;
		pNewPolar->ReType() = 1;
		break;
	case XFLR5::FIXEDLIFTPOLAR:
		pNewPolar->MaType() = 2;
		pNewPolar->ReType() = 2;
		break;
	case XFLR5::RUBBERCHORDPOLAR:
		pNewPolar->MaType() = 1;
		pNewPolar->ReType() = 3;
		break;
	case XFLR5::FIXEDAOAPOLAR:
		pNewPolar->MaType() = 1;
		pNewPolar->ReType() = 1;
		break;
	default:
		pNewPolar->ReType() = 1;
		pNewPolar->MaType() = 1;
		break;
	}
	if(polarType!=XFLR5::FIXEDAOAPOLAR)  pNewPolar->Reynolds() = Re;
	else                                 pNewPolar->aoa()    = 0.0;


    pNewPolar->setPolarName(Polar::autoPolarName(polarType, Re, Mach, NCrit, 0.0, XtrTop, XtrBot));
	return pNewPolar;
}




/**
* Adds the results of the XFoil Calculation to the OpPoint object
* @param pOpPoint a pointer to the instance of the OpPoint to be filled with the data from the XFoil object.
*/
void Objects2d::addXFoilData(OpPoint *pOpp, XFoil *pXFoil, Foil *pFoil)
{
    int i=0, j=0, ibl=0, is=0;

	pOpp->n            = pXFoil->n;
	pOpp->Cd           = pXFoil->cd;
	pOpp->Cdp          = pXFoil->cdp;
	pOpp->Cl           = pXFoil->cl;
	pOpp->m_XCP        = pXFoil->xcp;
	pOpp->Cm           = pXFoil->cm;
	pOpp->m_Reynolds   = pXFoil->reinf;
	pOpp->m_Mach       = pXFoil->minf;
	pOpp->ACrit        = pXFoil->acrit;

	pOpp->m_bTEFlap    = pFoil->m_bTEFlap;
	pOpp->m_bLEFlap    = pFoil->m_bLEFlap;

	pOpp->Cpmn   = pXFoil->cpmn;

    for (int k=0; k<pXFoil->n; k++)
	{
//		x[k]   = m_pXFoil->x[k+1];
//		y[k]   = m_pXFoil->y[k+1];
//		s[k]   = m_pXFoil->s[k+1];
		pOpp->Cpi[k] = pXFoil->cpi[k+1];
		pOpp->Qi[k]  = pXFoil->qgamm[k+1];
	}

	if(pXFoil->lvisc && pXFoil->lvconv)
	{
		pOpp->Xtr1 =pXFoil->xoctr[1];
		pOpp->Xtr2 =pXFoil->xoctr[2];
		pOpp->m_bViscResults = true;
		pOpp->m_bBL = true;
        for (int k=0; k<pXFoil->n; k++)
		{
			pOpp->Cpv[k] = pXFoil->cpv[k+1];
			pOpp->Qv[k] = pXFoil->qvis[k+1];
		}
	}
	else
	{
		pOpp->m_bViscResults = false;
	}

	if(pOpp->m_bTEFlap || pOpp->m_bLEFlap)
	{
		pOpp->setHingeMoments(pFoil);
/*		m_TEHMom = m_pXFoil->hmom;
		XForce   = m_pXFoil->hfx;
		YForce   = m_pXFoil->hfy;*/
	}

	if(!pXFoil->lvisc || !pXFoil->lvconv)	return;

//---- add boundary layer on both sides of airfoil
	pOpp->nd1=0;
	pOpp->nd2=0;
	pOpp->nd3=0;
	for (is=1; is<=2; is++)
	{
        for (ibl=2; ibl<=pXFoil->iblte[is];ibl++)
		{
			i = pXFoil->ipan[ibl][is];
			pOpp->xd1[i] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][is];
			pOpp->yd1[i] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][is];
			pOpp->nd1++;
		}
	}

//---- set upper and lower wake dstar fractions based on first wake point
	is=2;
	double dstrte = pXFoil->dstr[pXFoil->iblte[is]+1][is];
	double dsf1, dsf2;
    if(dstrte!=0.0) // d* at TE
	{
		dsf1 = (pXFoil->dstr[pXFoil->iblte[1]][1] + 0.5*pXFoil->ante) / dstrte;
		dsf2 = (pXFoil->dstr[pXFoil->iblte[2]][2] + 0.5*pXFoil->ante) / dstrte;
	}
	else
	{
		dsf1 = 0.5;
		dsf2 = 0.5;
	}

//---- plot upper wake displacement surface
	ibl = pXFoil->iblte[1];
	i = pXFoil->ipan[ibl][1];
	pOpp->xd2[0] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][1];
	pOpp->yd2[0] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][1];
	pOpp->nd2++;

	j= pXFoil->ipan[pXFoil->iblte[is]+1][is]  -1;
	for (ibl=pXFoil->iblte[is]+1; ibl<=pXFoil->nbl[is]; ibl++)
	{
		i = pXFoil->ipan[ibl][is];
		pOpp->xd2[i-j] = pXFoil->x[i] - pXFoil->nx[i]*pXFoil->dstr[ibl][is]*dsf1;
		pOpp->yd2[i-j] = pXFoil->y[i] - pXFoil->ny[i]*pXFoil->dstr[ibl][is]*dsf1;
		pOpp->nd2++;
	}

//---- plot lower wake displacement surface
	ibl = pXFoil->iblte[2];
	i = pXFoil->ipan[ibl][2];
	pOpp->xd3[0] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][2];
	pOpp->yd3[0] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][2];
	pOpp->nd3++;

	j = pXFoil->ipan[pXFoil->iblte[is]+1][is]  -1;
	for (ibl=pXFoil->iblte[is]+1; ibl<=pXFoil->nbl[is]; ibl++)
	{
		i = pXFoil->ipan[ibl][is];
		pOpp->xd3[i-j] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][is]*dsf2;
		pOpp->yd3[i-j] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][is]*dsf2;
		pOpp->nd3++;
	}

	pOpp->tklam = pXFoil->tklam;
	pOpp->qinf = pXFoil->qinf;

	memcpy(pOpp->thet, pXFoil->thet, IVX * ISX * sizeof(double));
	memcpy(pOpp->tau,  pXFoil->tau,  IVX * ISX * sizeof(double));
	memcpy(pOpp->ctau, pXFoil->ctau, IVX * ISX * sizeof(double));
	memcpy(pOpp->ctq,  pXFoil->ctq,  IVX * ISX * sizeof(double));
	memcpy(pOpp->dis,  pXFoil->dis,  IVX * ISX * sizeof(double));
	memcpy(pOpp->uedg, pXFoil->uedg, IVX * ISX * sizeof(double));
	memcpy(pOpp->dstr, pXFoil->dstr, IVX * ISX * sizeof(double));
	memcpy(pOpp->itran, pXFoil->itran, 3 * sizeof(int));

    pXFoil->createXBL();
    pXFoil->fillHk();
    pXFoil->fillRTheta();
	memcpy(pOpp->xbl, pXFoil->xbl, IVX * ISX * sizeof(double));
	memcpy(pOpp->Hk, pXFoil->Hk, IVX * ISX * sizeof(double));
	memcpy(pOpp->RTheta, pXFoil->RTheta, IVX * ISX * sizeof(double));
	pOpp->nside1 = pXFoil->m_nSide1;
	pOpp->nside2 = pXFoil->m_nSide2;
}


void Objects2d::deleteFoilResults(Foil *pFoil, bool bDeletePolars)
{
	for (int j=s_oaOpp.size()-1; j>=0; j--)
	{
		OpPoint *pOpPoint = s_oaOpp[j];
		if(pOpPoint->foilName() == pFoil->foilName())
		{
			if(pOpPoint==XDirect::curOpp()) XDirect::setCurOpp(NULL);
			s_oaOpp.removeAt(j);
			delete pOpPoint;
		}
	}

	for (int j=s_oaPolar.size()-1; j>=0; j--)
	{
        Polar *pPolar = s_oaPolar.at(j);
		if(pPolar->foilName() == pFoil->foilName())
		{
			if(bDeletePolars)
			{
				if(pPolar==XDirect::curPolar()) XDirect::setCurPolar(NULL);
				s_oaPolar.removeAt(j);
				delete pPolar;
			}
			else
			{
				pPolar->resetPolar();
			}
		}
	}
}












