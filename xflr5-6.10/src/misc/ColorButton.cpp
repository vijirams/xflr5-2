/****************************************************************************

	ColorButton Class
	Copyright (C) 2009-2013 Andre Deperrois adeperrois@xflr5.com

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

#include "./Settings.h"
#include "ColorButton.h"
#include <QPainter>
#include <QStyleOption>

ColorButton::ColorButton(QWidget *pParent) : QAbstractButton(pParent)
{
	m_Color = Qt::darkGray;

	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Minimum);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
	setSizePolicy(szPolicyExpanding);

}


QSize ColorButton::sizeHint() const
{
	QFontMetrics fm(Settings::s_TextFont);
	int w = 5 * fm.averageCharWidth();
	int h = fm.height()*3/2;
	return QSize(w, h);
}


void ColorButton::SetColor(QColor const & color)
{
	m_Color = color;
	update();
}


QColor &ColorButton::GetColor()
{
	return m_Color;
}


void ColorButton::paintEvent ( QPaintEvent * event )
{
	QColor paintcolor;


	if(isEnabled()) paintcolor = m_Color;
	else
	{
		if(isDown()) paintcolor = m_Color.lighter(150);
		else         paintcolor = Qt::lightGray;
	}

	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);

	painter.setRenderHint(QPainter::Antialiasing);

//	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

	painter.setBackgroundMode(Qt::TransparentMode);
	QRect r = rect();

	QPen blackPen(Qt::black, 1, Qt::SolidLine);
	QBrush colorbrush(paintcolor);
	painter.setBrush(colorbrush);
//	r.adjust(3,3,-3,-3);

	painter.setPen(blackPen);
	painter.drawRoundedRect(r, 5, 25, Qt::RelativeSize);

	event->accept();
}














