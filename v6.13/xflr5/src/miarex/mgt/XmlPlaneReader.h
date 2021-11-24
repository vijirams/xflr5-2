/****************************************************************************

	XMLPlaneReader Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#ifndef XMLPLANEREADER_H
#define XMLPLANEREADER_H

#include <QXmlStreamReader>
#include <Plane.h>


class XMLPlaneReader : public QXmlStreamReader
{
public:
	XMLPlaneReader(QFile &file, Plane *pPlane);

	void readXMLPlaneFile();

private:
	void readPlane(Plane *pPlane, double lengthUnit, double massUnit);
	void readBody(Body *pBody, CVector &position, double lengthUnit, double massUnit);
	void readPointMass(PointMass *ppm, double massUnit, double lengthUnit);
	void readColor(QColor &color);

	Plane *m_pPlane;
};

#endif // XMLPLANEREADER_H
