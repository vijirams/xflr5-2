/****************************************************************************

    DoubleEdit Class
    Copyright (C) 2014 Andre Deperrois adeperrois@xflr5.com

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

#include "DoubleEdit.h"
#include "./Settings.h"


DoubleEdit::DoubleEdit(QWidget *pParent)  : QLineEdit(pParent)
{
	setParent(pParent);
	m_Value = 0.0;
	m_pDV = new QDoubleValidator(this);
//	m_pDV->setNotation(QDoubleValidator::StandardNotation);
	m_pDV->setRange(-1.e10, 1.e10);
	m_pDV->setDecimals(2);
	setValidator(m_pDV);
	setAlignment(Qt::AlignRight);
	setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	FormatValue();
}


DoubleEdit::DoubleEdit(double val, int decimals, QWidget *pParent)
{
	setParent(pParent);
	m_Value = val;
	m_pDV = new QDoubleValidator(this);
//	m_pDV->setNotation(QDoubleValidator::StandardNotation);
	m_pDV->setRange(-1.e10, 1.e10);
	m_pDV->setDecimals(decimals);
	setValidator(m_pDV);
	setAlignment(Qt::AlignRight);
	setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	FormatValue();
}



QSize DoubleEdit::sizeHint() const
{
	QFontMetrics fm(Settings::s_TextFont);
	int w = 11 * fm.averageCharWidth();
	int h = fm.height();
	return QSize(w, h);
}


void DoubleEdit::focusOutEvent (QFocusEvent *event)
{
	ReadValue();
	FormatValue();
//	emit(editingFinished()); //is emitted by call to base class
	QLineEdit::focusOutEvent(event);
}


double DoubleEdit::ReadValue()
{
	bool bOK;
	double f = locale().toDouble(text().trimmed(), &bOK);
	if(bOK) m_Value = f;
	return m_Value;
//	DoubleEdit orig
//	if(bOK) return f;
//	else    return m_Value;
}


void DoubleEdit::SetValue(double val)
{
	m_Value = val;
	FormatValue();
}


void DoubleEdit::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			ReadValue();
			FormatValue();
			QLineEdit::keyPressEvent(event);

			break;
		}
		case Qt::Key_Escape:
		{
			FormatValue();
			QLineEdit::keyPressEvent(event);
			break;
		}
		default:
		{
			QLineEdit::keyPressEvent(event);
			ReadValue();
			break;
		}
	}
}


void DoubleEdit::FormatValue()
{
	QString str;
	if ((fabs(m_Value)<=1.e-10 || fabs(m_Value)>=pow(10.0, -precision())) && m_Value <10000000.0)
	{
		str=QString("%L1").arg(m_Value,0,'f', precision());
	}
	else
	{
		str=QString("%L1").arg(m_Value,0,'g',precision()+1);
	}
	setText(str);
}


void DoubleEdit::SetValueNoFormat(double val)
{
	m_Value = val;
}


