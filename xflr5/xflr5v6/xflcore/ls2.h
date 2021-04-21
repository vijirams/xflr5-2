/****************************************************************************

    LS2 Class
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

#include <QColor>
#include <QSettings>
#include <QDataStream>


namespace Line
{
    /** @enum The different applications available to the user */
    typedef enum {SOLID, DASH, DOT, DASHDOT, DASHDOTDOT, NOLINE} 	enumLineStipple;

    /**< @enum The different image formats usable to export screen captures*/
    typedef enum {NOSYMBOL, LITTLECIRCLE,   BIGCIRCLE, LITTLESQUARE, BIGSQUARE}	enumPointStyle;
}

/** new style line style - intended to replace the current LineStyle struct */
struct LS2
{
    LS2()
    {
        m_bIsVisible=true;
        m_Stipple= Line::SOLID;
        m_Width=1;
        m_Color = Qt::gray;
        m_PointStyle=Line::NOSYMBOL;
    }

    LS2(bool bVisible, Line::enumLineStipple style, int width, QColor color, Line::enumPointStyle pointstyle)
    {
        m_bIsVisible = bVisible;
        m_Stipple    = style;
        m_Width      = width;
        m_Color      = color;
        m_PointStyle = pointstyle;
    }

    void saveSettings(QSettings &settings, QString const &name) const
    {
        settings.setValue(name+"_visible", m_bIsVisible);
        settings.setValue(name+"_color", m_Color);
        settings.setValue(name+"_width", m_Width);

        switch (m_Stipple)
        {
            case Line::SOLID:      settings.setValue(name+"_line", 0);  break;
            case Line::DASH:       settings.setValue(name+"_line", 1);  break;
            case Line::DOT:        settings.setValue(name+"_line", 2);  break;
            case Line::DASHDOT:    settings.setValue(name+"_line", 3);  break;
            case Line::DASHDOTDOT: settings.setValue(name+"_line", 4);  break;
            case Line::NOLINE:     settings.setValue(name+"_line", 5);  break;
        }
        switch (m_PointStyle)
        {
            case Line::NOSYMBOL:       settings.setValue(name+"_pts", 0);   break;
            case Line::LITTLECIRCLE:   settings.setValue(name+"_pts", 1);   break;
            case Line::BIGCIRCLE:      settings.setValue(name+"_pts", 2);   break;
            case Line::LITTLESQUARE:   settings.setValue(name+"_pts", 3);   break;
            case Line::BIGSQUARE:      settings.setValue(name+"_pts", 4);   break;
        }
    }


    void loadSettings(QSettings &settings, QString const &name)
    {
        if(settings.contains(name+"_visible")) m_bIsVisible = settings.value(name+"_visible", true).toBool();
        if(settings.contains(name+"_color"))   m_Color      = settings.value(name+"_color", QColor(205,205,205)).value<QColor>();
        if(settings.contains(name+"_width"))   m_Width      = settings.value(name+"_width", 1).toInt();

        if(settings.contains(name+"_line"))
        {
            int istyle = settings.value(name+"_line", 0).toInt();
            setStipple(istyle);
        }
        if(settings.contains(name+"_pts"))
        {
            int ipts = settings.value(name+"_pts", 0).toInt();
            setPointStyle(ipts);
        }
    }

    //Conversion functions from deprecated struct LineStyle
    void setStipple(int n)
    {
        switch (n)
        {
            default:
            case 0:   m_Stipple = Line::SOLID;      break;
            case 1:   m_Stipple = Line::DASH;       break;
            case 2:   m_Stipple = Line::DOT;        break;
            case 3:   m_Stipple = Line::DASHDOT;    break;
            case 4:   m_Stipple = Line::DASHDOTDOT; break;
            case 5:   m_Stipple = Line::NOLINE;     break;
        }
    }

    void setPointStyle(int ipts)
    {
        switch (ipts)
        {
            default:
            case  0:   m_PointStyle = Line::NOSYMBOL;       break;
            case  1:   m_PointStyle = Line::LITTLECIRCLE;   break;
            case  2:   m_PointStyle = Line::BIGCIRCLE;      break;
            case  3:   m_PointStyle = Line::LITTLESQUARE;   break;
            case  4:   m_PointStyle = Line::BIGSQUARE;      break;
        }
    }

    static Line::enumLineStipple convertLineStyle(int iStipple)
    {
        switch (iStipple)
        {
            default:
            case 0: return Line::SOLID;
            case 1: return Line::DASH;
            case 2: return Line::DOT;
            case 3: return Line::DASHDOT;
            case 4: return Line::DASHDOTDOT;
            case 5: return Line::NOLINE;
        }
    }


    static int convertLineStyle(Line::enumLineStipple style)
    {
        switch (style)
        {
            case Line::SOLID:      return 0;
            case Line::DASH:       return 1;
            case Line::DOT:        return 2;
            case Line::DASHDOT:    return 3;
            case Line::DASHDOTDOT: return 4;
            case Line::NOLINE:     return 5;
        }
        return 0;
    }


    static Line::enumPointStyle convertPointStyle(int iStyle)
    {
        switch (iStyle)
        {
            default:
            case 0:  return Line::NOSYMBOL;
            case 1:  return Line::LITTLECIRCLE;
            case 2:  return Line::BIGCIRCLE;
            case 3:  return Line::LITTLESQUARE;
            case 4:  return Line::BIGSQUARE;
        }
    }


    static int convertPointStyle(Line::enumPointStyle ptStyle)
    {
        switch (ptStyle)
        {
            case Line::NOSYMBOL:        return 0;
            case Line::LITTLECIRCLE:    return 1;
            case Line::BIGCIRCLE:       return 2;
            case Line::LITTLESQUARE:    return 3;
            case Line::BIGSQUARE:       return 4;
        }
        return 0;
    }


    void serializeXfl(QDataStream &ar, bool bIsStoring)
    {
        int k=0;
        if(bIsStoring)
        {
            ar << convertLineStyle(m_Stipple);
            ar << m_Width;
            ar << convertPointStyle(m_PointStyle);
            ar << m_Color;
            ar << m_bIsVisible;
        }
        else
        {
            ar >> k; m_Stipple=convertLineStyle(k);
            ar >> m_Width;
            ar >> k; m_PointStyle=convertPointStyle(k);
            ar >> m_Color;
            ar >> m_bIsVisible;
        }
    }


public:
    bool m_bIsVisible=true;       /**< true if the curve is visible in the active view >*/
    Line::enumLineStipple m_Stipple=Line::SOLID;              /**< the index of the style with which to draw the curve >*/
    int m_Width=1;                /**< the width with which to draw the curve >*/
    QColor m_Color=Qt::darkGray;     /**< the color with which to draw the curve >*/

    Line::enumPointStyle m_PointStyle=Line::NOSYMBOL;
};

