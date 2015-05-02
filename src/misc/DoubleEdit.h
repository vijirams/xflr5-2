/****************************************************************************

    DoubleEdit Class
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


#ifndef DOUBLEEDIT_H
#define DOUBLEEDIT_H

#include <QDoubleValidator>
#include <QLineEdit>
#include <QKeyEvent>


class DoubleEdit : public QLineEdit
{
public:
	DoubleEdit(QWidget *pParent=NULL);
	DoubleEdit(double val, int decimals=2, QWidget *pParent=NULL);

	~DoubleEdit() {delete m_pDV;}

	void focusOutEvent ( QFocusEvent * event );
	void keyPressEvent(QKeyEvent *event);
	QSize sizeHint() const;

	double Value(){return m_Value;}
	void SetValue(double val);


	void SetValueNoFormat(double val);

	void FormatValue();
	double ReadValue();
	void SetMin(double min) {m_pDV->setBottom(min);}
	void SetMax(double max) {m_pDV->setTop(max);}


	void SetPrecision(int decimals) {m_pDV->setDecimals(decimals);}

	void setNotation(QDoubleValidator::Notation notation) {m_pDV->setNotation(notation);}

	int precision(){return m_pDV->decimals();}

public:
	QDoubleValidator *m_pDV;
	double m_Value;//we need to store a full precision value, irrespective of the display
};

#endif // DOUBLEEDIT_H
