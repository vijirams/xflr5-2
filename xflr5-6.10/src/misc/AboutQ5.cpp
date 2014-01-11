/****************************************************************************

    AboutQ5 Class
	Copyright (C) 2008-2008 Andre Deperrois adeperrois@xflr5.com

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
#include "../params.h"
#include "AboutQ5.h"
#include <QBitmap>
#include <QGridLayout>
#include <QPushButton>


AboutQ5::AboutQ5(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(tr("About XFLR5"));
	SetupLayout();
}


void AboutQ5::SetupLayout()
{
	QGridLayout *LogoLayout = new QGridLayout;
    {
        QLabel *LabIconQ5 = new QLabel;
        LabIconQ5->setObjectName("iconXFLR5");
        LabIconQ5->setPixmap(QPixmap(QString::fromUtf8(":/images/xflr5_64.png")));
	   QLabel *lab1  = new QLabel(VERSIONNAME);
        lab1->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);
        QLabel *XFLR5Link = new QLabel;
        XFLR5Link->setText("<a href=http://www.xflr5.com>http://www.xflr5.com</a>");
        XFLR5Link->setOpenExternalLinks(true);
        XFLR5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        XFLR5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

        LogoLayout->setColumnStretch(1,1);
        LogoLayout->setColumnStretch(2,2);
        LogoLayout->addWidget(LabIconQ5,1,1,2,1);
        LogoLayout->addWidget(lab1,1,2);
        LogoLayout->addWidget(XFLR5Link,2,2);
    }

	QLabel *lab2  = new QLabel(tr("Copyright (C) M. Drela and H. Youngren 2000 - XFoil v6.94"));
	QLabel *lab3  = new QLabel(tr("Copyright (C) Matthieu Scherrer 2004 - Miarex v1.00"));
	QLabel *lab4  = new QLabel(tr("Copyright (C) Andre Deperrois 2003-2013"));
	QLabel *lab5  = new QLabel(tr("This program is distributed in the hope that it will be useful,"));
	QLabel *lab6  = new QLabel(tr("but WITHOUT ANY WARRANTY; without even the implied warranty of"));
	QLabel *lab7  = new QLabel(tr("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
	QLabel *lab8  = new QLabel(tr("This program has been developed exclusively for the analysis of model aircraft"));
	QLabel *lab9  = new QLabel(tr("Any other usage is strongly disapproved"));
	QLabel *lab10 = new QLabel(tr("Program distributed  under the terms of the GNU General Public License"));
	QLabel *lab11 = new QLabel(tr("German translation by Martin Willner"));
	QLabel *lab12 = new QLabel(tr("Japanese translation by IKUSU, Koichi Akabe, Misatus, dynamicsoar, hide253"));
	QLabel *lab13 = new QLabel(tr("icchy_07, ina111, ohayo_cycling, ohisa_64, ozawa64."));
	QLabel *lab14 = new QLabel(tr("French translation by Jean-Luc Coulon"));

	QPushButton *OKButton = new QPushButton(tr("OK"));
	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	QHBoxLayout *OKLayout = new QHBoxLayout;
    {
        OKLayout->addStretch(1);
        OKLayout->addWidget(OKButton);
        OKLayout->addStretch(1);
    }

	QVBoxLayout *MainLayout = new QVBoxLayout;
    {
        MainLayout->addLayout(LogoLayout);
        MainLayout->addStretch(1);
        MainLayout->addWidget(lab2);
        MainLayout->addWidget(lab3);
        MainLayout->addWidget(lab4);
        MainLayout->addStretch(1);
        MainLayout->addWidget(lab11);
        MainLayout->addWidget(lab12);
        MainLayout->addWidget(lab13);
        MainLayout->addWidget(lab14);
        MainLayout->addSpacing(20);
        MainLayout->addStretch(1);
        MainLayout->addWidget(lab5);
        MainLayout->addWidget(lab6);
        MainLayout->addWidget(lab7);
        MainLayout->addStretch(1);
        MainLayout->addWidget(lab8);
        MainLayout->addWidget(lab9);
        MainLayout->addStretch(1);
        MainLayout->addWidget(lab10);
        MainLayout->addStretch(1);
        MainLayout->addLayout(OKLayout);
    }
	setLayout(MainLayout);
	setMinimumHeight(400);
}



