/****************************************************************************

	TwoDWidget Class
	Copyright (C) 2009-2012 Andre Deperrois adeperrois@xflr5.com

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

/**
*@file
*@brief This file contains the declaration of the class TwoDWidget,
*used for 2d drawing in the central area of the application's MainFrame.
*/
#ifndef TWODWIDGET_H
#define TWODWIDGET_H

#include <QWidget>
#include "params.h"

/**
*@class TwoDWidget 
* @brief This class is used for 2d drawing in the central area of the application's MainFrame.

* There is a unique instance of this class, attached to the QStackedWidget of the MainFrame.
* Depending on the active application, this class calls the drawings methods in QAFoil, QXDirect, QXInverse or QMiarex.
* All Qt events received by this widget are sent to the child applications for handling.
*/
class TwoDWidget : public QWidget
{
    Q_OBJECT

	friend class MainFrame;
	friend class QXDirect;
	friend class QMiarex;

public:
	TwoDWidget(QWidget *parent = 0);

protected:
	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);
 	void wheelEvent (QWheelEvent *event);

private:
	void *m_pAFoil;       /**< A void pointer to the instance of the QAFoil object. */
	void *m_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
	void *m_pMiarex;      /**< A void pointer to the instance of the QMiarex object. */
	void *m_pXDirect;     /**< A void pointer to the instance of the QXDirect object. */
	void *m_pXInverse;    /**< A void pointer to the instance of the QXInverse object. */

};

#endif
