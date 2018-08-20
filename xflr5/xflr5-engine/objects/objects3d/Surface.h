/****************************************************************************

    CSurface Class
	Copyright (C) 2005-2009 Andre Deperrois 

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


/*! @file
 *
 * @brief This class defines the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 *
 */


#ifndef SURFACE_H
#define SURFACE_H




#include <QVector>
#include <objects/objects3d/Panel.h>
#include <analysis3d/analysis3d_enums.h>

#define VLMHALF 2500

/**
*@class Surface
*@brief
 * The class which defines the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 * The mesh panels are not constructed using the Wing object, but on this proxy Surface object.
 *
 A surface extends from one left Foil to the next right Foil in the spanwise directions.

 * The Surface's geometry is defined by
 *    - its Leading edge  : m_LA, m_LB
 *    - its Trailing edge : m_TA, m_TB
 *    - its left and right twist
 *    - its left and right Foil objects
 *    - its Normal vector
 *    - its left and right normal vectors NormalA and NormalB are
 *      the average of the normals of the two continuous surfaces ; used to define junction between panels
 *
 * The points and other geometric data may be requested by calling methods on any of the top, middle, or bottom surfaces.

 Surfaces are constructed and indexed from left tip to right tip.

 For a half-wing, there will be one surface less than the number of foils defined by the user.


 The letter A refers to the Surface's left side, and B refers to the right side.
 The letter L refers to the Leading upstream side, T refers to the Trailing downstream side

 A surface is described by its geometric properties inherited from the wing's definition.
 Surfaces with a span length less than a minimum value are ignored.

 A surface may only supports trailing edge flaps.
 A surface may not be flat if different washout angles have been defined at the tips. However the deviation from
 the flat panel is assumed to be small, and corresponding approximations are made.

 The process of Surface construction starts by the creation of the array of master points at each end Foil.
 These points are defined i.a.w. the number of panels and the type of distribution specified by the user.
 The panel points are interpolated between the left and right Foil points.
 This implies that a wing is built with a fixed number of chordwise panels all along the span.
 The panels on the surface are added incrementally to the global array of panels used for the panel analysis.

 The panels are numbered from left tip to right tip in the span wise direction. Then in the chordwise direction:
		from T.E. to L.E in the case of VLM
		from lower surface TE, to leading edge, and to upper surface TE


 The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
 Angular data is stored in degrees.


*/

class WingSection;
class Foil;
class Body;
class Vector3d;

class XFLR5ENGINELIBSHARED_EXPORT Surface
{
	friend class Wing;
	friend class Miarex;
	friend class Objects3D;
	friend class MainFrame;
	friend class GL3dWingDlg;
	friend class PanelAnalysis;
	friend class PanelAnalysisDlg;
	friend class InertiaDlg;
	friend class WPolar;
	friend class ViewObjectDlg;
	friend class PlaneAnalysisTask;

public:

	Surface();

	void addFlapPanel(Panel *pPanel);
	void copy(Surface *pSurface);
	void createXPoints();
	void getC4(int k, Vector3d &Pt, double &tau);
	void getLeadingPt(int k, Vector3d &C);
	void getNormal(double yrel, Vector3d &N);
	void getTrailingPt(int k, Vector3d &C);
	void getPanel(int const &k, int const &l, enumPanelPosition pos);
    void getSidePoint(double xRel, bool bRight, enumPanelPosition pos, Vector3d &Point, Vector3d &PtNormal);
    void getSurfacePoint(double xArel, double xBrel, double yrel, enumPanelPosition pos, Vector3d &Point, Vector3d &PtNormal);
    void getSection(double const &tau, double &Chord, double &Area, Vector3d &PtC4);
	void getYDist(int const &k, double &y1, double &y2);
	void getSidePoints(enumPanelPosition pos, Body * pBody, Vector3d *PtA, Vector3d *PtB, Vector3d *NA, Vector3d *NB, int nPoints);
    void init();
	void resetFlap();
	void rotateX(Vector3d const &O, double XTilt);
	void rotateY(Vector3d const &O, double YTilt);
	void rotateZ(Vector3d const &O, double ZTilt);
	void setCornerPoints(Vector3d LA, Vector3d TA, Vector3d LB, Vector3d TB);
	void setNormal();
	void setFlap();
	void setSidePoints(Body *pBody, double dx, double dz);
	void setTwist();
	void setTwist2();
	void translate(Vector3d const &T);
	void translate(double tx, double ty, double tz);

	bool isCenterSurf() {return m_bIsCenterSurf;}
	bool isLeftSurf()   {return m_bIsLeftSurf;}
	bool isRightSurf()  {return m_bIsRightSurf;}
	bool isTipLeft()    {return m_bIsTipLeft;}
	bool isTipRight()   {return m_bIsTipRight;}
	bool isInSymPlane() {return m_bIsInSymPlane;}

	bool isFlapPanel(Panel *pPanel);
	bool isFlapPanel(int p);
	bool isFlapNode(int nNode);
	bool rotateFlap(double Angle);

	double twist(int k);
	double chord(int k);
	double chord(double tau);
	double offset(double tau);
	double stripSpanPos(int k);
	double foilArea(double tau);
	double stripWidth(int k);
	double spanLength();
	double planformLength(){return m_Length;}

	int innerSection(){return m_innerSection;}
	int outerSection(){return m_outerSection;}

	int NXPanels(){return m_NXPanels;}
	int NYPanels(){return m_NYPanels;}
	int NXFlap() {return m_NXFlap;}
	int & NElements(){return m_NElements;}

	Foil *foilA() {return m_pFoilA;}
	Foil *foilB() {return m_pFoilB;}

	static void setPanelPointers(Panel *pPanel, Vector3d *pNode);

    QVector<Vector3d> SideA;      /**< the array of panel points on the left foil's mid-line*/
    QVector<Vector3d> SideB;      /**< the array of panel points on the right foil's mid-line*/

	Vector3d m_LA;              /**< the Surface's leading left point */
	Vector3d m_LB;              /**< the Surface's leading right point */
	Vector3d m_TA;              /**< the Surface's trailing left point */
	Vector3d m_TB;              /**< the Surface's trailing right point */
	Vector3d Normal;            /**< the Surface's normal vector */
	Vector3d NormalA;           /**< the normal at the left tip, defined as the average of this Surface's normal and of the one adjacent on the left side, if any */
	Vector3d NormalB;           /**< the normal at the right tip, defined as the average of this Surface's normal and of the one adjacent on the right side, if any */
	double m_TwistA;           /**< the twist at side A in degrees */
	double m_TwistB;           /**< the twist at side B in degrees */

private :
    QVector<Vector3d> SideA_T;    /**< the array of panel points on the left foil's top-line*/
    QVector<Vector3d> SideB_T;    /**< the array of panel points on the right foil's top-line*/
    QVector<Vector3d> SideA_B;    /**< the array of panel points on the left foil's bottom-line*/
    QVector<Vector3d> SideB_B;    /**< the array of panel points on the right foil's bottom-line*/
	Vector3d VTemp;
	static Panel *s_pPanel;    /**< a pointer to the array of this Surface's panels, This array is a sub-array of the total array.*/
	static Vector3d *s_pNode;   /**< a pointer to the array of this panel nodes.*/

	bool m_bIsInSymPlane;      /**< true if the Surface is positioned in the symetry xz plane defined by y=0. Case of a single fin. */
	bool m_bTEFlap;            /**< true if the Surface has a flap on the trailing edge */
	bool m_bIsLeftSurf;        /**< true if the Surface is built on the left wing */
	bool m_bIsRightSurf;       /**< true if the Surface is built on the right wing */
	bool m_bIsTipLeft;         /**< true if the Surface is built on the tip left wing */
	bool m_bIsTipRight;        /**< true if the Surface is built on the tip right wing */
	bool m_bIsCenterSurf;      /**< true if the Surface is either a left or right center surface... need to connect to body */


	double m_Length;           /**< the Surface's planform length from A to B*/
	double chordA;             /**< the chord length at tip A */
	double chordB;             /**< the chord length at tip B */

	double m_posATE, m_posBTE;      /**< the relative flap hinge positions at sides A and B */
	QVarLengthArray<double> m_xPointA;        /**< the chordwise relative position of the VLM panel left corner points at side A */
	QVarLengthArray<double> m_xPointB;        /**< the chordwise relative position of the VLM panel right corner points at side B */

	XFLR5::enumPanelDistribution m_XDistType;            /**< the type of distribution along the Surface's x axis */
	XFLR5::enumPanelDistribution m_YDistType;            /**< the type of distribution along the Surface's y axis */
	int m_NXLead;               /**< the number of panels upstream of the flap, i.e. between the leading edge and the hinge */
	int m_NXFlap;               /**< the number of panels on the flap, i.e. between the hinge and the trailing edge */
	int m_NElements;            /**< the number of panel elements constructer on this Surface. */

	int m_nFlapNodes;           /**< the number of nodes defined on the trailing edge flap */
	int m_nFlapPanels;          /**< the number of panels defined on the trailing edge flap */
	int m_FlapNode[VLMHALF];    /**< the array of flap node indexes, used to avoid defining two nodes at the same location */
	int m_FlapPanel[VLMHALF];   /**< the array of flap panel indexes */

	Vector3d m_HingePoint;       /**< a point on the trailing flap hinge */
	Vector3d m_HingeVector;      /**< a vector which defines the axis of the hinge */

	Surface *m_pLeftSurface;    /**< a pointer to this Surface's left neighbour, or NULL if none */
	Surface *m_pRightSurface;   /**< a pointer to this Surface's right neighbour, or NULL if none */

	int m_innerSection;         /**< the index of the inner wing's section corresponding to this surface */
	int m_outerSection;         /**< the index of the outer wing's section corresponding to this surface */
	int m_NYPanels;                /**< the number of spanwise panels of this surface */
	int m_NXPanels;                /**< the number of chordwise panels of this surface */

public:
	bool m_bJoinRight;             /**< true if the surface's right side should be connected to the next right surface's right left side - for panel analysis only */
	Vector3d LA, LB, TA, TB; /**< leading and trailing corners of strip k */
	Foil *m_pFoilA;                /**< a pointer to the Surface's left Foil object */
	Foil *m_pFoilB;                /**< a pointer to the Surface's right Foil object */

};
#endif
