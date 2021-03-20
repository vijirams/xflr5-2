/****************************************************************************

    XflEvents
    Copyright (C) 2021 Andre Deperrois

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

#pragma once

#include <QEvent>
#include <QString>

// Custom event identifier
const QEvent::Type MESH_UPDATE_EVENT         = static_cast<QEvent::Type>(QEvent::User + 100);
const QEvent::Type MESSAGE_EVENT             = static_cast<QEvent::Type>(QEvent::User + 101);
const QEvent::Type STREAMLINE_END_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 102);
const QEvent::Type XFOIL_END_TASK_EVENT      = static_cast<QEvent::Type>(QEvent::User + 103);
const QEvent::Type XFOIL_END_OPP_EVENT       = static_cast<QEvent::Type>(QEvent::User + 104);
const QEvent::Type PLANE_END_TASK_EVENT      = static_cast<QEvent::Type>(QEvent::User + 105);
const QEvent::Type PLANE_END_POPP_EVENT      = static_cast<QEvent::Type>(QEvent::User + 106);
const QEvent::Type VPW_UPDATE_EVENT          = static_cast<QEvent::Type>(QEvent::User + 107);
const QEvent::Type OPTIM_ITER_EVENT          = static_cast<QEvent::Type>(QEvent::User + 108);
const QEvent::Type OPTIM_END_EVENT           = static_cast<QEvent::Type>(QEvent::User + 109);

class Foil;
class Polar;
class OpPoint;


class MessageEvent : public QEvent
{
    public:
        MessageEvent(QString const &msg): QEvent(MESSAGE_EVENT)
        {
            m_Msg = msg;
        }

        QString const & msg() const {return m_Msg;}

    private:
        QString m_Msg;
};


// Custom event identifier

class Foil;
class Polar;
class OpPoint;

class XFoilTaskEvent : public QEvent
{

public:
    XFoilTaskEvent(Foil const*pFoil, Polar *pPolar): QEvent(XFOIL_END_TASK_EVENT),
        m_pFoil(pFoil),
        m_pPolar(pPolar)
    {
    }

    Foil const*foilPtr() const  {return m_pFoil;}
    Polar * polarPtr() const    {return m_pPolar;}

private:
    Foil const*m_pFoil=nullptr;
    Polar *m_pPolar=nullptr;
};


class XFoilOppEvent : public QEvent
{

public:
    XFoilOppEvent(Foil const*pFoil, Polar *pPolar, OpPoint *pOpPoint): QEvent(XFOIL_END_OPP_EVENT),
        m_pFoil(pFoil),
        m_pPolar(pPolar)
    {
//        memcpy(&m_XFoil, pXFoilPtr, sizeof(XFoil));
        m_pOpPoint = pOpPoint; /** use the copy constructor and = operator defined implicitly by the compiler */
    }

    ~XFoilOppEvent()
    {
    }

    Foil const* foilPtr()   const {return m_pFoil;}
    Polar * polarPtr() const {return m_pPolar;}
    OpPoint * oppPtr() const {return m_pOpPoint;}

private:
    Foil const*m_pFoil=nullptr;
    Polar *m_pPolar=nullptr;
    OpPoint *m_pOpPoint=nullptr;    /** need to store current XFoil results */
};




