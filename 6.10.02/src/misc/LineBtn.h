/****************************************************************************

    LineBtn Class
    Copyright (C) 2013 Andre Deperrois adeperrois@xflr5.com

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



#ifndef LINEBTN_H
#define LINEBTN_H


#include <QAbstractButton>


class LineBtn : public QAbstractButton
{
	Q_OBJECT

public:
	LineBtn(QWidget *parent = 0);

	void SetStyle(int const &style, int const &width, QColor const & color);
	void SetColor(QColor const & color);
	void SetStyle(int const &style);
	void SetWidth(int const &width);
	QColor &GetColor();
	int & GetStyle();
	int & GetWidth();


signals:
    void clickedLB();

public:
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	QSize sizeHint() const;

private:
	QColor m_Color;
	int m_Style, m_Width;
};

#endif
