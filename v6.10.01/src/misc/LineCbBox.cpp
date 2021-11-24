/****************************************************************************

	LineCbBox Class
	Copyright (C) 2009-2014 Andre Deperrois adeperrois@xflr5.com

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
#include "./Settings.h"
#include "LineCbBox.h"
#include <QtDebug>
#include <QPainter>
#include <QPaintEvent>

LineCbBox::LineCbBox(QWidget *pParent)
	:QComboBox(pParent)
{
	setParent(pParent);
	m_Style = 0;
	m_Width = 1;
	m_Color = QColor(255,100,50);

	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
	setSizePolicy(szPolicyExpanding);
}


QSize LineCbBox::sizeHint() const
{
	QFontMetrics fm(Settings::s_TextFont);
	int w = 7 * fm.averageCharWidth();
	int h = fm.height();
	return QSize(w, h);
}



void LineCbBox::SetLine(int const &style, int const &width, QColor const &color)
{
	m_Style = style;
	m_Width = width;
	m_Color = color;
}


void LineCbBox::paintEvent (QPaintEvent *event)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

	painter.save();

	painter.setRenderHint(QPainter::Antialiasing);

	QColor ContourColor = Qt::gray;

	if(isEnabled()) ContourColor = Qt::gray;
	else            ContourColor = Qt::lightGray;

	QRect r = event->rect();
//	QRect g = rect();
	painter.setBrush(Qt::NoBrush);
	painter.setBackgroundMode(Qt::TransparentMode);

	QPen LinePen(m_Color);
	LinePen.setStyle(getStyle(m_Style));
	LinePen.setWidth(m_Width);
	painter.setPen(LinePen);
	painter.drawLine(r.left()+5, r.height()/2, r.width()-10, r.height()/2);

//	QPen ContourPen(ContourColor);
//	painter.setPen(ContourPen);
//	r.adjust(0,2,-1,-3);
//	painter.drawRoundRect(r,5,40);

	painter.restore();
}

