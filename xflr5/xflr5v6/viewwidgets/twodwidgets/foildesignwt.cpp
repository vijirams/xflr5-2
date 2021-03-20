/****************************************************************************

    FoilDesignWt Class
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

#include <QPainter>
#include <QMessageBox>


#include "foildesignwt.h"
#include <xflcore/xflcore.h>
#include <globals/mainframe.h>
#include <graph/graph_globals.h>
#include <gui_objects/splinefoil.h>
#include <misc/options/settings.h>
#include <xdirect/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflobjects/objects2d/foil.h>

FoilDesignWt::FoilDesignWt(QWidget *pParent) : Section2dWidget(pParent)
{
    m_bNeutralLine = true;
    m_bLECircle      = false;
    m_LERad   = 1.0;

    m_pSF = nullptr;

    m_pBufferFoil = nullptr;
    createContextMenu();
}


void FoilDesignWt::setObjects(Foil *pBufferFoil, SplineFoil *pSF)
{
    m_pBufferFoil = pBufferFoil;
    m_pSF = pSF;
}


void FoilDesignWt::setScale()
{
    //scale is set by user zooming
    m_fRefScale = rect().width()*6.0/8.0;
    m_fScale = m_fRefScale;


    m_ptOffset.rx() = rect().width()/8;
    m_ptOffset.ry() = rect().height()/2;

    m_ViewportTrans = QPoint(0,0);
}


void FoilDesignWt::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Settings::s_BackgroundColor);

    drawScaleLegend(painter);
    drawBackImage(painter);


    paintGrids(painter);
    paintLECircle(painter);
    paintSplines(painter);
    paintFoils(painter);
    paintLegend(painter);
}


/**
 * Draws a dashed circle located at the leading edge of the foil.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilDesignWt::paintLECircle(QPainter &painter)
{
    if(m_bLECircle)
    {
        int rx = int(m_LERad/100.0 * m_fScale);
        int ry = int(m_LERad/100.0 * m_fScale * m_fScaleY);
        QRectF rc(m_ptOffset.x(), m_ptOffset.y() - ry,  2*rx, 2*ry);

        QPen CirclePen(QColor(128,128,128));
        CirclePen.setStyle(Qt::DashLine);
        painter.setPen(CirclePen);
        painter.drawEllipse(rc);
    }
}


/**
 * Draws the SplineFoil object.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilDesignWt::paintSplines(QPainter &painter)
{
    painter.save();

    QPen CtrlPen;

    QBrush FillBrush(Settings::s_BackgroundColor);
    painter.setBrush(FillBrush);

    if(m_pSF->isVisible())
    {
        m_pSF->drawFoil(painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);

        CtrlPen.setStyle(Qt::SolidLine);
        CtrlPen.setColor(m_pSF->splineFoilColor());
        painter.setPen(CtrlPen);

        m_pSF->drawCtrlPoints(painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset);

        if (m_pSF->showCenterLine())
        {
            m_pSF->drawMidLine(painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset);
        }
        if (m_pSF->showOutPoints())
        {
            m_pSF->drawOutPoints(painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset);
        }
    }

    painter.restore();
}


/**
 * Draws the visible Foil objects.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilDesignWt::paintFoils(QPainter &painter)
{
    painter.save();
    QPen FoilPen, CenterPen, CtrlPen;

    QBrush FillBrush(Settings::s_BackgroundColor);
    painter.setBrush(FillBrush);

    for (int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil const*pFoil = Objects2d::foilAt(k);
        if (pFoil->isVisible())
        {
            FoilPen.setStyle(getStyle(pFoil->foilLineStyle()));
            FoilPen.setWidth(pFoil->foilLineWidth());
            FoilPen.setColor(colour(pFoil));
            painter.setPen(FoilPen);

            drawFoil(painter, pFoil, 0.0, m_fScale, m_fScale*m_fScaleY,m_ptOffset);
            if (pFoil->bCenterLine())
            {
                CenterPen.setColor(colour(pFoil));
                CenterPen.setStyle(Qt::DashLine);
                painter.setPen(CenterPen);
                drawMidLine(painter, pFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
            }

            drawPoints(painter, pFoil, 0.0, m_fScale,m_fScale*m_fScaleY, m_ptOffset, Settings::s_BackgroundColor);

        }
    }
    if (m_pBufferFoil->isVisible())
    {
        drawFoil(painter, m_pBufferFoil, 0.0, m_fScale, m_fScale*m_fScaleY,m_ptOffset);

        if (m_pBufferFoil->bCenterLine())
        {
            CenterPen.setColor(colour(m_pBufferFoil));
            CenterPen.setStyle(Qt::DashLine);
            painter.setPen(CenterPen);
            drawMidLine(painter, m_pBufferFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
        }

        CtrlPen.setColor(colour(m_pBufferFoil));
        painter.setPen(CtrlPen);
        drawPoints(painter, m_pBufferFoil, 0.0, m_fScale,m_fScale*m_fScaleY, m_ptOffset, Settings::s_BackgroundColor);

    }
    painter.restore();
}



/**
 * Draws the legend.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilDesignWt::paintLegend(QPainter &painter)
{
    painter.save();

    painter.setFont(Settings::s_TextFont);

    if(m_bShowLegend)
    {
        QFont fnt(Settings::s_TextFont); //valgrind
        QFontMetrics fm(fnt);
        int fmw = fm.averageCharWidth();

        Foil const* pRefFoil;
        QString strong;
        QPoint Place(rect().right()-35*fmw, 10);

        int LegendSize = 10*fmw;
        int ypos = 15;
        int delta = 5;

        painter.setBackgroundMode(Qt::TransparentMode);

        QPen TextPen(Settings::s_TextColor);
        painter.setPen(TextPen);

        QBrush FillBrush(Settings::s_BackgroundColor);
        painter.setBrush(FillBrush);

        QPen LegendPen;

        int k=0;

        if(m_pSF && m_pSF->isVisible())
        {
            LegendPen.setColor(m_pSF->splineFoilColor());
            LegendPen.setStyle(getStyle(m_pSF->splineFoilStyle()));
            LegendPen.setWidth(m_pSF->splineFoilWidth());

            painter.setPen(LegendPen);
            painter.drawLine(Place.x(), Place.y() + ypos*k, Place.x() + LegendSize, Place.y() + ypos*k);
            if(m_pSF->showOutPoints())
            {
//                    x1 = Place.x + (int)(0.5*LegendSize);
//                    pDC->Rectangle(x1-2, Place.y + ypos*k-2, x1+2, Place.y + ypos*k+2);
                int x1 = Place.x() + int(0.5*LegendSize);
                painter.drawRect(x1-2, Place.y() + ypos*k-2, 4,4);
            }
            painter.setPen(TextPen);
            painter.drawText(Place.x() + LegendSize + fmw, Place.y() + ypos*k+delta, m_pSF->splineFoilName());
        }

        k++;


        for (int n=0; n <Objects2d::foilCount(); n++)
        {
            pRefFoil = Objects2d::foilAt(n);
            if(pRefFoil && pRefFoil->isVisible())
            {
                strong = pRefFoil->name();
                if(strong.length())
                {
                    LegendPen.setColor(colour(pRefFoil));
                    LegendPen.setStyle(getStyle(pRefFoil->foilLineStyle()));
                    LegendPen.setWidth(pRefFoil->foilLineWidth());

                    painter.setPen(LegendPen);
                    painter.drawLine(Place.x(), Place.y() + ypos*k, Place.x() + LegendSize, Place.y() + ypos*k);

                    int x1 = Place.x() + int(0.5*LegendSize);
/*                        if(pRefFoil->showPoints())
                    {
                        painter.drawRect(x1-2, Place.y() + ypos*k-2, 4,4);
                    }*/
                    drawPoint(painter, pRefFoil->foilPointStyle(), Settings::s_BackgroundColor, QPoint(x1, Place.y() + ypos*k));
                    painter.setPen(TextPen);
                    painter.drawText(Place.x() + LegendSize + fmw, Place.y() + ypos*k+delta, pRefFoil->name());
                    k++;
                }
            }
        }
    }
    painter.restore();
}


void FoilDesignWt::resizeEvent (QResizeEvent *event)
{
    setScale();
    event->accept();
}



int FoilDesignWt::highlightPoint(Vector3d real)
{
    if(!m_pSF->isVisible()) return -1;
    {
        int n = m_pSF->extrados()->isControlPoint(real, m_fScale/m_fRefScale);
        if (n>=0 && n<m_pSF->extrados()->m_CtrlPt.size())
        {
            m_pSF->extrados()->m_iHighlight = n;
            return n;
        }
        else
        {
            if(m_pSF->extrados()->m_iHighlight>=0)
            {
                m_pSF->extrados()->m_iHighlight = -10;
            }
        }

        n = m_pSF->intrados()->isControlPoint(real, m_fScale/m_fRefScale);
        if (n>=0 && n<m_pSF->intrados()->m_CtrlPt.size())
        {
            m_pSF->intrados()->m_iHighlight = n;
            return n;
        }
        else
        {
            if(m_pSF->intrados()->m_iHighlight>=0)
            {
                m_pSF->intrados()->m_iHighlight = -10;
            }
        }
    }
    return  -1;
}


int FoilDesignWt::selectPoint(Vector3d real)
{
    if(!m_pSF->isVisible()) return -1;

    //Selects the point
    m_pSF->extrados()->m_iSelect = m_pSF->extrados()->isControlPoint(real, m_fScale/m_fRefScale);
    if(m_pSF->extrados()->m_iSelect>=0) return m_pSF->extrados()->m_iSelect;

    m_pSF->intrados()->m_iSelect = m_pSF->intrados()->isControlPoint(real, m_fScale/m_fRefScale);
    if(m_pSF->intrados()->m_iSelect>=0) return m_pSF->intrados()->m_iSelect;

    return  -1;
}


void FoilDesignWt::dragSelectedPoint(double , double )
{
    if(!m_pSF->isVisible()) return;

    // user is dragging the point
    int n = m_pSF->extrados()->m_iSelect;
    if (n>=0 && n<m_pSF->extrados()->m_CtrlPt.size())
    {
        m_pSF->extrados()->m_CtrlPt[n].x = m_MousePos.x;
        m_pSF->extrados()->m_CtrlPt[n].y = m_MousePos.y;
        m_pSF->extrados()->splineCurve();

        if(m_pSF->isSymetric())
        {
            m_pSF->intrados()->m_CtrlPt[n].x = m_MousePos.x;
            m_pSF->intrados()->m_CtrlPt[n].y = -m_MousePos.y;
            m_pSF->intrados()->splineCurve();
        }
        if(m_pSF->bClosedTE())
        {
            if(n==m_pSF->extrados()->m_CtrlPt.size()-1)
            {
                m_pSF->intrados()->m_CtrlPt.back() = m_pSF->extrados()->m_CtrlPt.back();
                m_pSF->intrados()->splineCurve();
            }
        }
        if(m_pSF->bClosedLE())
        {
            if(n==0)
            {
                m_pSF->intrados()->m_CtrlPt.front() = m_pSF->extrados()->m_CtrlPt.front();
                m_pSF->intrados()->splineCurve();
            }
        }

        m_pSF->updateSplineFoil();
        m_pSF->setModified(true);
    }
    else
    {
        int n = m_pSF->intrados()->m_iSelect;
        if (n>=0 && n<m_pSF->intrados()->m_CtrlPt.size())
        {
            m_pSF->intrados()->m_CtrlPt[n].x = m_MousePos.x;
            m_pSF->intrados()->m_CtrlPt[n].y = m_MousePos.y;
            m_pSF->intrados()->splineCurve();
            m_pSF->updateSplineFoil();

            if(m_pSF->isSymetric())
            {
                m_pSF->extrados()->m_CtrlPt[n].x =  m_MousePos.x;
                m_pSF->extrados()->m_CtrlPt[n].y = -m_MousePos.y;
                m_pSF->extrados()->splineCurve();
                m_pSF->updateSplineFoil();
            }
            m_pSF->setModified(true);
        }
    }
}


/**
 * The user has requested the insertion of a control point in the SplineFoil at the location of the mouse
 */
void FoilDesignWt::onInsertPt()
{
    if(XDirect::curFoil()) return; // Action can be performed only if the spline foil is selected

    Vector3d Real = mousetoReal(m_PointDown);

    if(Real.y>=0)
    {
        m_pSF->extrados()->insertPoint(Real.x,Real.y);
        m_pSF->extrados()->splineKnots();
        m_pSF->extrados()->splineCurve();
        m_pSF->updateSplineFoil();
    }
    else
    {
        m_pSF->intrados()->insertPoint(Real.x,Real.y);
        m_pSF->intrados()->splineKnots();
        m_pSF->intrados()->splineCurve();
        m_pSF->updateSplineFoil();
    }

//    TakePicture();
}


/**
 * The user has requested the deletion of a control point in the SplineFoil at the location of the mouse.
 */
void FoilDesignWt::onRemovePt()
{
    //Removes a point in the spline
    if(XDirect::curFoil()) return; // Action can be performed only if the spline foil is selected

    Vector3d Real = mousetoReal(m_PointDown);

    int n =  m_pSF->extrados()->isControlPoint(Real, m_fScale/m_fRefScale);
    if (n>=0)
    {
        if(!m_pSF->extrados()->removePoint(n))
        {
            QMessageBox::warning(this, tr("Warning"), tr("The minimum number of control points has been reached for this spline degree"));
            return;
        }
        m_pSF->extrados()->splineKnots();
        m_pSF->extrados()->splineCurve();
        m_pSF->updateSplineFoil();
    }
    else
    {
        int n=m_pSF->intrados()->isControlPoint(Real, m_fScale/m_fRefScale);
        if (n>=0)
        {
            if(!m_pSF->intrados()->removePoint(n))
            {
                QMessageBox::warning(this, tr("Warning"), tr("The minimum number of control points has been reached for this spline degree"));
                return;
            }
            m_pSF->intrados()->splineKnots();
            m_pSF->intrados()->splineCurve();
            m_pSF->updateSplineFoil();
        }
    }

//    TakePicture();
}












